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

#include "config.h"
#include "QtDiskCacheDelegate.h"

#include <QStandardPaths>
#include <QDir>
#include <QTimer>

#include <JavaScriptCore/Options.h>
#include <wtf/FileSystem.h>
#include <wtf/FileHandle.h>
#include <wtf/SHA1.h>
#include <wtf/text/MakeString.h>
#include <wtf/HexNumber.h>
#include "Logging.h"

namespace WebCore {


QtDiskCacheDelegate::QtDiskCacheDelegate(const QString& cachePath, size_t maxSize)
    : m_customCachePath(cachePath)
    , m_maxCacheSize(maxSize)
    , m_maintenanceTimer(std::make_unique<QTimer>())
{
    m_maintenanceTimer->setSingleShot(true);
    QObject::connect(m_maintenanceTimer.get(), &QTimer::timeout, [this]() {
        performDeferredMaintenance();
    });
    
    // Initialize eagerly if logging is enabled to show startup message
    if (LogBytecodeCache.state != WTFLogChannelState::Off)
        ensureInitialized();
}

void QtDiskCacheDelegate::ensureInitialized()
{
    if (m_initialized)
        return;

    m_initialized = true;

    m_cacheDirectory = determineCacheDirectory();
    if (m_cacheDirectory.isEmpty()) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: ERROR - Could not determine cache directory");
        return;
    }

    if (!FileSystem::makeAllDirectories(m_cacheDirectory)) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: ERROR - Could not create cache directory: %s", m_cacheDirectory.utf8().data());
        m_cacheDirectory = String();
        return;
    }

    LOG(BytecodeCache, "QtDiskCacheDelegate: ENABLED - Location: %s MaxSize: %zuMB", 
        m_cacheDirectory.utf8().data(), m_maxCacheSize / (1024 * 1024));
}

String QtDiskCacheDelegate::determineCacheDirectory() const
{
    if (!m_customCachePath.isEmpty())
        return String(m_customCachePath);

    QString qtCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (qtCacheDir.isEmpty()) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Could not determine Qt cache location");
        return String();
    }

    QDir dir(qtCacheDir);
    if (!dir.mkpath(QStringLiteral("qtwebkit/bytecode"))) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Could not create bytecode subdirectory");
        return String();
    }

    return String(dir.absoluteFilePath(QStringLiteral("qtwebkit/bytecode")));
}

String QtDiskCacheDelegate::cacheFilePath(const QString& sourceURL) const
{
    if (m_cacheDirectory.isEmpty())
        return String();

    SHA1 sha1;
    sha1.addUTF8Bytes(String(sourceURL));
    String hexKey = String::fromUTF8(sha1.computeHexDigest().data());

    String subDirName = hexKey.substring(0, 2);
    String subDirPath = FileSystem::pathByAppendingComponent(m_cacheDirectory, subDirName);

    if (!FileSystem::fileExists(subDirPath)) {
        if (!FileSystem::makeAllDirectories(subDirPath)) {
            LOG(BytecodeCache, "QtDiskCacheDelegate: Could not create subdirectory: %s", subDirPath.utf8().data());
            return String();
        }
    }

    String fileName = makeString(hexKey, ".cache"_s);
    return FileSystem::pathByAppendingComponent(subDirPath, fileName);
}

bool QtDiskCacheDelegate::validateCacheFile(const String& filePath, const QString& sourceHash) const
{
    auto handle = FileSystem::openFile(filePath, FileSystem::FileOpenMode::Read);
    if (!handle) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Could not open cache file for validation: %s", filePath.utf8().data());
        return false;
    }

    auto fileSizeOpt = handle.size();
    if (!fileSizeOpt || fileSizeOpt.value() < SHA1_HASH_SIZE) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Cache file too small: %s", filePath.utf8().data());
        return false;
    }

    uint64_t fileSize = fileSizeOpt.value();

    Vector<uint8_t> storedHash(SHA1_HASH_SIZE);
    auto seekResult = handle.seek(fileSize - SHA1_HASH_SIZE, FileSystem::FileSeekOrigin::Beginning);
    if (!seekResult) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Could not seek to hash position in cache file: %s", filePath.utf8().data());
        return false;
    }

    auto readResult = handle.read(std::span<uint8_t>(storedHash.data(), SHA1_HASH_SIZE));
    if (!readResult || readResult.value() != SHA1_HASH_SIZE) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Could not read hash from cache file: %s", filePath.utf8().data());
        return false;
    }

    SHA1 sha1;
    sha1.addUTF8Bytes(String(sourceHash));
    SHA1::Digest computedHash;
    sha1.computeHash(computedHash);

    bool valid = memcmp(storedHash.data(), computedHash.data(), SHA1_HASH_SIZE) == 0;
    if (!valid) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Hash mismatch for cache file: %s", filePath.utf8().data());
        FileSystem::deleteFile(filePath);
    }
    
    return valid;
}

