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
#include "QtBytecodeDiskCache.h"

#if PLATFORM(QT)

#include <JavaScriptCore/CachedBytecode.h>
#include <JavaScriptCore/Options.h>
#include <wtf/FileSystem.h>
#include <wtf/FileHandle.h>
#include <wtf/SHA1.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/HexNumber.h>
#include "Logging.h"

#include <QStandardPaths>
#include <QDir>
#include <QTimer>

namespace WebCore {

QtBytecodeDiskCache& QtBytecodeDiskCache::shared()
{
    static QtBytecodeDiskCache instance;
    
    // Initialize on first access if logging is enabled to show startup message
    if (LogBytecodeCache.state != WTFLogChannelState::Off)
        instance.initialize();
    
    return instance;
}

bool QtBytecodeDiskCache::isEnabled() const
{
    return JSC::Options::useQtDiskCache();
}

void QtBytecodeDiskCache::initialize()
{
    if (m_initialized)
        return;
    
    m_initialized = true;
    
    if (!isEnabled()) {
        LOG(BytecodeCache, "QtBytecodeDiskCache: DISABLED");
        return;
    }
    
    String directory = cacheDirectory();
    if (directory.isEmpty()) {
        LOG(BytecodeCache, "QtBytecodeDiskCache: ERROR - Could not determine cache directory");
        return;
    }
    
    FileSystem::makeAllDirectories(directory);
    m_cacheDirectory = directory;
    
    size_t maxSize = JSC::Options::qtDiskCacheMaxSize();
    LOG(BytecodeCache, "QtBytecodeDiskCache: ENABLED - Location: %s MaxSize: %zuMB", directory.utf8().data(), maxSize / (1024 * 1024));
}

String QtBytecodeDiskCache::cacheDirectory() const
{
    // Use explicit path if provided
    if (JSC::Options::qtDiskCachePath())
        return String::fromLatin1(JSC::Options::qtDiskCachePath());
    
    // Use Qt's platform-specific cache location
    QString qtCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (qtCacheDir.isEmpty())
        return String();
    
    QDir cacheDir(qtCacheDir);
    if (!cacheDir.exists(QStringLiteral("qtwebkit")))
        cacheDir.mkpath(QStringLiteral("qtwebkit"));
    
    QString bytecodeCacheDir = cacheDir.absoluteFilePath(QStringLiteral("qtwebkit/bytecode"));
    return String(bytecodeCacheDir);
}

String QtBytecodeDiskCache::cacheFilePath(const String& sourceURL) const
{
    if (m_cacheDirectory.isEmpty())
        return String();
    
    // Generate SHA1 hash of URL for safe filename
    SHA1 sha1;
    sha1.addBytes(sourceURL.utf8().span());
    SHA1::Digest digest;
    sha1.computeHash(digest);
    
    CString hexHash = SHA1::hexDigest(digest);
    String hexHashString = String::fromUTF8(hexHash.data());
    String subDir = hexHashString.left(2);           // First 2 chars: 00-ff  
    StringBuilder fileNameBuilder;
    fileNameBuilder.append(hexHashString.substring(2));
    fileNameBuilder.append(".cache"_s);
    String fileName = fileNameBuilder.toString();  // Remaining 38 chars
    
    String dirPath = FileSystem::pathByAppendingComponent(m_cacheDirectory, subDir);
    
    // Lazy directory creation - only create when needed
    if (!FileSystem::fileExists(dirPath))
        FileSystem::makeAllDirectories(dirPath);
    
    return FileSystem::pathByAppendingComponent(dirPath, fileName);
}

RefPtr<JSC::CachedBytecode> QtBytecodeDiskCache::retrieve(const String& sourceURL, unsigned)
{
    if (!isEnabled()) {
        initialize();
        return nullptr;
    }
    
    if (m_cacheDirectory.isEmpty())
        return nullptr;
    
    String filePath = cacheFilePath(sourceURL);
    if (filePath.isEmpty())
        return nullptr;
    
    auto handle = FileSystem::openFile(filePath, FileSystem::FileOpenMode::Read);
    if (!handle) {
        LOG(BytecodeCache, "QtBytecodeDiskCache: MISS - %s (file not found)", sourceURL.utf8().data());
        return nullptr;
    }
    
    auto mappedFile = handle.map(FileSystem::MappedFileMode::Private);
    if (!mappedFile)
        return nullptr;
    
    auto fileData = mappedFile->span();
    
    // Verify minimum size (need SHA1 digest) - Apple-compatible format
    if (fileData.size() < sizeof(SHA1::Digest))
        return nullptr;
    
    unsigned fileDataSize = fileData.size() - sizeof(SHA1::Digest);
    
    // Verify integrity hash (Apple-compatible format)
    SHA1::Digest computedHash;
    SHA1 sha1;
    sha1.addBytes(fileData.first(fileDataSize));
    sha1.computeHash(computedHash);
    
    SHA1::Digest fileHash;
    auto hashSpan = fileData.subspan(fileDataSize, sizeof(SHA1::Digest));
    memcpySpan(std::span { fileHash }, hashSpan);
    
    if (computedHash != fileHash) {
        FileSystem::deleteFile(filePath);
        return nullptr;
    }
    
    // Return the mapped file - JSC will handle additional validation
    LOG(BytecodeCache, "QtBytecodeDiskCache: HIT - %s (%u bytes)", sourceURL.utf8().data(), fileDataSize);
    
    return JSC::CachedBytecode::create(WTFMove(*mappedFile));
}

void QtBytecodeDiskCache::store(const String& sourceURL, unsigned, const JSC::CachedBytecode& bytecode)
{
    if (!isEnabled()) {
        initialize();
        return;
    }
    
    if (m_cacheDirectory.isEmpty())
        return;
    
    String filePath = cacheFilePath(sourceURL);
    if (filePath.isEmpty())
        return;
    
    // Schedule background maintenance instead of blocking store operation
    scheduleMaintenanceIfNeeded();
    
    // Write to temp file first for atomic operation
    StringBuilder tempPathBuilder;
    tempPathBuilder.append(filePath);
    tempPathBuilder.append(".tmp"_s);
    String tempPath = tempPathBuilder.toString();
    
    auto handle = FileSystem::openFile(tempPath, FileSystem::FileOpenMode::ReadWrite);
    if (!handle) {
        FileSystem::deleteFile(tempPath);
        return;
    }
    
    // Write bytecode data first (Apple-compatible format)
    auto bytecodeSpan = bytecode.span();
    if (handle.write(bytecodeSpan) != static_cast<int>(bytecodeSpan.size())) {
        FileSystem::deleteFile(tempPath);
        return;
    }
    
    // Write integrity hash of bytecode data (Apple-compatible)
    SHA1::Digest computedHash;
    SHA1 sha1;
    sha1.addBytes(bytecodeSpan);
    sha1.computeHash(computedHash);
    
    if (handle.write(std::span { computedHash }) != sizeof(SHA1::Digest)) {
        FileSystem::deleteFile(tempPath);
        return;
    }
    
    // FileHandle automatically closes on destruction
    
    // Atomic rename
    FileSystem::moveFile(tempPath, filePath);
    
    LOG(BytecodeCache, "QtBytecodeDiskCache: STORE - %s (%zu bytes)", sourceURL.utf8().data(), bytecode.span().size());
    
    // Update approximate cache size for maintenance scheduling
    m_approximateCacheSize.fetch_add(bytecode.span().size());
}

bool QtBytecodeDiskCache::validateCacheFile(const String& filePath, unsigned) const
{
    // Simple existence check - JSC will do full validation
    return FileSystem::fileExists(filePath);
}

void QtBytecodeDiskCache::evictLRUIfNeeded()
{
    if (m_cacheDirectory.isEmpty())
        return;
        
    size_t currentSize = totalCacheSize();
    size_t maxSize = JSC::Options::qtDiskCacheMaxSize();
    
    if (currentSize <= maxSize)
        return;
    
    // Collect all cache files with their modification times
    Vector<std::pair<String, std::optional<WallTime>>> cacheFiles;
    
    // Scan all subdirectories (00-ff)
    for (unsigned i = 0; i < 256; ++i) {
        StringBuilder subDirBuilder;
        subDirBuilder.append(hex(i, 2, Lowercase));
        String subDirName = subDirBuilder.toString();
        String subDir = FileSystem::pathByAppendingComponent(m_cacheDirectory, subDirName);
        
        auto entries = FileSystem::listDirectory(subDir);
        
        for (const auto& entry : entries) {
            if (!entry.endsWith(".cache"_s))
                continue;
                
            String fullPath = FileSystem::pathByAppendingComponent(subDir, entry);
            auto modTime = FileSystem::fileModificationTime(fullPath);
            if (modTime)
                cacheFiles.append({ fullPath, modTime });
        }
    }
    
    // Sort by modification time (oldest first)
    std::sort(cacheFiles.begin(), cacheFiles.end(), [](const auto& a, const auto& b) {
        if (!a.second || !b.second)
            return false;
        return *a.second < *b.second;
    });
    
    // Remove oldest files until under size limit
    size_t removedSize = 0;
    for (const auto& file : cacheFiles) {
        if (currentSize - removedSize <= maxSize)
            break;
            
        auto fileSize = FileSystem::fileSize(file.first);
        if (fileSize) {
            removedSize += *fileSize;
            FileSystem::deleteFile(file.first);
        }
    }
}

size_t QtBytecodeDiskCache::totalCacheSize() const
{
    if (m_cacheDirectory.isEmpty())
        return 0;
        
    size_t totalSize = 0;
    
    // Scan all subdirectories (00-ff)
    for (unsigned i = 0; i < 256; ++i) {
        StringBuilder subDirBuilder;
        subDirBuilder.append(hex(i, 2, Lowercase));
        String subDirName = subDirBuilder.toString();
        String subDir = FileSystem::pathByAppendingComponent(m_cacheDirectory, subDirName);
        
        auto entries = FileSystem::listDirectory(subDir);
        
        for (const auto& entry : entries) {
            if (!entry.endsWith(".cache"_s))
                continue;
                
            String fullPath = FileSystem::pathByAppendingComponent(subDir, entry);
            auto fileSize = FileSystem::fileSize(fullPath);
            if (fileSize)
                totalSize += *fileSize;
        }
    }
    
    return totalSize;
}

void QtBytecodeDiskCache::scheduleMaintenanceIfNeeded()
{
    size_t approximateSize = m_approximateCacheSize.load();
    size_t maxSize = JSC::Options::qtDiskCacheMaxSize();
    
    // Schedule maintenance if cache appears to be over 80% full
    if (approximateSize > maxSize * 8 / 10) {
        bool expected = false;
        if (m_maintenanceScheduled.compare_exchange_strong(expected, true)) {
            // Use Qt's deferred execution - safe and follows Qt WebKit patterns
            QTimer::singleShot(0, [this]() {
                performDeferredMaintenance();
            });
            LOG(BytecodeCache, "QtBytecodeDiskCache: Scheduled deferred maintenance (cache ~%zuMB)", approximateSize / (1024 * 1024));
        }
    }
}

void QtBytecodeDiskCache::performDeferredMaintenance()
{
    LOG(BytecodeCache, "QtBytecodeDiskCache: Starting deferred maintenance");
    
    // Reset the scheduled flag
    m_maintenanceScheduled.store(false);
    
    // Perform actual eviction (reusing existing logic)
    evictLRUIfNeeded();
    
    // Update approximate size with actual size
    size_t actualSize = totalCacheSize();
    m_approximateCacheSize.store(actualSize);
    
    LOG(BytecodeCache, "QtBytecodeDiskCache: Maintenance complete (actual size: %zuMB)", actualSize / (1024 * 1024));
}

} // namespace WebCore

#endif // PLATFORM(QT)