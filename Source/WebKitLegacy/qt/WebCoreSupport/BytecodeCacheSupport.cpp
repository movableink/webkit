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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "BytecodeCacheSupport.h"
#include "qwebbytecodecachedelegate.h"

QWebBytecodeCacheDelegateAdapter::QWebBytecodeCacheDelegateAdapter(QWebBytecodeCacheDelegate* delegate)
    : m_delegate(delegate)
{
}

QByteArray QWebBytecodeCacheDelegateAdapter::loadBytecode(const QString& sourceURL, const QString& sourceHash)
{
    return m_delegate ? m_delegate->loadBytecode(sourceURL, sourceHash) : QByteArray();
}

void QWebBytecodeCacheDelegateAdapter::storeBytecode(const QString& sourceURL, const QString& sourceHash, const QByteArray& bytecode)
{
    if (m_delegate)
        m_delegate->storeBytecode(sourceURL, sourceHash, bytecode);
}

void QWebBytecodeCacheDelegateAdapter::performMaintenance()
{
    if (m_delegate)
        m_delegate->performMaintenance();
}