QByteArray QtDiskCacheDelegate::loadBytecode(const QString& sourceURL, const QString& sourceHash)
{
    ensureInitialized();

    String filePath = cacheFilePath(sourceURL);
    if (filePath.isEmpty())
        return QByteArray();

    if (!validateCacheFile(filePath, sourceHash)) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: MISS - %s (file not found or invalid)", sourceURL.toUtf8().constData());
        return QByteArray();
    }

    auto handle = FileSystem::openFile(filePath, FileSystem::FileOpenMode::Read);
    if (!handle) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Could not open validated cache file: %s", filePath.utf8().data());
        return QByteArray();
    }

    auto fileSizeOpt = handle.size();
    if (!fileSizeOpt || fileSizeOpt.value() <= SHA1_HASH_SIZE) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Invalid file size for validated cache file: %s", filePath.utf8().data());
        return QByteArray();
    }

    uint64_t fileSize = fileSizeOpt.value();
    size_t bytecodeSize = fileSize - SHA1_HASH_SIZE;

    QByteArray bytecode(bytecodeSize, Qt::Uninitialized);
    auto readResult = handle.read(std::span<uint8_t>(reinterpret_cast<uint8_t*>(bytecode.data()), bytecodeSize));

    if (!readResult || readResult.value() != bytecodeSize) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Could not read bytecode from cache file: %s", filePath.utf8().data());
        return QByteArray();
    }

    FileSystem::updateFileModificationTime(filePath);

    LOG(BytecodeCache, "QtDiskCacheDelegate: HIT - %s (%zu bytes)", sourceURL.toUtf8().constData(), bytecodeSize);
    return bytecode;
}

void QtDiskCacheDelegate::storeBytecode(const QString& sourceURL, const QString& sourceHash, const QByteArray& bytecode)
{
    ensureInitialized();

    if (bytecode.isEmpty()) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Refusing to store empty bytecode for %s", sourceURL.toUtf8().constData());
        return;
    }

    String filePath = cacheFilePath(sourceURL);
    if (filePath.isEmpty())
        return;

    String tempPath = makeString(filePath, ".tmp"_s);
    
    {
        auto handle = FileSystem::openFile(tempPath, FileSystem::FileOpenMode::Truncate);
        if (!handle) {
            LOG(BytecodeCache, "QtDiskCacheDelegate: Could not create temp file: %s", tempPath.utf8().data());
            return;
        }

        auto writeResult = handle.write(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(bytecode.constData()), bytecode.size()));
        if (!writeResult || writeResult.value() != static_cast<uint64_t>(bytecode.size())) {
            LOG(BytecodeCache, "QtDiskCacheDelegate: Failed to write bytecode to temp file: %s", tempPath.utf8().data());
            FileSystem::deleteFile(tempPath);
            return;
        }

        SHA1 sha1;
        sha1.addUTF8Bytes(String(sourceHash));
        SHA1::Digest computedHash;
        sha1.computeHash(computedHash);

        // Hash appended to the end of the bytecode
        auto hashWriteResult = handle.write(std::span<const uint8_t>(computedHash.data(), SHA1_HASH_SIZE));
        if (!hashWriteResult || hashWriteResult.value() != SHA1_HASH_SIZE) {
            LOG(BytecodeCache, "QtDiskCacheDelegate: Failed to write hash to temp file: %s", tempPath.utf8().data());
            FileSystem::deleteFile(tempPath);
            return;
        }
    }

    if (!FileSystem::moveFile(tempPath, filePath)) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Failed to move temp file to cache location: %s -> %s", 
            tempPath.utf8().data(), filePath.utf8().data());
        FileSystem::deleteFile(tempPath);
        return;
    }

    size_t totalSize = bytecode.size() + SHA1_HASH_SIZE;
    m_approximateCacheSize.fetch_add(totalSize);
    scheduleMaintenanceIfNeeded();

    LOG(BytecodeCache, "QtDiskCacheDelegate: STORE - %s (%lld bytes)", 
        sourceURL.toUtf8().constData(), static_cast<long long>(bytecode.size()));
}

