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

#include "config.h"
#include "CSSScrollPaddingEdgeValue.h"

#include "CSSPrimitiveNumericTypes+CSSValueVisitation.h"
#include "CSSPrimitiveNumericTypes+Serialization.h"

namespace WebCore {

Ref<CSSScrollPaddingEdgeValue> CSSScrollPaddingEdgeValue::create(CSS::ScrollPaddingEdge edge)
{
    return adoptRef(*new CSSScrollPaddingEdgeValue(WTFMove(edge)));
}

CSSScrollPaddingEdgeValue::CSSScrollPaddingEdgeValue(CSS::ScrollPaddingEdge&& edge)
    : CSSValue(ClassType::ScrollPaddingEdge)
    , m_edge(WTFMove(edge))
{
}

String CSSScrollPaddingEdgeValue::customCSSText() const
{
    return CSS::serializationForCSS(m_edge);
}

bool CSSScrollPaddingEdgeValue::equals(const CSSScrollPaddingEdgeValue& other) const
{
    return m_edge == other.m_edge;
}

IterationStatus CSSScrollPaddingEdgeValue::customVisitChildren(const Function<IterationStatus(CSSValue&)>& func) const
{
    return CSS::visitCSSValueChildren(func, m_edge);
}

} // namespace WebCore
