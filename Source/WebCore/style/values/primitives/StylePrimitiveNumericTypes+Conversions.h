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

#include "CSSPrimitiveNumericTypes+Canonicalization.h"
#include "CSSPrimitiveNumericTypes+ComputedStyleDependencies.h"
#include "CSSToLengthConversionData.h"
#include "FloatConversion.h"
#include "StyleBuilderState.h"
#include "StylePrimitiveNumericTypes.h"

namespace WebCore {
namespace Style {

// MARK: - Conversion Data specialization

template<typename T> struct ConversionDataSpecializer {
    CSSToLengthConversionData operator()(const BuilderState& state)
    {
        return state.cssToLengthConversionData();
    }
};

template<auto R> struct ConversionDataSpecializer<CSS::LengthRaw<R>> {
    CSSToLengthConversionData operator()(const BuilderState& state)
    {
        return state.useSVGZoomRulesForLength()
             ? state.cssToLengthConversionData().copyWithAdjustedZoom(1.0f)
             : state.cssToLengthConversionData();
    }
};

template<typename T> CSSToLengthConversionData conversionData(const BuilderState& state)
{
    return ConversionDataSpecializer<T>{}(state);
}

// MARK: - Raw canonicalization

template<auto R, typename T, typename... Rest> constexpr Integer<R, T> canonicalize(const CSS::IntegerRaw<R, T>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { raw.value };
}

template<auto R, typename T, typename... Rest> constexpr Integer<R, T> canonicalize(const CSS::IntegerRaw<R, T>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> constexpr Number<R> canonicalize(const CSS::NumberRaw<R>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { raw.value };
}

template<auto R, typename... Rest> constexpr Number<R> canonicalize(const CSS::NumberRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> constexpr Percentage<R> canonicalize(const CSS::PercentageRaw<R>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { raw.value };
}

template<auto R, typename... Rest> constexpr Percentage<R> canonicalize(const CSS::PercentageRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> Angle<R> canonicalize(const CSS::AngleRaw<R>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { CSS::canonicalizeAngle(raw.value, raw.type) };
}

template<auto R, typename... Rest> Angle<R> canonicalize(const CSS::AngleRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> Length<R> canonicalize(const CSS::LengthRaw<R>& raw, NoConversionDataRequiredToken token, Rest&&... rest)
{
    ASSERT(!requiresConversionData(raw));

    return { CSS::canonicalizeAndClampLength(raw.value, raw.type, token, std::forward<Rest>(rest)...) };
}

template<auto R, typename... Rest> Length<R> canonicalize(const CSS::LengthRaw<R>& raw, const CSSToLengthConversionData& conversionData, Rest&&...)
{
    ASSERT(CSS::collectComputedStyleDependencies(raw).canResolveDependenciesWithConversionData(conversionData));

    return { CSS::canonicalizeAndClampLength(raw.value, raw.type, conversionData) };
}

template<auto R, typename... Rest> Time<R> canonicalize(const CSS::TimeRaw<R>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { CSS::canonicalizeTime(raw.value, raw.type) };
}

template<auto R, typename... Rest> Time<R> canonicalize(const CSS::TimeRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> Frequency<R> canonicalize(const CSS::FrequencyRaw<R>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { CSS::canonicalizeFrequency(raw.value, raw.type) };
}

template<auto R, typename... Rest> Frequency<R> canonicalize(const CSS::FrequencyRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> Resolution<R> canonicalize(const CSS::ResolutionRaw<R>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { CSS::canonicalizeResolution(raw.value, raw.type) };
}

template<auto R, typename... Rest> Resolution<R> canonicalize(const CSS::ResolutionRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> constexpr Flex<R> canonicalize(const CSS::FlexRaw<R>& raw, NoConversionDataRequiredToken, Rest&&...)
{
    return { raw.value };
}

template<auto R, typename... Rest> constexpr Flex<R> canonicalize(const CSS::FlexRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> AnglePercentage<R> canonicalize(const CSS::AnglePercentageRaw<R>& raw, NoConversionDataRequiredToken token, Rest&&... rest)
{
    if (raw.type == CSSUnitType::CSS_PERCENTAGE)
        return { canonicalize(CSS::PercentageRaw<R> { raw.value }, token, std::forward<Rest>(rest)...) };
    return { canonicalize(CSS::AngleRaw<R> { raw.type, raw.value }, token, std::forward<Rest>(rest)...) };
}

template<auto R, typename... Rest> AnglePercentage<R> canonicalize(const CSS::AnglePercentageRaw<R>& raw, const CSSToLengthConversionData&, Rest&&... rest)
{
    return canonicalize(raw, NoConversionDataRequiredToken { }, std::forward<Rest>(rest)...);
}

template<auto R, typename... Rest> LengthPercentage<R> canonicalize(const CSS::LengthPercentageRaw<R>& raw, NoConversionDataRequiredToken token, Rest&&... rest)
{
    if (raw.type == CSSUnitType::CSS_PERCENTAGE)
        return canonicalize(CSS::PercentageRaw<R> { raw.value }, token, std::forward<Rest>(rest)...);

    // NOTE: This uses the non-clamping version length canonicalization to match the behavior of CSSPrimitiveValue::convertToLength().
    return Length<R> { narrowPrecisionToFloat(CSS::canonicalizeLength(raw.value, raw.type, token)) };
}

template<auto R, typename... Rest> LengthPercentage<R> canonicalize(const CSS::LengthPercentageRaw<R>& raw, const CSSToLengthConversionData& conversionData, Rest&&... rest)
{
    ASSERT(CSS::collectComputedStyleDependencies(raw).canResolveDependenciesWithConversionData(conversionData));

    if (raw.type == CSSUnitType::CSS_PERCENTAGE)
        return canonicalize(CSS::PercentageRaw<R> { raw.value }, conversionData, std::forward<Rest>(rest)...);

    // NOTE: This uses the non-clamping version length canonicalization to match the behavior of CSSPrimitiveValue::convertToLength().
    return Length<R> { narrowPrecisionToFloat(CSS::canonicalizeLength(raw.value, raw.type, conversionData)) };
}

// MARK: - Conversion from "Style to "CSS"

// Out of line to avoid inclusion of CSSCalcValue.h
Ref<CSSCalcValue> makeCalc(const CalculationValue&, const RenderStyle&);
// Out of line to avoid inclusion of RenderStyleInlines.h
float adjustForZoom(float, const RenderStyle&);

// Length requires a specialized implementation due to zoom adjustment.
template<auto R> struct ToCSS<Length<R>> {
    auto operator()(const Length<R>& value, const RenderStyle& style) -> CSS::Length<R>
    {
        return CSS::LengthRaw<R> { value.unit, adjustForZoom(value.value, style) };
    }
};

// AnglePercentage / LengthPercentage require specialized implementations due to additional `calc` field.
template<auto R> struct ToCSS<AnglePercentage<R>> {
    auto operator()(const AnglePercentage<R>& value, const RenderStyle& style) -> CSS::AnglePercentage<R>
    {
        return WTF::switchOn(value,
            [&](Angle<R> angle) -> CSS::AnglePercentage<R> {
                return CSS::AnglePercentageRaw<R> { angle.unit, angle.value };
            },
            [&](Percentage<R> percentage) -> CSS::AnglePercentage<R> {
                return CSS::AnglePercentageRaw<R> { percentage.unit, percentage.value };
            },
            [&](const CalculationValue& calculation) -> CSS::AnglePercentage<R> {
                return CSS::UnevaluatedCalc<CSS::AnglePercentageRaw<R>> { makeCalc(calculation, style) };
            }
        );
    }
};

template<auto R> struct ToCSS<LengthPercentage<R>> {
    auto operator()(const LengthPercentage<R>& value, const RenderStyle& style) -> CSS::LengthPercentage<R>
    {
        return WTF::switchOn(value,
            [&](Length<R> length) -> CSS::LengthPercentage<R> {
                return CSS::LengthPercentageRaw<R> { length.unit, adjustForZoom(length.value, style) };
            },
            [&](Percentage<R> percentage) -> CSS::LengthPercentage<R> {
                return CSS::LengthPercentageRaw<R> { percentage.unit, percentage.value };
            },
            [&](const CalculationValue& calculation) -> CSS::LengthPercentage<R> {
                return CSS::UnevaluatedCalc<CSS::LengthPercentageRaw<R>> { makeCalc(calculation, style) };
            }
        );
    }
};

// Partial specialization for remaining numeric types.
template<StyleNumeric StylePrimitive> struct ToCSS<StylePrimitive> {
    auto operator()(const StylePrimitive& value, const RenderStyle&) -> typename StylePrimitive::CSS
    {
        return { typename StylePrimitive::Raw { value.unit, value.value } };
    }
};

// NumberOrPercentageResolvedToNumber requires specialization due to asymmetric representations.
template<auto nR, auto pR> struct ToCSS<NumberOrPercentageResolvedToNumber<nR, pR>> {
    auto operator()(const NumberOrPercentageResolvedToNumber<nR, pR>& value, const RenderStyle& style) -> CSS::NumberOrPercentageResolvedToNumber<nR, pR>
    {
        return { toCSS(value.value, style) };
    }
};

// MARK: - Conversion from CSS -> Style

// Integer, Length, AnglePercentage and LengthPercentage require specialized implementations for their calc canonicalization.

template<auto R, typename T> struct ToStyle<CSS::UnevaluatedCalc<CSS::IntegerRaw<R, T>>> {
    using From = CSS::UnevaluatedCalc<CSS::IntegerRaw<R, T>>;
    using To = Integer<R, T>;

    template<typename... Rest> auto operator()(const From& value, Rest&&... rest) -> To
    {
        return { roundForImpreciseConversion<T>(CSS::unevaluatedCalcEvaluate(value.protectedCalc(), From::category, std::forward<Rest>(rest)...)) };
    }
};

template<auto R> struct ToStyle<CSS::UnevaluatedCalc<CSS::LengthRaw<R>>> {
    using From = CSS::UnevaluatedCalc<CSS::LengthRaw<R>>;
    using To = Length<R>;

    template<typename... Rest> auto operator()(const From& value, Rest&&... rest) -> To
    {
        return { CSS::clampLengthToAllowedLimits(CSS::unevaluatedCalcEvaluate(value.protectedCalc(), From::category, std::forward<Rest>(rest)...)) };
    }
};

template<auto R> struct ToStyle<CSS::UnevaluatedCalc<CSS::AnglePercentageRaw<R>>> {
    using From = CSS::UnevaluatedCalc<CSS::AnglePercentageRaw<R>>;
    using To = AnglePercentage<R>;

    template<typename... Rest> auto operator()(const From& value, Rest&&... rest) -> To
    {
        Ref calc = value.protectedCalc();

        ASSERT(calc->tree().category == From::category);

        if (!calc->tree().type.percentHint)
            return { Style::Angle<R> { calc->doubleValue(std::forward<Rest>(rest)...) } };
        if (std::holds_alternative<CSSCalc::Percentage>(calc->tree().root))
            return { Style::Percentage<R> { calc->doubleValue(std::forward<Rest>(rest)...) } };
        return { calc->createCalculationValue(std::forward<Rest>(rest)...) };
    }
};

template<auto R> struct ToStyle<CSS::UnevaluatedCalc<CSS::LengthPercentageRaw<R>>> {
    using From = CSS::UnevaluatedCalc<CSS::LengthPercentageRaw<R>>;
    using To = LengthPercentage<R>;

    template<typename... Rest> auto operator()(const From& value, Rest&&... rest) -> To
    {
        Ref calc = value.protectedCalc();

        ASSERT(calc->tree().category == From::category);

        if (!calc->tree().type.percentHint)
            return { Style::Length<R> { CSS::clampLengthToAllowedLimits(calc->doubleValue(std::forward<Rest>(rest)...)) } };
        if (std::holds_alternative<CSSCalc::Percentage>(calc->tree().root))
            return { Style::Percentage<R> { calc->doubleValue(std::forward<Rest>(rest)...) } };
        return { calc->createCalculationValue(std::forward<Rest>(rest)...) };
    }
};

// Partial specialization for remaining numeric types.

template<CSS::RawNumeric RawType> struct ToStyle<RawType> {
    using From = RawType;
    using To = CSS::TypeTransform::Type::RawToStyle<RawType>;

    template<typename... Rest> auto operator()(const From& value, Rest&&... rest) -> To
    {
        return { canonicalize(value, std::forward<Rest>(rest)...) };
    }
};

template<CSS::RawNumeric RawType> struct ToStyle<CSS::UnevaluatedCalc<RawType>> {
    using From = CSS::UnevaluatedCalc<RawType>;
    using To = CSS::TypeTransform::Type::RawToStyle<RawType>;

    template<typename... Rest> auto operator()(const From& value, Rest&&... rest) -> To
    {
        return { CSS::unevaluatedCalcEvaluate(value.protectedCalc(), From::category, std::forward<Rest>(rest)...) };
    }
};

template<CSS::RawNumeric RawType> struct ToStyle<CSS::PrimitiveNumeric<RawType>> {
    using From = CSS::PrimitiveNumeric<RawType>;
    using To = CSS::TypeTransform::Type::RawToStyle<RawType>;

    template<typename... Rest> auto operator()(const From& value, Rest&&... rest) -> To
    {
        return WTF::switchOn(value, [&](const auto& value) { return toStyle(value, std::forward<Rest>(rest)...); });
    }

    // Implement `BuilderState` overload to explicitly forward to the `CSSToLengthConversionData` overload.
    template<typename... Rest> auto operator()(const From& value, const BuilderState& state, Rest&&... rest) -> To
    {
        return toStyle(value, conversionData<typename From::Raw>(state), std::forward<Rest>(rest)...);
    }
};

// NumberOrPercentageResolvedToNumber, as the name implies, resolves its percentage to a number.
template<auto nR, auto pR> struct ToStyle<CSS::NumberOrPercentageResolvedToNumber<nR, pR>> {
    template<typename... Rest> auto operator()(const CSS::NumberOrPercentageResolvedToNumber<nR, pR>& value, Rest&&... rest) -> NumberOrPercentageResolvedToNumber<nR, pR>
    {
        return WTF::switchOn(value,
            [&](CSS::Number<nR> number) -> NumberOrPercentageResolvedToNumber<nR, pR> {
                return { toStyle(number, std::forward<Rest>(rest)...) };
            },
            [&](CSS::Percentage<pR> percentage) -> NumberOrPercentageResolvedToNumber<nR, pR> {
                return { toStyle(percentage, std::forward<Rest>(rest)...).value / 100.0 };
            }
        );
    }
};

} // namespace Style
} // namespace WebCore
