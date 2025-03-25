/*
 * Copyright (C) 2024 Igalia S.L. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if PLATFORM(GTK) || PLATFORM(WPE)
#include "FloatRect.h"
#include "Region.h"
#include <wtf/ForbidHeapAllocation.h>

namespace WebCore {

class Damage {
    WTF_FORBID_HEAP_ALLOCATION;

public:
    using Rects = Vector<IntRect, 1>;

    enum class Propagation : uint8_t {
        None,
        Region,
        Unified,
    };

    static constexpr int s_tileSize { 256 };

    enum class Mode : uint8_t {
        Rectangles, // Tracks dirty regions as rectangles, only unifying when maximum is reached.
        BoundingBox, // Dirty region is always the minimum bounding box of all added rectangles.
        Full, // All area is always dirty.
    };

    explicit Damage(const IntSize& size, Mode mode = Mode::Rectangles)
        : m_mode(mode)
        , m_size(size)
        , m_tileSize(s_tileSize, s_tileSize)
    {
        initialize();
    }

    explicit Damage(const FloatSize& size, Mode mode = Mode::Rectangles)
        : Damage(ceiledIntSize(LayoutSize(size)), mode)
    {
    }

    Damage(Damage&&) = default;
    Damage(const Damage&) = default;
    Damage& operator=(const Damage&) = default;
    Damage& operator=(Damage&&) = default;

    ALWAYS_INLINE const IntRect& bounds() const { return m_minimumBoundingRectangle; }

    // May return both empty and overlapping rects.
    ALWAYS_INLINE const Rects& rects() const { return m_rects; }
    ALWAYS_INLINE bool isEmpty() const  { return m_rects.isEmpty(); }

    void makeFull(const IntSize& size)
    {
        if (m_mode == Mode::Full && m_size == size)
            return;

        m_size = size;
        m_mode = Mode::Full;
        m_rects.clear();
        initialize();
    }

    void makeFull(const FloatSize& size)
    {
        makeFull(ceiledIntSize(LayoutSize(size)));
    }

    void makeFull()
    {
        makeFull(m_size);
    }

    ALWAYS_INLINE void add(const Region& region)
    {
        if (region.isEmpty() || !shouldAdd())
            return;

        for (const auto& rect : region.rects())
            add(rect);
    }

    void add(const IntRect& rect)
    {
        if (rect.isEmpty() || !shouldAdd())
            return;

        const auto rectsCount = m_rects.size();
        if (!rectsCount || rect.contains(m_minimumBoundingRectangle)) {
            m_rects.clear();
            m_rects.append(rect);
            m_minimumBoundingRectangle = rect;
            return;
        }

        if (rectsCount == 1 && m_minimumBoundingRectangle.contains(rect))
            return;

        m_minimumBoundingRectangle.unite(rect);
        if (m_mode == Mode::BoundingBox) {
            ASSERT(rectsCount == 1);
            m_rects[0] = m_minimumBoundingRectangle;
            return;
        }

        if (m_shouldUnite) {
            unite(rect);
            return;
        }

        if (rectsCount == m_gridSize.unclampedArea()) {
            m_shouldUnite = true;
            uniteExistingRects();
            unite(rect);
            return;
        }

        m_rects.append(rect);
    }

    ALWAYS_INLINE void add(const FloatRect& rect)
    {
        if (rect.isEmpty() || !shouldAdd())
            return;

        add(enclosingIntRect(rect));
    }

    ALWAYS_INLINE void add(const Damage& other)
    {
        if (other.isEmpty() || !shouldAdd())
            return;

        for (const auto& rect : other.rects())
            add(rect);
    }

private:
    void initialize()
    {
        switch (m_mode) {
        case Mode::Rectangles:
            m_gridSize = ceiledIntSize({ static_cast<float>(m_size.width()) / m_tileSize.width(), static_cast<float>(m_size.height()) / m_tileSize.height() }).expandedTo({ 1, 1 });
            m_shouldUnite = m_gridSize.width() == 1 && m_gridSize.height() == 1;
            break;
        case Mode::BoundingBox:
            break;
        case Mode::Full:
            m_minimumBoundingRectangle = { { }, m_size };
            m_rects.append(m_minimumBoundingRectangle);
            break;
        }
    }

    ALWAYS_INLINE bool shouldAdd() const
    {
        return !m_size.isEmpty() && m_mode != Mode::Full;
    }

    void uniteExistingRects()
    {
        Rects rectsCopy(m_rects.size());
        m_rects.swap(rectsCopy);

        for (const auto& rect : rectsCopy)
            unite(rect);
    }

    ALWAYS_INLINE size_t tileIndexForRect(const IntRect& rect) const
    {
        if (m_rects.size() == 1)
            return 0;

        const auto rectCenter = rect.center();
        const auto rectCell = flooredIntPoint(FloatPoint { static_cast<float>(rectCenter.x()) / m_tileSize.width(), static_cast<float>(rectCenter.y()) / m_tileSize.height() });
        return std::clamp(rectCell.x(), 0, m_gridSize.width() - 1) + std::clamp(rectCell.y(), 0, m_gridSize.height() - 1) * m_gridSize.width();
    }

    void unite(const IntRect& rect)
    {
        // When merging cannot be avoided, we use m_rects to store minimal bounding rectangles
        // and perform merging while trying to keep minimal bounding rectangles small and
        // separated from each other.
        const auto index = tileIndexForRect(rect);
        ASSERT(index < m_rects.size());
        m_rects[index].unite(rect);
    }

    Mode m_mode { Mode::Rectangles };
    IntSize m_size;
    bool m_shouldUnite { false };
    IntSize m_tileSize;
    IntSize m_gridSize;
    Rects m_rects;
    IntRect m_minimumBoundingRectangle;

    friend bool operator==(const Damage&, const Damage&) = default;
};

class FrameDamageHistory {
    WTF_MAKE_FAST_ALLOCATED_WITH_HEAP_IDENTIFIER(FrameDamageHistory);
public:
    const Vector<Region>& damageInformation() const { return m_damageInfo; }

    void addDamage(const Damage& damage)
    {
        Region region;
        for (const auto& rect : damage.rects())
            region.unite(rect);
        m_damageInfo.append(WTFMove(region));
    }

private:
    // Use a Region to remove overlaps so that Damage rects are more predictable from the testing perspective.
    Vector<Region> m_damageInfo;
};

static inline WTF::TextStream& operator<<(WTF::TextStream& ts, const Damage& damage)
{
    return ts << "Damage"_s << damage.rects();
}

} // namespace WebCore

#endif // PLATFORM(GTK) || PLATFORM(WPE)
