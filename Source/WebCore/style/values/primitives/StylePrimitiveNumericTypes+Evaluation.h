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

#include "FloatConversion.h"
#include "FloatPoint.h"
#include "FloatSize.h"
#include "StylePrimitiveNumericTypes+Calculation.h"
#include "StylePrimitiveNumericTypes.h"

namespace WebCore {
namespace Style {

// MARK: - Number

template<auto R, typename V> constexpr float evaluate(const Number<R, V>& number, float)
{
    return narrowPrecisionToFloat(number.value);
}

template<auto R, typename V> constexpr double evaluate(const Number<R, V>& number, double)
{
    return number.value;
}

// MARK: - Percentage

template<auto R, typename V> constexpr float evaluate(const Percentage<R, V>& percentage, float referenceLength)
{
    return clampTo<float>(percentage.value) / 100.0f * referenceLength;
}

template<auto R, typename V> constexpr double evaluate(const Percentage<R, V>& percentage, double referenceLength)
{
    return percentage.value / 100.0 * referenceLength;
}

// MARK: - Numeric

constexpr float evaluate(Numeric auto const& value, float)
{
    return value.value;
}

constexpr double evaluate(Numeric auto const& value, double)
{
    return value.value;
}

inline float evaluate(const CalculationValue& calculation, float referenceValue)
{
    return calculation.evaluate(referenceValue);
}

inline double evaluate(const CalculationValue& calculation, double referenceValue)
{
    return calculation.evaluate(referenceValue);
}

inline float evaluate(Calc auto const& calculation, float referenceValue)
{
    return evaluate(calculation.protectedCalculation(), referenceValue);
}

inline double evaluate(Calc auto const& calculation, double referenceValue)
{
    return evaluate(calculation.protectedCalculation(), referenceValue);
}

// MARK: - DimensionPercentageNumeric (e.g. AnglePercentage/LengthPercentage)

inline float evaluate(DimensionPercentageNumeric auto const& value, float referenceValue)
{
    return WTF::switchOn(value, [&referenceValue](const auto& value) -> float { return evaluate(value, referenceValue); });
}

inline double evaluate(DimensionPercentageNumeric auto const& value, double referenceValue)
{
    return WTF::switchOn(value, [&referenceValue](const auto& value) -> double { return evaluate(value, referenceValue); });
}

// MARK: - NumberOrPercentage

template<auto nR, auto pR, typename V> double evaluate(const NumberOrPercentage<nR, pR, V>& value)
{
    return WTF::switchOn(value,
        [](const typename NumberOrPercentage<nR, pR, V>::Number& number) -> double { return number.value; },
        [](const typename NumberOrPercentage<nR, pR, V>::Percentage& percentage) -> double { return percentage.value / 100.0; }
    );
}

// MARK: - SpaceSeparatedPoint

template<typename T> FloatPoint evaluate(const SpaceSeparatedPoint<T>& value, FloatSize referenceBox)
{
    return {
        evaluate(value.x(), referenceBox.width()),
        evaluate(value.y(), referenceBox.height())
    };
}

// MARK: - SpaceSeparatedSize

template<typename T> FloatSize evaluate(const SpaceSeparatedSize<T>& value, FloatSize referenceBox)
{
    return {
        evaluate(value.width(), referenceBox.width()),
        evaluate(value.height(), referenceBox.height())
    };
}

// MARK: - Calculated Evaluations

// Convert to `calc(100% - value)`.
template<auto R, typename V> auto reflect(const LengthPercentage<R, V>& value) -> LengthPercentage<R, V>
{
    using Result = LengthPercentage<R, V>;
    using Dimension = typename Result::Dimension;
    using Percentage = typename Result::Percentage;
    using Calc = typename Result::Calc;

    return WTF::switchOn(value,
        [&](const Dimension& value) -> Result {
            // If `value` is 0, we can avoid the `calc` altogether.
            if (value.value == 0)
                return Percentage { 100 };

            // Turn this into a calc expression: `calc(100% - value)`.
            return Calc { Calculation::subtract(Calculation::percentage(100), copyCalculation(value)) };
        },
        [&](const Percentage& value) -> Result {
            // If `value` is a percentage, we can avoid the `calc` altogether.
            return Percentage { 100 - value.value };
        },
        [&](const Calc& value) -> Result {
            // Turn this into a calc expression: `calc(100% - value)`.
            return Calc { Calculation::subtract(Calculation::percentage(100), copyCalculation(value)) };
        }
    );
}

// Merges the two ranges, `aR` and `bR`, creating a union of their ranges.
consteval CSS::Range mergeRanges(CSS::Range aR, CSS::Range bR)
{
    return CSS::Range { std::min(aR.min, bR.min), std::max(aR.max, bR.max) };
}

// Convert to `calc(100% - (a + b))`.
//
// Returns a LengthPercentage with range, `resultR`, equal to union of the two input ranges `aR` and `bR`.
template<auto aR, auto bR, typename V> auto reflectSum(const LengthPercentage<aR, V>& a, const LengthPercentage<bR, V>& b) -> LengthPercentage<mergeRanges(aR, bR), V>
{
    constexpr auto resultR = mergeRanges(aR, bR);

    using Result = LengthPercentage<resultR, V>;
    using PercentageResult = typename Result::Percentage;
    using CalcResult = typename Result::Calc;
    using PercentageA = typename LengthPercentage<aR, V>::Percentage;
    using PercentageB = typename LengthPercentage<bR, V>::Percentage;

    bool aIsZero = a.isZero();
    bool bIsZero = b.isZero();

    // If both `a` and `b` are 0, turn this into a calc expression: `calc(100% - (0 + 0))` aka `100%`.
    if (aIsZero && bIsZero)
        return PercentageResult { 100 };

    // If just `a` is 0, we can just consider the case of `calc(100% - b)`.
    if (aIsZero) {
        return WTF::switchOn(b,
            [&](const PercentageB& b) -> Result {
                // And if `b` is a percent, we can avoid the `calc` altogether.
                return PercentageResult { 100 - b.value };
            },
            [&](const auto& b) -> Result {
                // Otherwise, turn this into a calc expression: `calc(100% - b)`.
                return CalcResult { Calculation::subtract(Calculation::percentage(100), copyCalculation(b)) };
            }
        );
    }

    // If just `b` is 0, we can just consider the case of `calc(100% - a)`.
    if (bIsZero) {
        return WTF::switchOn(a,
            [&](const PercentageA& a) -> Result {
                // And if `a` is a percent, we can avoid the `calc` altogether.
                return PercentageResult { 100 - a.value };
            },
            [&](const auto& a) -> Result {
                // Otherwise, turn this into a calc expression: `calc(100% - a)`.
                return CalcResult { Calculation::subtract(Calculation::percentage(100), copyCalculation(a)) };
            }
        );
    }

    // If both and `a` and `b` are percentages, we can avoid the `calc` altogether.
    if (WTF::holdsAlternative<PercentageA>(a) && WTF::holdsAlternative<PercentageB>(b))
        return PercentageResult { 100 - (get<PercentageA>(a).value + get<PercentageB>(b).value) };

    // Otherwise, turn this into a calc expression: `calc(100% - (a + b))`.
    return CalcResult { Calculation::subtract(Calculation::percentage(100), Calculation::add(copyCalculation(a), copyCalculation(b))) };
}

} // namespace Style
} // namespace WebCore
