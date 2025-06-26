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
#include <wtf/text/StringBuilder.h>
#include <wtf/HexNumber.h>
#include "QtBytecodeCacheDelegate.h"
#include "QtDiskCacheDelegate.h"

#include <QString>
#include <QByteArray>

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
    auto* delegate = getOrCreateDefaultDelegate();
    if (!delegate)
        return nullptr;

    QString key = generateCacheKey(sourceURL());
    QString hashString = QString::number(hash());

    QByteArray data = delegate->loadBytecode(QString(sourceURL()), hashString);
    if (data.isEmpty())
        return nullptr;

    // Convert QByteArray to CachedBytecode
    auto mallocSpan = MallocSpan<uint8_t, JSC::VMMalloc>::malloc(data.size());
    if (!mallocSpan)
        return nullptr;

    memcpy(mallocSpan.mutableSpan().data(), data.constData(), data.size());
    return JSC::CachedBytecode::create(WTFMove(mallocSpan), { });
}

void CachedScriptSourceProvider::cacheBytecode(const JSC::BytecodeCacheGenerator& generator) const
{
    auto* delegate = getOrCreateDefaultDelegate();
    if (!delegate)
        return;

    auto bytecode = generator();
    if (!bytecode)
        return;

    QString key = generateCacheKey(sourceURL());
    QString hashString = QString::number(hash());

    // Convert CachedBytecode to QByteArray
    auto span = bytecode->span();
    QByteArray data(reinterpret_cast<const char*>(span.data()), span.size());

    delegate->storeBytecode(QString(sourceURL()), hashString, data);
}

} // namespace WebCore

#endif // PLATFORM(QT)