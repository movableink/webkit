/*
 * Copyright (C) 2025 Movable, Inc.
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

#pragma once

class QString;
class QByteArray;

namespace WebCore {

class WEBCORE_EXPORT QtBytecodeCacheDelegate {
public:
    virtual ~QtBytecodeCacheDelegate() = default;

    // Load cached bytecode for the given source URL and hash
    // Returns empty QByteArray if not found or on error
    virtual QByteArray loadBytecode(const QString& sourceURL, const QString& sourceHash) = 0;

    // Store bytecode for the given source URL and hash
    // Implementation should handle errors gracefully (no exceptions)
    virtual void storeBytecode(const QString& sourceURL, const QString& sourceHash, const QByteArray& bytecode) = 0;

    // Optional maintenance callback (e.g., cache cleanup)
    // Called periodically by QtWebKit when appropriate
    virtual void performMaintenance() { }
};

// Internal API for QWebSettings integration
WEBCORE_EXPORT void setGlobalBytecodeCacheDelegate(QtBytecodeCacheDelegate* delegate);
WEBCORE_EXPORT QtBytecodeCacheDelegate* globalBytecodeCacheDelegate();

} // namespace WebCore