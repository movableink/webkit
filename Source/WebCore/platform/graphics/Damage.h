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

#if ENABLE(DAMAGE_TRACKING)
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

    Damage() = default;
    Damage(Damage&&) = default;
    Damage(const Damage&) = default;
    Damage& operator=(const Damage&) = default;
    Damage& operator=(Damage&&) = default;

    static const Damage& invalid()
    {
        static const Damage invalidDamage(true);
        return invalidDamage;
    }

    Region region() const
    {
        Region region;
        for (const auto& rect : rects())
            region.unite(rect);
        return region;
    }

    ALWAYS_INLINE IntRect bounds() const { return m_minimumBoundingRectangle; }

    // May return both empty and overlapping rects.
    ALWAYS_INLINE const Rects& rects() const { return m_rects; }
    ALWAYS_INLINE bool isEmpty() const  { return !m_invalid && m_rects.isEmpty(); }
    ALWAYS_INLINE bool isInvalid() const { return m_invalid; }

    void invalidate()
    {
        m_invalid = true;
        m_rects = { };
    }

    ALWAYS_INLINE void add(const Region& region)
    {
        if (isInvalid())
            return;
        for (const auto& rect : region.rects())
            add(rect);
    }

    void add(const IntRect& rect)
    {
        if (isInvalid() || rect.isEmpty())
            return;

        if (rect.contains(m_minimumBoundingRectangle)) {
            m_rects = { rect };
            m_minimumBoundingRectangle = rect;
            return;
        }

        if (m_rects.size() == 1 && m_minimumBoundingRectangle.contains(rect))
            return;

        m_minimumBoundingRectangle.unite(rect);

        if (!shouldUnite()) {
            m_rects.append(rect);
            if (shouldUnite())
                uniteExistingRects();
        } else
            unite(rect);
    }

    ALWAYS_INLINE void add(const FloatRect& rect)
    {
        add(enclosingIntRect(rect));
    }

    ALWAYS_INLINE void add(const Damage& other)
    {
        m_invalid = other.isInvalid();
        for (const auto& rect : other.rects())
            add(rect);
    }

private:
    // Artificial NxM grid is used to direct input rectangles into proper buckets (cells)
    // used to create minimum bounding rectangles that approximate the damaged area -
    // potentially with overlaps.
    static constexpr size_t s_gridCols { 8 };
    static constexpr size_t s_gridRows { 4 };
    static constexpr size_t s_gridCells { s_gridCols * s_gridRows };

    bool shouldUnite() const { return m_rects.size() >= s_gridCells; }

    void uniteExistingRects()
    {
        const Rects rectsCopy = m_rects;
        for (auto& rect : m_rects)
            rect = { };
        for (const auto& rect : rectsCopy)
            unite(rect);
    }

    void unite(const IntRect& rect)
    {
        // When merging cannot be avoided, we use m_rects to store minimal bounding rectangles
        // and perform merging while trying to keep minimal bounding rectangles small and
        // separated from each other.

        // FIXME: Figure out a way to pass viewport size here for better results.
        const IntSize viewportSize { 2048, 1024 };
        const IntSize gridSize { s_gridCols, s_gridRows };
        const auto tileSize = IntSize(viewportSize / gridSize);
        const auto rectCenter = rect.center();
        const auto rectCell = ceiledIntPoint(FloatPoint { static_cast<float>(rectCenter.x()) / tileSize.width(), static_cast<float>(rectCenter.y()) / tileSize.height() });
        const size_t index = std::clamp(rectCell.x(), 0, static_cast<int>(s_gridCols - 1)) + std::clamp(rectCell.y(), 0, static_cast<int>(s_gridRows - 1)) * s_gridCols;
        m_rects[index].unite(rect);
    }

    bool m_invalid { false };
    Rects m_rects;
    IntRect m_minimumBoundingRectangle;

    explicit Damage(bool invalid)
        : m_invalid(invalid)
    {
    }

    friend struct IPC::ArgumentCoder<Damage, void>;

    friend bool operator==(const Damage&, const Damage&) = default;
};

class FrameDamageHistory {
    WTF_MAKE_FAST_ALLOCATED_WITH_HEAP_IDENTIFIER(FrameDamageHistory);
public:
    using SimplifiedDamage = std::pair<bool, Region>;

    const Vector<SimplifiedDamage>& damageInformation() const { return m_damageInfo; }
    void addDamage(const SimplifiedDamage& damage) { m_damageInfo.append(damage); }

private:
    Vector<SimplifiedDamage> m_damageInfo;
};

static inline WTF::TextStream& operator<<(WTF::TextStream& ts, const Damage& damage)
{
    if (damage.isInvalid())
        return ts << "Damage[invalid]";
    return ts << "Damage" << damage.rects();
}

};

#endif // ENABLE(DAMAGE_TRACKING)
