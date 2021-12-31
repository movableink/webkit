/*
 * Copyright (C) 2008-2018 Apple Inc. All rights reserved.
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

#include "CachedResourceClient.h"
#include "CachedResourceHandle.h"
#include "CachedScript.h"
#include "CachedScriptFetcher.h"
#include "LoaderStrategy.h"
#include "PlatformStrategies.h"
#include <JavaScriptCore/BytecodeCacheMetadata.h>
#include <JavaScriptCore/SourceProvider.h>

namespace WebCore {

class CachedScriptSourceProvider : public JSC::SourceProvider, public CachedResourceClient {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static Ref<CachedScriptSourceProvider> create(CachedScript* cachedScript, JSC::SourceProviderSourceType sourceType, Ref<CachedScriptFetcher>&& scriptFetcher) { return adoptRef(*new CachedScriptSourceProvider(cachedScript, sourceType, WTFMove(scriptFetcher))); }

    virtual ~CachedScriptSourceProvider()
    {
        commitCachedBytecode();
        m_cachedScript->removeClient(*this);
    }

    unsigned hash() const override { return m_cachedScript->scriptHash(); }
    StringView source() const override { return m_cachedScript->script(); }
    RefPtr<JSC::CachedBytecode> cachedBytecode() const override { return m_cachedScript->cachedBytecode(); }
    void commitCachedBytecode() const override;

    void cacheBytecode(const JSC::BytecodeCacheGenerator&) const override;
    void updateCache(const JSC::UnlinkedFunctionExecutable*, const JSC::SourceCode&, JSC::CodeSpecializationKind, const JSC::UnlinkedFunctionCodeBlock*) const override;

private:
    void commitBytecodeCache();

    CachedScriptSourceProvider(CachedScript* cachedScript, JSC::SourceProviderSourceType sourceType, Ref<CachedScriptFetcher>&& scriptFetcher)
        : SourceProvider(JSC::SourceOrigin { cachedScript->response().url(), WTFMove(scriptFetcher) }, URL(cachedScript->response().url()), TextPosition(), sourceType)
        , m_cachedScript(cachedScript)
    {
        m_cachedScript->addClient(*this);
    }

    mutable bool m_shouldCache { false };
    mutable bool m_hasUpdates { false };
    mutable JSC::BytecodeCacheMetadata m_bytecodeCacheMetadata;
    CachedResourceHandle<CachedScript> m_cachedScript;
};

} // namespace WebCore
