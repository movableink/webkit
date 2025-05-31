/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "FloatRect.h"
#include "LayoutRect.h"
#include <wtf/FixedVector.h>

namespace WebCore {
namespace Style {

enum class ShadowStyle : bool { Normal, Inset };

template<typename T> concept Shadow = requires(const T& shadow) {
    { shadowStyle(shadow) } -> std::same_as<ShadowStyle>;
    { isInset(shadow) } -> std::same_as<bool>;
    { paintingSpread(shadow) } -> std::same_as<LayoutUnit>;
};

LayoutUnit paintingExtent(Shadow auto const& shadow)
{
    // Blurring uses a Gaussian function whose std. deviation is m_radius/2, and which in theory
    // extends to infinity. In 8-bit contexts, however, rounding causes the effect to become
    // undetectable at around 1.4x the radius.
    constexpr const float radiusExtentMultiplier = 1.4;
    return LayoutUnit { ceilf(shadow.blur.value * radiusExtentMultiplier) };
}

LayoutUnit paintingExtentAndSpread(Shadow auto const& shadow)
{
    return paintingExtent(shadow) + paintingSpread(shadow);
}

template<Shadow ShadowType> LayoutBoxExtent shadowOutsetExtent(const FixedVector<ShadowType>& shadows)
{
    LayoutUnit top;
    LayoutUnit right;
    LayoutUnit bottom;
    LayoutUnit left;

    for (const auto& shadow : shadows) {
        if (isInset(shadow))
            continue;

        auto extentAndSpread = paintingExtentAndSpread(shadow);

        left = std::min(LayoutUnit(shadow.location.x().value) - extentAndSpread, left);
        right = std::max(LayoutUnit(shadow.location.x().value) + extentAndSpread, right);
        top = std::min(LayoutUnit(shadow.location.y().value) - extentAndSpread, top);
        bottom = std::max(LayoutUnit(shadow.location.y().value) + extentAndSpread, bottom);
    }

    return { top, right, bottom, left };
}

template<Shadow ShadowType> LayoutBoxExtent shadowInsetExtent(const FixedVector<ShadowType>& shadows)
{
    LayoutUnit top;
    LayoutUnit right;
    LayoutUnit bottom;
    LayoutUnit left;

    for (const auto& shadow : shadows) {
        if (!isInset(shadow))
            continue;

        auto extentAndSpread = paintingExtentAndSpread(shadow);

        top = std::max<LayoutUnit>(top, LayoutUnit(shadow.location.y().value) + extentAndSpread);
        right = std::min<LayoutUnit>(right, LayoutUnit(shadow.location.x().value) - extentAndSpread);
        bottom = std::min<LayoutUnit>(bottom, LayoutUnit(shadow.location.y().value) - extentAndSpread);
        left = std::max<LayoutUnit>(left, LayoutUnit(shadow.location.x().value) + extentAndSpread);
    }

    return { top, right, bottom, left };
}

template<Shadow ShadowType> void getShadowHorizontalExtent(const FixedVector<ShadowType>& shadows, LayoutUnit& left, LayoutUnit& right)
{
    left = 0;
    right = 0;

    for (const auto& shadow : shadows) {
        if (isInset(shadow))
            continue;

        auto extentAndSpread = paintingExtentAndSpread(shadow);

        left = std::min<LayoutUnit>(left, LayoutUnit(shadow.location.x().value) - extentAndSpread);
        right = std::max<LayoutUnit>(right, LayoutUnit(shadow.location.x().value) + extentAndSpread);
    }
}

template<Shadow ShadowType> void getShadowVerticalExtent(const FixedVector<ShadowType>& shadows, LayoutUnit& top, LayoutUnit& bottom)
{
    top = 0;
    bottom = 0;

    for (const auto& shadow : shadows) {
        if (isInset(shadow))
            continue;

        auto extentAndSpread = paintingExtentAndSpread(shadow);

        top = std::min<LayoutUnit>(top, LayoutUnit(static_cast<int>(shadow.location.y().value)) - extentAndSpread);
        bottom = std::max<LayoutUnit>(bottom, LayoutUnit(static_cast<int>(shadow.location.y().value)) + extentAndSpread);
    }
}

template<Shadow ShadowType> void adjustRectForShadow(LayoutRect& rect, const FixedVector<ShadowType>& shadows)
{
    auto shadowExtent = shadowOutsetExtent(shadows);

    rect.move(shadowExtent.left(), shadowExtent.top());
    rect.setWidth(rect.width() - shadowExtent.left() + shadowExtent.right());
    rect.setHeight(rect.height() - shadowExtent.top() + shadowExtent.bottom());
}

template<Shadow ShadowType> void adjustRectForShadow(FloatRect& rect, const FixedVector<ShadowType>& shadows)
{
    auto shadowExtent = shadowOutsetExtent(shadows);

    rect.move(shadowExtent.left(), shadowExtent.top());
    rect.setWidth(rect.width() - shadowExtent.left() + shadowExtent.right());
    rect.setHeight(rect.height() - shadowExtent.top() + shadowExtent.bottom());
}

} // namespace Style
} // namespace WebCore