void QtDiskCacheDelegate::scheduleMaintenanceIfNeeded()
{
    size_t currentSize = m_approximateCacheSize.load();

    if (currentSize > m_maxCacheSize * MAINTENANCE_THRESHOLD && !m_maintenanceScheduled.exchange(true)) {
        LOG(BytecodeCache, "QtDiskCacheDelegate: Scheduled deferred maintenance (cache ~%zuMB)", 
            currentSize / (1024 * 1024));

        m_maintenanceTimer->start(0);
    }
}

void QtDiskCacheDelegate::performDeferredMaintenance()
{
    m_maintenanceScheduled.store(false);
    evictLRUIfNeeded();

    size_t actualSize = calculateTotalCacheSize();
    m_approximateCacheSize.store(actualSize);

    LOG(BytecodeCache, "QtDiskCacheDelegate: Maintenance complete (actual size: %zuMB)", 
        actualSize / (1024 * 1024));
}

void QtDiskCacheDelegate::performMaintenance()
{
    performDeferredMaintenance();
}

void QtDiskCacheDelegate::evictLRUIfNeeded()
{
    if (m_cacheDirectory.isEmpty())
        return;

    struct CacheFileInfo {
        String path;
        std::optional<WallTime> modTime;
        uint64_t size;
    };
    Vector<CacheFileInfo> cacheFiles;
    size_t currentSize = 0;

    for (unsigned i = 0; i < SUBDIRECTORY_COUNT; ++i) {
        String subDirName = makeString(hex(i, 2, Lowercase));
        String subDirPath = FileSystem::pathByAppendingComponent(m_cacheDirectory, subDirName);

        if (!FileSystem::fileExists(subDirPath))
            continue;

        for (auto& fileName : FileSystem::listDirectory(subDirPath)) {
            if (!fileName.endsWith(".cache"_s))
                continue;
                
            String fullPath = FileSystem::pathByAppendingComponent(subDirPath, fileName);
            auto fileSize = FileSystem::fileSize(fullPath);
            if (!fileSize) {
                LOG(BytecodeCache, "QtDiskCacheDelegate: Could not get size of cache file: %s", fullPath.utf8().data());
                continue;
            }
            
            auto modTime = FileSystem::fileModificationTime(fullPath);
            cacheFiles.append({ fullPath, modTime, fileSize.value() });
            currentSize += fileSize.value();
        }
    }

    if (currentSize <= m_maxCacheSize)
        return;

    LOG(BytecodeCache, "QtDiskCacheDelegate: Starting eviction (current size: %zuMB, max: %zuMB)", 
        currentSize / (1024 * 1024), m_maxCacheSize / (1024 * 1024));

    std::sort(cacheFiles.begin(), cacheFiles.end(), [](const auto& a, const auto& b) {
        if (!a.modTime || !b.modTime)
            return !a.modTime < !b.modTime; // Files without timestamps go first
        return a.modTime.value() < b.modTime.value();
    });

    // Remove oldest files until we're under the target
    size_t targetSize = m_maxCacheSize * EVICTION_TARGET;
    size_t deletedCount = 0;
    
    for (const auto& fileInfo : cacheFiles) {
        if (!FileSystem::deleteFile(fileInfo.path)) {
            LOG(BytecodeCache, "QtDiskCacheDelegate: Failed to delete cache file: %s", fileInfo.path.utf8().data());
            continue;
        }
        
        currentSize -= fileInfo.size;
        deletedCount++;
        
        if (currentSize <= targetSize)
            break;
    }

    LOG(BytecodeCache, "QtDiskCacheDelegate: Eviction complete (deleted %zu files, new size: ~%zuMB)", 
        deletedCount, currentSize / (1024 * 1024));
}

size_t QtDiskCacheDelegate::calculateTotalCacheSize() const
{
    if (m_cacheDirectory.isEmpty())
        return 0;

    size_t totalSize = 0;

    for (unsigned i = 0; i < SUBDIRECTORY_COUNT; ++i) {
        String subDirName = makeString(hex(i, 2, Lowercase));
        String subDirPath = FileSystem::pathByAppendingComponent(m_cacheDirectory, subDirName);

        if (!FileSystem::fileExists(subDirPath))
            continue;

        for (auto& fileName : FileSystem::listDirectory(subDirPath)) {
            if (!fileName.endsWith(".cache"_s))
                continue;
                
            String fullPath = FileSystem::pathByAppendingComponent(subDirPath, fileName);
            auto fileSize = FileSystem::fileSize(fullPath);
            if (fileSize)
                totalSize += fileSize.value();
        }
    }

    return totalSize;
}

} // namespace WebCore

