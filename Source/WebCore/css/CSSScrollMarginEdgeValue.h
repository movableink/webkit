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

#include "CSSPrimitiveNumericTypes+CSSValueCreation.h"
#include "CSSScrollMargin.h"
#include "CSSValue.h"

namespace WebCore {

class CSSScrollMarginEdgeValue final : public CSSValue {
public:
    static Ref<CSSScrollMarginEdgeValue> create(CSS::ScrollMarginEdge);

    const CSS::ScrollMarginEdge& edge() const { return m_edge; }

    String customCSSText() const;
    bool equals(const CSSScrollMarginEdgeValue&) const;
    IterationStatus customVisitChildren(const Function<IterationStatus(CSSValue&)>&) const;

private:
    CSSScrollMarginEdgeValue(CSS::ScrollMarginEdge&&);

    CSS::ScrollMarginEdge m_edge;
};

template<> struct CSS::CSSValueCreation<CSS::ScrollMarginEdge> {
    Ref<CSSValue> operator()(const CSS::ScrollMarginEdge& value)
    {
        return CSSScrollMarginEdgeValue::create(value);
    }
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_CSS_VALUE(CSSScrollMarginEdgeValue, isScrollMarginEdgeValue())
