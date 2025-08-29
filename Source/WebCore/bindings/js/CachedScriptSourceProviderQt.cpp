/*
 * Copyright (C) 2025 The Qt Company Ltd.
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
 * THIS SOFTWARE IS PROVIDED BY THE QT COMPANY LTD. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE QT COMPANY LTD. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CachedScriptSourceProvider.h"

#if PLATFORM(QT)

#include <wtf/MallocSpan.h>
#include <wtf/SHA1.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/HexNumber.h>
#include "QtBytecodeCacheDelegate.h"
#include "QtDiskCacheDelegate.h"
#include <JavaScriptCore/BytecodeCacheError.h>
#include <JavaScriptCore/CachedTypes.h>
#include <JavaScriptCore/HeapCellInlines.h>
#include <JavaScriptCore/UnlinkedFunctionExecutable.h>

#include <QByteArray>
#include <QString>

namespace WebCore {

// Forward declaration from QWebSettings
QtBytecodeCacheDelegate* globalBytecodeCacheDelegate();

static QtBytecodeCacheDelegate* getOrCreateDefaultDelegate()
{
    // Check if user provided a custom delegate
    auto* delegate = globalBytecodeCacheDelegate();
    if (delegate)
        return delegate;

    // No default delegate - user must explicitly set one to enable caching
    return nullptr;
}

static QString generateCacheKey(const String& sourceURL)
{
    SHA1 sha1;
    CString urlUtf8 = sourceURL.utf8();
    sha1.addBytes(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(urlUtf8.data()), urlUtf8.length()));
    SHA1::Digest digest;
    sha1.computeHash(digest);

    StringBuilder hexString;
    for (unsigned i = 0; i < 20; ++i)
        hexString.append(hex(digest[i], 2, Lowercase));

    return QString::fromLatin1(hexString.toString().latin1().data());
}

RefPtr<JSC::CachedBytecode> CachedScriptSourceProvider::cachedBytecode() const
{
    if (m_cachedBytecode)
        return m_cachedBytecode;

    auto* delegate = getOrCreateDefaultDelegate();
    if (!delegate) {
        // Create empty cache for accumulating updates even without delegate
        m_cachedBytecode = JSC::CachedBytecode::create();
        return m_cachedBytecode;
    }

    QString key = generateCacheKey(sourceURL());
    QString hashString = QString::number(hash());

    QByteArray data = delegate->loadBytecode(QString(sourceURL()), hashString);
    if (data.isEmpty()) {
        // Create empty cache for accumulating updates
        m_cachedBytecode = JSC::CachedBytecode::create();
        return m_cachedBytecode;
    }

    // Load existing cache that can receive updates
    auto mallocSpan = MallocSpan<uint8_t, JSC::VMMalloc>::malloc(data.size());
    if (!mallocSpan) {
        m_cachedBytecode = JSC::CachedBytecode::create();
        return m_cachedBytecode;
    }

    memcpySpan(mallocSpan.mutableSpan(), std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.constData()), data.size()));
    m_cachedBytecode = JSC::CachedBytecode::create(WTFMove(mallocSpan), {});
    return m_cachedBytecode;
}

void CachedScriptSourceProvider::cacheBytecode(const JSC::BytecodeCacheGenerator& generator) const
{
    // Ensure we have a cache to accumulate updates
    if (!m_cachedBytecode)
        m_cachedBytecode = JSC::CachedBytecode::create();

    auto update = generator();
    if (update)
        m_cachedBytecode->addGlobalUpdate(*update);
}

void CachedScriptSourceProvider::updateCache(const JSC::UnlinkedFunctionExecutable* executable, const JSC::SourceCode&, JSC::CodeSpecializationKind kind, const JSC::UnlinkedFunctionCodeBlock* codeBlock) const
{
    if (!m_cachedBytecode)
        return;

    JSC::BytecodeCacheError error;
    RefPtr<JSC::CachedBytecode> functionBytecode = JSC::encodeFunctionCodeBlock(executable->vm(), codeBlock, error);
    if (functionBytecode && !error.isValid())
        m_cachedBytecode->addFunctionUpdate(executable, kind, *functionBytecode);
}

void CachedScriptSourceProvider::commitCachedBytecode() const
{
    auto* delegate = getOrCreateDefaultDelegate();
    if (!delegate || !m_cachedBytecode || !m_cachedBytecode->hasUpdates())
        return;

    QString key = generateCacheKey(sourceURL());
    QString hashString = QString::number(hash());

    // Create a buffer to hold the complete updated bytecode
    QByteArray updatedData;
    updatedData.resize(m_cachedBytecode->sizeForUpdate());

    // Start with existing payload if any
    auto existingSpan = m_cachedBytecode->span();
    if (!existingSpan.empty()) {
        auto destinationSpan = std::span<uint8_t>(reinterpret_cast<uint8_t*>(updatedData.data()), existingSpan.size());
        memcpySpan(destinationSpan, existingSpan);
    }

    // Apply all accumulated updates
    m_cachedBytecode->commitUpdates([&](off_t offset, std::span<const uint8_t> updateData) {
        // Ensure buffer is large enough
        auto requiredSize = static_cast<qsizetype>(offset) + static_cast<qsizetype>(updateData.size());
        if (updatedData.size() < requiredSize)
            updatedData.resize(requiredSize);

        // Copy update data at the specified offset
        auto destinationSpan = std::span<uint8_t>(reinterpret_cast<uint8_t*>(updatedData.data()) + offset, updateData.size());
        memcpySpan(destinationSpan, updateData);
    });

    // Store the complete updated bytecode through delegate
    delegate->storeBytecode(QString(sourceURL()), hashString, updatedData);
}

} // namespace WebCore

#endif // PLATFORM(QT)