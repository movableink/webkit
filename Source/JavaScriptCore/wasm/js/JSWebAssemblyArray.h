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

#include "WasmFormat.h"
#include "WasmOps.h"
#include "WasmTypeDefinition.h"
#include "WebAssemblyGCObjectBase.h"

WTF_ALLOW_UNSAFE_BUFFER_USAGE_BEGIN

namespace JSC {

// Ideally this would just subclass TrailingArray<JSWebAssemblyArray, uint8_t> but we need the m_size field to be in units
// of element size rather than byte size.
class JSWebAssemblyArray final : public WebAssemblyGCObjectBase {
public:
    using Base = WebAssemblyGCObjectBase;
    static constexpr DestructionMode needsDestruction = NeedsDestruction;

    static void destroy(JSCell*);

    template<typename CellType, SubspaceAccess mode>
    static CompleteSubspace* subspaceFor(VM& vm)
    {
        return vm.heap.webAssemblyArraySpace<mode>();
    }

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    static JSWebAssemblyArray* create(VM& vm, Structure* structure, Wasm::FieldType elementType, unsigned size, RefPtr<const Wasm::RTT>&& rtt)
    {
        auto* object = new (NotNull, allocateCell<JSWebAssemblyArray>(vm, allocationSizeInBytes(elementType, size))) JSWebAssemblyArray(vm, structure, elementType, size, WTFMove(rtt));
        object->finishCreation(vm);
        return object;
    }

    DECLARE_VISIT_CHILDREN;

    Wasm::FieldType elementType() const { return m_elementType; }
    static bool needsAlignmentCheck(Wasm::StorageType type) { return type.unpacked().isV128(); }
    size_t size() const { return m_size; }
    size_t sizeInBytes() const { return size() * elementType().type.elementSize(); }

    template<typename T>
    std::span<T> span() LIFETIME_BOUND
    {
        ASSERT(sizeof(T) == elementType().type.elementSize());
        uint8_t* data = this->data();
        if constexpr (std::is_same_v<T, v128_t>)
            data += isPreciseAllocation() ? PreciseAllocation::halfAlignment : 0;
        else
            ASSERT(!needsAlignmentCheck(elementType().type));
        return { std::bit_cast<T*>(data), size() };
    }

    template<typename T>
    std::span<const T> span() const LIFETIME_BOUND { return const_cast<JSWebAssemblyArray*>(this)->span<T>(); }

    bool elementsAreRefTypes() const
    {
        return Wasm::isRefType(m_elementType.type.unpacked());
    }

    std::span<uint64_t> refTypeSpan() LIFETIME_BOUND
    {
        ASSERT(elementsAreRefTypes());
        return span<uint64_t>();
    }

    ALWAYS_INLINE auto visitSpan(auto functor)
    {
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
            return functor(span<v128_t>());
        default:
            return functor(span<uint64_t>());
        }
    }

    ALWAYS_INLINE auto visitSpanNonVector(auto functor)
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

    uint64_t get(uint32_t index)
    {
        // V128 is not supported in LLInt.
        return visitSpanNonVector([&](auto span) ALWAYS_INLINE_LAMBDA -> uint64_t {
            return span[index];
        });
    }

    void set(VM& vm, uint32_t index, uint64_t value)
    {
        visitSpanNonVector([&]<typename T>(std::span<T> span) ALWAYS_INLINE_LAMBDA {
            span[index] = static_cast<T>(value);
            if (elementsAreRefTypes())
                vm.writeBarrier(this);
        });
    }

    void set(VM&, uint32_t index, v128_t value)
    {
        ASSERT(m_elementType.type.is<Wasm::Type>());
        ASSERT(m_elementType.type.as<Wasm::Type>().kind == Wasm::TypeKind::V128);
        span<v128_t>()[index] = value;
    }

    void fill(VM&, uint32_t, uint64_t, uint32_t);
    void fill(VM&, uint32_t, v128_t, uint32_t);
    void copy(VM&, JSWebAssemblyArray&, uint32_t, uint32_t, uint32_t);

    // We add 8 bytes for v128 arrays since a PreciseAllocation will have the wrong alignment as the base pointer for a PreciseAllocation is shifted by 8.
    // Note: Technically this isn't needed since the GC/malloc always allocates 16 byte chunks so for precise allocations
    // there will be a 8 spare bytes at the end. This is just a bit more explicit and shouldn't make a difference.
    static size_t allocationSizeInBytes(Wasm::FieldType fieldType, unsigned size) { return sizeof(JSWebAssemblyArray) + size * fieldType.type.elementSize() + (needsAlignmentCheck(fieldType.type) * 8); }
    static constexpr ptrdiff_t offsetOfSize() { return OBJECT_OFFSETOF(JSWebAssemblyArray, m_size); }
    static constexpr ptrdiff_t offsetOfData() { return sizeof(JSWebAssemblyArray); }

private:
    friend class LLIntOffsetsExtractor;
    std::span<uint8_t> bytes() { return { isPreciseAllocation() ? data() + PreciseAllocation::halfAlignment : data(), sizeInBytes() }; }
    uint8_t* data() { return reinterpret_cast<uint8_t*>(this) + offsetOfData(); }
    const uint8_t* data() const { return const_cast<JSWebAssemblyArray*>(this)->data(); }

    JSWebAssemblyArray(VM&, Structure*, Wasm::FieldType, unsigned, RefPtr<const Wasm::RTT>&&);

    DECLARE_DEFAULT_FINISH_CREATION;

    Wasm::FieldType m_elementType;
    unsigned m_size;
};

static_assert(std::is_final_v<JSWebAssemblyArray>, "JSWebAssemblyArray is a TrailingArray-like object so must know about all members");
// We still have to check for PreciseAllocations since those are shifted by 8 bytes for v128 but this asserts our shifted offset will be correct.
static_assert(!(JSWebAssemblyArray::offsetOfData() % alignof(v128_t)), "JSWebAssemblyArray storage needs to be aligned for v128_t");

} // namespace JSC

WTF_ALLOW_UNSAFE_BUFFER_USAGE_END

#endif // ENABLE(WEBASSEMBLY)
