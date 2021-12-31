/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

 #include "CodeSpecializationKind.h"
 #include <wtf/HashMap.h>
 #include <wtf/MallocPtr.h>
 #include <wtf/Optional.h>

 namespace JSC {

 class SourceCode;

 class BytecodeCacheMetadata {
     class Entry;

     using EncodedEntry = KeyValuePair<int, Entry>;

 public:

     bool isEmpty() const { return m_map.isEmpty(); }

     JS_EXPORT_PRIVATE void recordCachedFunctionCodeBlock(const SourceCode&, CodeSpecializationKind);
     JS_EXPORT_PRIVATE bool shouldCache(const SourceCode&, CodeSpecializationKind) const;

     JS_EXPORT_PRIVATE static BytecodeCacheMetadata decode(const uint8_t*, size_t);
     JS_EXPORT_PRIVATE std::pair<MallocPtr<uint8_t>, size_t> encode() const;

     template<typename Encoder>
     void encode(Encoder&) const;

     template<typename Decoder>
     static Optional<BytecodeCacheMetadata> decode(Decoder&);

 private:
     class Entry {
         friend class BytecodeCacheMetadata;

     public:
         Entry()
             : m_codeForCall(false)
             , m_codeForConstrut(false)
         {
         }

     private:
         bool m_codeForCall : 1;
         bool m_codeForConstrut : 1;
     };

     HashMap<int, Entry> m_map;
 };

 template<typename Encoder>
 void BytecodeCacheMetadata::encode(Encoder& encoder) const
 {
     std::pair<MallocPtr<uint8_t>, size_t> data = encode();
     encoder << static_cast<uint64_t>(data.second);
     encoder.encodeFixedLengthData(data.first.get(), data.second, alignof(EncodedEntry));
 }

 template<typename Decoder>
 Optional<BytecodeCacheMetadata> BytecodeCacheMetadata::decode(Decoder& decoder)
 {
     uint64_t size;
     if (!decoder.decode(size))
         return WTF::nullopt;
     MallocPtr<uint8_t> data = MallocPtr<uint8_t>::malloc(size);
     if (!decoder.decodeFixedLengthData(data.get(), size, alignof(EncodedEntry)))
         return WTF::nullopt;
     return BytecodeCacheMetadata::decode(data.get(), size);
 }

 } // namespace JSC
