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

 #include "config.h"
 #include "BytecodeCacheMetadata.h"

 #include "SourceCode.h"

 namespace JSC {

 void BytecodeCacheMetadata::recordCachedFunctionCodeBlock(const SourceCode& sourceCode, CodeSpecializationKind kind)
 {
     auto result = m_map.add(sourceCode.startOffset(), Entry { });
     switch (kind) {
     case CodeForCall:
         result.iterator->value.m_codeForCall = true;
         break;
     case CodeForConstruct:
         result.iterator->value.m_codeForConstrut = true;
         break;
     }
 }

 bool BytecodeCacheMetadata::shouldCache(const SourceCode& sourceCode, CodeSpecializationKind kind) const
 {
     auto iterator = m_map.find(sourceCode.startOffset());
     if (iterator == m_map.end())
         return false;

     switch (kind) {
     case CodeForCall:
         return iterator->value.m_codeForCall;
         break;
     case CodeForConstruct:
         return iterator->value.m_codeForConstrut;
         break;
     }
 }

 BytecodeCacheMetadata BytecodeCacheMetadata::decode(const uint8_t* data, size_t size)
 {
     ASSERT(!(size % sizeof(EncodedEntry)));
     size_t length = size / sizeof(EncodedEntry);
     EncodedEntry* entries = bitwise_cast<EncodedEntry*>(data);
     BytecodeCacheMetadata metadata;
     for (unsigned i = 0; i < length; i++)
         metadata.m_map.add(entries[i].key, entries[i].value);
     return metadata;
 }

 std::pair<MallocPtr<uint8_t>, size_t> BytecodeCacheMetadata::encode() const
 {
     size_t size = m_map.size() * sizeof(EncodedEntry);
     MallocPtr<uint8_t> data = MallocPtr<uint8_t>::malloc(size);
     unsigned offset = 0;
     for (const EncodedEntry& entry : m_map) {
         uint8_t* ptr = data.get() + (offset * sizeof(EncodedEntry));
         memcpy(ptr, &entry, sizeof(entry));
         ++offset;
     }
     return std::make_pair(WTFMove(data), size);
 }

 } // namespace JSC
