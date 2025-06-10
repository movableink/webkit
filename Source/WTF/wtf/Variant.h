/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include <utility>
#include <variant>

namespace WTF {
template<typename... Ts> using Variant = std::variant<Ts...>;
template<typename T> constexpr std::in_place_type_t<T> InPlaceType { };
template<typename T> using InPlaceTypeT = std::in_place_type_t<T>;
template<size_t I> constexpr std::in_place_index_t<I> InPlaceIndex { };
template<size_t I> using InPlaceIndexT = std::in_place_index_t<I>;
template <size_t I, class T> struct VariantAlternative;
template<size_t I, typename... Types> struct VariantAlternative<I, Variant<Types...>> : std::variant_alternative<I, Variant<Types...>> { };
template<size_t I, typename T> using VariantAlternativeT = typename VariantAlternative<I, T>::type;
template<typename T> struct VariantSize;
template<typename... Types> struct VariantSize<Variant<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> { };
template<typename T> struct VariantSize<const T> : VariantSize<T> { };
template<typename T> constexpr size_t VariantSizeV = VariantSize<T>::value;

template<typename Visitor, typename... Variants> constexpr auto visit(Visitor&& v, Variants&&... values)
    -> decltype(std::visit<Visitor, Variants...>(std::forward<Visitor>(v), std::forward<Variants>(values)...))
{
    return std::visit<Visitor, Variants...>(std::forward<Visitor>(v), std::forward<Variants>(values)...);
}

}
using WTF::Variant;
