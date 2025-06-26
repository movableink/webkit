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

#include "QtBytecodeCacheDelegate.h"
#include <wtf/Noncopyable.h>
#include <wtf/text/WTFString.h>
#include <atomic>
#include <QTimer>

class QString;
class QByteArray;

namespace WebCore {

class WEBCORE_EXPORT QtDiskCacheDelegate final : public QtBytecodeCacheDelegate {
    WTF_MAKE_NONCOPYABLE(QtDiskCacheDelegate);
public:
    QtDiskCacheDelegate(const QString& cachePath = QString(), size_t maxSize = 100 * 1024 * 1024);
    ~QtDiskCacheDelegate() = default;

    // QtBytecodeCacheDelegate interface
    QByteArray loadBytecode(const QString& sourceURL, const QString& sourceHash) override;
    void storeBytecode(const QString& sourceURL, const QString& sourceHash, const QByteArray& bytecode) override;
    void performMaintenance() override;

private:
    // Constants
    static constexpr size_t SHA1_HASH_SIZE = 20;
    static constexpr double MAINTENANCE_THRESHOLD = 0.8; // 80% of max size
    static constexpr double EVICTION_TARGET = 0.7; // 70% of max size
    static constexpr unsigned SUBDIRECTORY_COUNT = 256;

    void ensureInitialized();
    String determineCacheDirectory() const;
    String cacheFilePath(const QString& sourceURL) const;
    bool validateCacheFile(const String& filePath, const QString& sourceHash) const;
    void evictLRUIfNeeded();
    size_t calculateTotalCacheSize() const;

    // Deferred maintenance using Qt timers
    void scheduleMaintenanceIfNeeded();
    void performDeferredMaintenance();

    bool m_initialized { false };
    String m_cacheDirectory;
    QString m_customCachePath;
    size_t m_maxCacheSize;

    // Deferred maintenance state
    std::atomic<bool> m_maintenanceScheduled { false };
    std::atomic<size_t> m_approximateCacheSize { 0 };
    std::unique_ptr<QTimer> m_maintenanceTimer;
    
};

} // namespace WebCore