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

#include "CSSPrimitiveNumericTypes.h"
#include "StyleUnevaluatedCalculation.h"
#include "StyleValueTypes.h"
#include <wtf/CompactVariant.h>

namespace WebCore {
namespace Style {

// Concept for use in generic contexts to filter on Numeric Style types.
template<typename T> concept StyleNumeric = requires {
    typename T::CSS;
    typename T::Raw;
};

// Concept for use in generic contexts to filter on base, single type
// primitives, excluding "dimension-percentage" types.
template<typename T> concept StyleNumericPrimitive = StyleNumeric<T> && requires {
    T::unit;
};

// Concept for use in generic contexts to filter on "dimension-percentage"
// types such as `<length-percentage>`.
template<typename T> concept StyleDimensionPercentage = StyleNumeric<T> && requires {
    typename T::Dimension;
};

// MARK: Number Primitive

template<CSS::Range R = CSS::All, typename T = int> struct Integer {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_INTEGER;
    using CSS = WebCore::CSS::Integer<R, T>;
    using Raw = WebCore::CSS::IntegerRaw<R, T>;
    using ValueType = T;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Integer<R, T>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

template<CSS::Range R = CSS::All> struct Number {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_NUMBER;
    using CSS = WebCore::CSS::Number<R>;
    using Raw = WebCore::CSS::NumberRaw<R>;
    using ValueType = double;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Number<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

// MARK: Percentage Primitive

template<CSS::Range R = CSS::All> struct Percentage {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_PERCENTAGE;
    using CSS = WebCore::CSS::Percentage<R>;
    using Raw = WebCore::CSS::PercentageRaw<R>;
    using ValueType = double;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Percentage<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

// MARK: Dimension Primitives

template<CSS::Range R = CSS::All> struct Angle {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_DEG;
    using CSS = WebCore::CSS::Angle<R>;
    using Raw = WebCore::CSS::AngleRaw<R>;
    using ValueType = double;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Angle<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

template<CSS::Range R = CSS::All> struct Length {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_PX;
    using CSS = WebCore::CSS::Length<R>;
    using Raw = WebCore::CSS::LengthRaw<R>;

    // NOTE: Unlike the other primitive numeric types, Length<> uses a `float`,
    // not a `double` for its value type.
    using ValueType = float;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Length<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

template<CSS::Range R = CSS::All> struct Time {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_S;
    using CSS = WebCore::CSS::Time<R>;
    using Raw = WebCore::CSS::TimeRaw<R>;
    using ValueType = double;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Time<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

template<CSS::Range R = CSS::All> struct Frequency {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_HZ;
    using CSS = WebCore::CSS::Frequency<R>;
    using Raw = WebCore::CSS::FrequencyRaw<R>;
    using ValueType = double;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Frequency<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

template<CSS::Range R = CSS::Nonnegative> struct Resolution {
    static_assert(R.min >= 0, "<resolution> values must always have a minimum range of at least 0.");
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_DPPX;
    using CSS = WebCore::CSS::Resolution<R>;
    using Raw = WebCore::CSS::ResolutionRaw<R>;
    using ValueType = double;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Resolution<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

template<CSS::Range R = CSS::All> struct Flex {
    static constexpr auto range = R;
    static constexpr auto unit = CSSUnitType::CSS_FR;
    using CSS = WebCore::CSS::Flex<R>;
    using Raw = WebCore::CSS::FlexRaw<R>;
    using ValueType = double;

    ValueType value { 0 };

    constexpr bool isZero() const { return !value; }

    constexpr bool operator==(const Flex<R>&) const = default;
    constexpr bool operator==(ValueType other) const { return value == other; };
};

} // namespace Style
} // namespace WebCore

namespace WTF {
 
// Allow primitives numeric types that usually store their value as a `double` to be
// used with CompactVariant by using a `float`, rather than `double` representation
// when used in a `CompactVariant`.

template<WebCore::Style::StyleNumericPrimitive T>
    requires std::same_as<typename T::ValueType, double>
struct CompactVariantTraits<T> {
   static constexpr bool hasAlternativeRepresentation = true;

   static constexpr uint64_t encodeFromArguments(double value)
   {
       return static_cast<uint64_t>(std::bit_cast<uint32_t>(clampTo<float>(value)));
   }

   static constexpr uint64_t encode(const T& value)
   {
       return static_cast<uint64_t>(std::bit_cast<uint32_t>(clampTo<float>(value.value)));
   }

   static constexpr T decode(uint64_t value)
   {
       return { std::bit_cast<float>(static_cast<uint32_t>(value)) };
   }
};

} // namespace WTF

namespace WebCore {
namespace Style {

template<CSS::Range R, typename D, typename C> struct DimensionPercentage {
    static constexpr auto range = R;
    using CSS = C;
    using Raw = typename CSS::Raw;
    using Dimension = D;
    using Percentage = Style::Percentage<R>;
    using Calc = UnevaluatedCalculation<R, CSS::category>;
    using Representation = CompactVariant<Dimension, Percentage, Calc>;

    DimensionPercentage(Dimension dimension)
        : m_value { WTFMove(dimension) }
    {
    }

    DimensionPercentage(Percentage percentage)
        : m_value { WTFMove(percentage) }
    {
    }

    DimensionPercentage(Calc calc)
        : m_value { WTFMove(calc) }
    {
    }

    DimensionPercentage(Ref<CalculationValue> calculationValue)
        : m_value { Calc { WTFMove(calculationValue) } }
    {
    }

    DimensionPercentage(Calculation::Child calculationValue)
        : m_value { Calc { WTFMove(calculationValue) } }
    {
    }

    // NOTE: CalculatedValue is intentionally not part of IPCData.
    using IPCData = std::variant<Dimension, Percentage>;
    DimensionPercentage(IPCData&& data)
        : m_value { WTF::switchOn(WTFMove(data), [&](auto&& data) -> Representation { return { WTFMove(data) }; }) }
    {
    }

    IPCData ipcData() const
    {
        return WTF::switchOn(m_value,
            [](const Dimension& dimension) -> IPCData { return dimension; },
            [](const Percentage& percentage) -> IPCData { return percentage; },
            [](const Calc&) -> IPCData { ASSERT_NOT_REACHED(); return Dimension { 0 }; }
        );
    }

    constexpr size_t index() const { return m_value.index(); }

    template<typename T> constexpr bool holdsAlternative() const { return WTF::holdsAlternative<T>(m_value); }
    template<size_t I> constexpr bool holdsAlternative() const { return WTF::holdsAlternative<I>(m_value); }

    template<typename T> T get() const
    {
        return WTF::switchOn(m_value,
            []<std::same_as<T> U>(const U& alternative) -> T { return alternative; },
            [](const auto&) -> T { RELEASE_ASSERT_NOT_REACHED(); }
        );
    }

    template<typename... F> decltype(auto) switchOn(F&&... functors) const
    {
        return WTF::switchOn(m_value, std::forward<F>(functors)...);
    }

    constexpr bool isZero() const
    {
        return WTF::switchOn(m_value,
            []<HasIsZero T>(const T& alternative) { return alternative.isZero(); },
            [](const auto&) { return false; }
        );
    }

    bool operator==(const DimensionPercentage&) const = default;

private:
    Representation m_value;
};

template<CSS::Range R = CSS::All> struct AnglePercentage : DimensionPercentage<R, Angle<R>, CSS::AnglePercentage<R>> {
    using DimensionPercentage<R, Angle<R>, CSS::AnglePercentage<R>>::DimensionPercentage;
};

template<CSS::Range R = CSS::All> struct LengthPercentage : DimensionPercentage<R, Length<R>, CSS::LengthPercentage<R>> {
    using DimensionPercentage<R, Length<R>, CSS::LengthPercentage<R>>::DimensionPercentage;
};

template<typename T, StyleDimensionPercentage D> constexpr bool holdsAlternative(const D& dimensionPercentage)
{
    return WTF::holdsAlternative<T>(dimensionPercentage);
}

template<size_t I, StyleDimensionPercentage D> constexpr bool holdsAlternative(const D& dimensionPercentage)
{
    return WTF::holdsAlternative<I>(dimensionPercentage);
}

template<typename T, StyleDimensionPercentage D> T get(const D& dimensionPercentage)
{
    return dimensionPercentage.template get<T>();
}

// MARK: Additional Common Type and Groupings

// NOTE: This is spelled with an explicit "Or" to distinguish it from types like AnglePercentage/LengthPercentage that have behavior distinctions beyond just being a union of the two types (specifically, calc() has specific behaviors for those types).
template<CSS::Range nR = CSS::All, CSS::Range pR = nR> struct NumberOrPercentage {
    NumberOrPercentage(std::variant<Number<nR>, Percentage<pR>> value)
    {
        WTF::switchOn(WTFMove(value), [this](auto&& alternative) { this->value = WTFMove(alternative); });
    }

    NumberOrPercentage(Number<nR> value)
        : value { WTFMove(value) }
    {
    }

    NumberOrPercentage(Percentage<pR> value)
        : value { WTFMove(value) }
    {
    }

    bool operator==(const NumberOrPercentage&) const = default;

    template<typename... F> decltype(auto) switchOn(F&&... f) const
    {
        auto visitor = WTF::makeVisitor(std::forward<F>(f)...);
        using ResultType = decltype(visitor(std::declval<Number<nR>>()));

        return WTF::switchOn(value,
            [](EmptyToken) -> ResultType {
                RELEASE_ASSERT_NOT_REACHED();
            },
            [&](const Number<nR>& number) -> ResultType {
                return visitor(number);
            },
            [&](const Percentage<pR>& percentage) -> ResultType {
                return visitor(percentage);
            }
        );
    }

    struct MarkableTraits {
        static bool isEmptyValue(const NumberOrPercentage& value) { return value.isEmpty(); }
        static NumberOrPercentage emptyValue() { return NumberOrPercentage(EmptyToken()); }
    };

private:
    struct EmptyToken { constexpr bool operator==(const EmptyToken&) const = default; };
    NumberOrPercentage(EmptyToken token)
        : value { WTFMove(token) }
    {
    }

    bool isEmpty() const { return std::holds_alternative<EmptyToken>(value); }

    std::variant<EmptyToken, Number<nR>, Percentage<pR>> value;
};


template<CSS::Range nR = CSS::All, CSS::Range pR = nR> struct NumberOrPercentageResolvedToNumber {
    Number<nR> value { 0 };

    constexpr NumberOrPercentageResolvedToNumber(typename Number<nR>::ValueType value)
        : value { value }
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(Number<nR> number)
        : value { number }
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(Percentage<pR> percentage)
        : value { percentage.value / 100.0 }
    {
    }

    constexpr bool isZero() const { return value.isZero(); }

    constexpr bool operator==(const NumberOrPercentageResolvedToNumber<nR, pR>&) const = default;
    constexpr bool operator==(typename Number<nR>::ValueType other) const { return value.value == other; };
};

// Standard Numbers
using NumberAll = Number<CSS::All>;
using NumberNonnegative = Number<CSS::Nonnegative>;

// Standard Angles
using AngleAll = Length<CSS::All>;

// Standard Lengths
using LengthAll = Length<CSS::All>;
using LengthNonnegative = Length<CSS::Nonnegative>;

// Standard LengthPercentages
using LengthPercentageAll = LengthPercentage<CSS::All>;
using LengthPercentageNonnegative = LengthPercentage<CSS::Nonnegative>;

// Standard Percentages
using Percentage0To100 = LengthPercentage<CSS::Range{0,100}>;

// Standard Points
using LengthPercentageSpaceSeparatedPointAll = SpaceSeparatedPoint<LengthPercentageAll>;
using LengthPercentageSpaceSeparatedPointNonnegative = SpaceSeparatedPoint<LengthPercentageNonnegative>;

// Standard Sizes
using LengthPercentageSpaceSeparatedSizeAll = SpaceSeparatedSize<LengthPercentageAll>;
using LengthPercentageSpaceSeparatedSizeNonnegative = SpaceSeparatedSize<LengthPercentageNonnegative>;

// MARK: CSS type -> Style type mapping

template<auto R, typename T> struct ToStyleMapping<CSS::Integer<R, T>> { using type = Integer<R, T>; };
template<auto R> struct ToStyleMapping<CSS::Number<R>>                 { using type = Number<R>; };
template<auto R> struct ToStyleMapping<CSS::Percentage<R>>             { using type = Percentage<R>; };
template<auto R> struct ToStyleMapping<CSS::Angle<R>>                  { using type = Angle<R>; };
template<auto R> struct ToStyleMapping<CSS::Length<R>>                 { using type = Length<R>; };
template<auto R> struct ToStyleMapping<CSS::Time<R>>                   { using type = Time<R>; };
template<auto R> struct ToStyleMapping<CSS::Frequency<R>>              { using type = Frequency<R>; };
template<auto R> struct ToStyleMapping<CSS::Resolution<R>>             { using type = Resolution<R>; };
template<auto R> struct ToStyleMapping<CSS::Flex<R>>                   { using type = Flex<R>; };
template<auto R> struct ToStyleMapping<CSS::AnglePercentage<R>>        { using type = AnglePercentage<R>; };
template<auto R> struct ToStyleMapping<CSS::LengthPercentage<R>>       { using type = LengthPercentage<R>; };

template<auto nR, auto pR> struct ToStyleMapping<CSS::NumberOrPercentage<nR, pR>>                 { using type = NumberOrPercentage<nR, pR>; };
template<auto nR, auto pR> struct ToStyleMapping<CSS::NumberOrPercentageResolvedToNumber<nR, pR>> { using type = NumberOrPercentageResolvedToNumber<nR, pR>; };

// MARK: Style type mapping -> CSS type mappings

template<StyleNumeric T> struct ToCSSMapping<T>                        { using type = typename T::CSS; };

template<auto nR, auto pR> struct ToCSSMapping<NumberOrPercentage<nR, pR>>                 { using type = CSS::NumberOrPercentage<nR, pR>; };
template<auto nR, auto pR> struct ToCSSMapping<NumberOrPercentageResolvedToNumber<nR, pR>> { using type = CSS::NumberOrPercentageResolvedToNumber<nR, pR>; };

} // namespace Style

namespace CSS {
namespace TypeTransform {

// MARK: - Type transforms

namespace Type {

// MARK: Transform CSS type -> Style type.

// Transform `css1`  -> `style1`
template<typename T> struct CSSToStyleLazy {
    using type = typename Style::ToStyleMapping<T>::type;
};
template<typename T> using CSSToStyle = typename CSSToStyleLazy<T>::type;

// MARK: Transform Raw type -> Style type.

// Transform `raw1`  -> `style1`
template<typename T> struct RawToStyleLazy {
    using type = typename Style::ToStyleMapping<typename RawToCSSMapping<T>::CSS>::type;
};
template<typename T> using RawToStyle = typename RawToStyleLazy<T>::type;

} // namespace Type

namespace List {

// MARK: - List transforms

// MARK: Transform CSS type list -> Style type list.

// Transform `brigand::list<css1, css2, ...>`  -> `brigand::list<style1, style2, ...>`
template<typename TypeList> struct CSSToStyleLazy {
    using type = brigand::transform<TypeList, Type::CSSToStyleLazy<brigand::_1>>;
};
template<typename TypeList> using CSSToStyle = typename CSSToStyleLazy<TypeList>::type;

// MARK: Transform Raw type list -> Style type list.

// Transform `brigand::list<raw1, raw2, ...>`  -> `brigand::list<style1, style2, ...>`
template<typename TypeList> struct RawToStyleLazy {
    using type = brigand::transform<TypeList, Type::RawToStyleLazy<brigand::_1>>;
};
template<typename TypeList> using RawToStyle = typename RawToStyleLazy<TypeList>::type;

} // namespace List
} // namespace TypeTransform

} // namespace CSS
} // namespace WebCore

template<WebCore::Style::StyleDimensionPercentage T> inline constexpr auto WebCore::TreatAsVariantLike<T> = true;
template<auto nR, auto pR> inline constexpr auto WebCore::TreatAsVariantLike<WebCore::Style::NumberOrPercentage<nR, pR>> = true;
