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

#ifndef QWEBBYTECODEDISKCACHE_H
#define QWEBBYTECODEDISKCACHE_H

#include "qwebbytecodecachedelegate.h"

class QWEBKIT_EXPORT QWebBytecodeDiskCache final : public QWebBytecodeCacheDelegate {
public:
    QWebBytecodeDiskCache(const QString& cachePath = QString(), size_t maxSize = 100 * 1024 * 1024);
    ~QWebBytecodeDiskCache();

    // QWebBytecodeCacheDelegate interface
    QByteArray loadBytecode(const QString& sourceURL, const QString& sourceHash) override;
    void storeBytecode(const QString& sourceURL, const QString& sourceHash, const QByteArray& bytecode) override;
    void performMaintenance() override;

private:
    class QWebBytecodeDiskCachePrivate;
    QWebBytecodeDiskCachePrivate* d;
};

#endif // QWEBBYTECODEDISKCACHE_H