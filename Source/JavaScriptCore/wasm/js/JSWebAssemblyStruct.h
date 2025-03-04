/*
 * Copyright (C) 2022-2025 Apple Inc. All rights reserved.
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

#include "JSObject.h"
#include "WasmTypeDefinitionInlines.h"
#include "WebAssemblyGCObjectBase.h"
#include <wtf/Ref.h>
#include <wtf/TrailingArray.h>

namespace JSC {

class JSWebAssemblyInstance;

class JSWebAssemblyStruct final : public WebAssemblyGCObjectBase, private TrailingArray<JSWebAssemblyStruct, uint8_t> {
public:
    using Base = WebAssemblyGCObjectBase;
    using TrailingArrayType = TrailingArray<JSWebAssemblyStruct, uint8_t>;
    friend TrailingArrayType;
    static constexpr DestructionMode needsDestruction = NeedsDestruction;

    static void destroy(JSCell*);

    template<typename CellType, SubspaceAccess mode>
    static CompleteSubspace* subspaceFor(VM& vm)
    {
        return vm.heap.webAssemblyStructSpace<mode>();
    }

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    static JSWebAssemblyStruct* create(VM&, Structure*, JSWebAssemblyInstance*, uint32_t, RefPtr<const Wasm::RTT>&&);

    DECLARE_VISIT_CHILDREN;

    uint64_t get(uint32_t) const;
    void set(uint32_t, uint64_t);
    void set(uint32_t, v128_t);
    const Wasm::TypeDefinition& typeDefinition() const { return m_type.get(); }
    const Wasm::StructType& structType() const { return *m_type->as<Wasm::StructType>(); }
    Wasm::FieldType fieldType(uint32_t fieldIndex) const { return structType().field(fieldIndex); }

    uint8_t* fieldPointer(uint32_t fieldIndex) { return &at(structType().offsetOfFieldInPayload(fieldIndex)); }
    const uint8_t* fieldPointer(uint32_t fieldIndex) const { return const_cast<JSWebAssemblyStruct*>(this)->fieldPointer(fieldIndex); }

    using TrailingArrayType::offsetOfData;

protected:
    JSWebAssemblyStruct(VM&, Structure*, Ref<const Wasm::TypeDefinition>&&, RefPtr<const Wasm::RTT>&&);
    DECLARE_DEFAULT_FINISH_CREATION;

    // FIXME: It is possible to encode the type information in the structure field of Wasm.Struct and remove this field.
    // https://bugs.webkit.org/show_bug.cgi?id=244838
    Ref<const Wasm::TypeDefinition> m_type;
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
