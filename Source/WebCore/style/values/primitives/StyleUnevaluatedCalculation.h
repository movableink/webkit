/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "CSSPrimitiveNumericRange.h"
#include "CalculationValue.h"

namespace WebCore {
namespace Style {

// Wrapper for `Ref<CalculationValue>` that includes range and category as part of the type.
template<CSS::Range R, Calculation::Category C> struct UnevaluatedCalculation {
    static constexpr auto range = R;
    static constexpr auto category = C;

    Ref<CalculationValue> value;

    Ref<CalculationValue> protectedCalculation() const
    {
        return value;
    }

    explicit UnevaluatedCalculation(Ref<CalculationValue> root)
        : value { WTFMove(root) }
    {
    }

    explicit UnevaluatedCalculation(Calculation::Child root)
        : UnevaluatedCalculation {
            CalculationValue::create(
                Calculation::Tree {
                    .root = WTFMove(root),
                    .category = category,
                    .range = { range.min, range.max },
                }
            )
        }
    {
    }

    bool operator==(const UnevaluatedCalculation&) const = default;
};

} // namespace Style
} // namespace WebCore

template<auto R, auto C> struct WTF::IsSmartPtr<WebCore::Style::UnevaluatedCalculation<R, C>> {
    static constexpr bool value = true;
};
