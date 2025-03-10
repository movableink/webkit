/*
 * Copyright (C) 2022 Igalia S.L. All rights reserved.
 * Copyright (C) 2023-2025 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(WEBASSEMBLY)

#include "HeapCellInlines.h"
#include "JSWebAssemblyArray.h"

WTF_ALLOW_UNSAFE_BUFFER_USAGE_BEGIN

namespace JSC {

template<typename T>
std::span<T> JSWebAssemblyArray::span()
{
    ASSERT(sizeof(T) == elementType().type.elementSize());
    uint8_t* data = this->data();
    if constexpr (std::is_same_v<T, v128_t>)
        data += isPreciseAllocation() ? PreciseAllocation::halfAlignment : 0;
    else
        ASSERT(!needsAlignmentCheck(elementType().type));
    return { std::bit_cast<T*>(data), size() };
}

std::span<uint64_t> JSWebAssemblyArray::refTypeSpan()
{
    ASSERT(elementsAreRefTypes());
    return span<uint64_t>();
}

std::span<uint8_t> JSWebAssemblyArray::bytes()
{
    return { isPreciseAllocation() ? data() + PreciseAllocation::halfAlignment : data(), sizeInBytes() };
}

auto JSWebAssemblyArray::visitSpanNonVector(auto functor)
{
    // Ideally this would have just been:
    // return visitSpan([&][&]<typename T>(std::span<T> span) { RELEASE_ASSERT(!std::is_same<T, v128_t>::value); ... });
    // but that causes weird compiler errors...

    if (m_elementType.type.is<Wasm::PackedType>()) {
        switch (m_elementType.type.as<Wasm::PackedType>()) {
        case Wasm::PackedType::I8:
            return functor(span<uint8_t>());
        case Wasm::PackedType::I16:
            return functor(span<uint16_t>());
        }
    }

    // m_element_type must be a type, so we can get its kind
    ASSERT(m_elementType.type.is<Wasm::Type>());
    switch (m_elementType.type.as<Wasm::Type>().kind) {
    case Wasm::TypeKind::I32:
    case Wasm::TypeKind::F32:
        return functor(span<uint32_t>());
    case Wasm::TypeKind::V128:
        RELEASE_ASSERT_NOT_REACHED();
    default:
        return functor(span<uint64_t>());
    }
}

uint64_t JSWebAssemblyArray::get(uint32_t index)
{
    // V128 is not supported in LLInt.
    return visitSpanNonVector([&](auto span) ALWAYS_INLINE_LAMBDA -> uint64_t {
        return span[index];
    });
}

void JSWebAssemblyArray::set(VM& vm, uint32_t index, uint64_t value)
{
    visitSpanNonVector([&]<typename T>(std::span<T> span) ALWAYS_INLINE_LAMBDA {
        span[index] = static_cast<T>(value);
        if (elementsAreRefTypes())
            vm.writeBarrier(this);
    });
}

void JSWebAssemblyArray::set(VM&, uint32_t index, v128_t value)
{
    ASSERT(m_elementType.type.is<Wasm::Type>());
    ASSERT(m_elementType.type.as<Wasm::Type>().kind == Wasm::TypeKind::V128);
    span<v128_t>()[index] = value;
}


} // namespace JSC

WTF_ALLOW_UNSAFE_BUFFER_USAGE_END

#endif // ENABLE(WEBASSEMBLY)
