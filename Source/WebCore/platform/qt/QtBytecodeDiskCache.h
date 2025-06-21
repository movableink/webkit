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

#pragma once

#if PLATFORM(QT)

#include <JavaScriptCore/CachedBytecode.h>
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>
#include <atomic>

namespace WebCore {

class QtBytecodeDiskCache {
    WTF_MAKE_NONCOPYABLE(QtBytecodeDiskCache);
public:
    static QtBytecodeDiskCache& shared();
    
    RefPtr<JSC::CachedBytecode> retrieve(const String& sourceURL, unsigned sourceHash);
    void store(const String& sourceURL, unsigned sourceHash, const JSC::CachedBytecode&);
    
    bool isEnabled() const;
    void initialize(); // Public so it can be called early for logging
    
private:
    QtBytecodeDiskCache() = default;
    
    String cacheDirectory() const;
    String cacheFilePath(const String& sourceURL) const;
    bool validateCacheFile(const String& filePath, unsigned expectedSourceHash) const;
    void evictLRUIfNeeded();
    size_t totalCacheSize() const;
    
    // Deferred maintenance using Qt timers
    void scheduleMaintenanceIfNeeded();
    void performDeferredMaintenance();
    
    mutable bool m_initialized { false };
    mutable String m_cacheDirectory;
    
    // Deferred maintenance state  
    std::atomic<bool> m_maintenanceScheduled { false };
    std::atomic<size_t> m_approximateCacheSize { 0 };
};

} // namespace WebCore

#endif // PLATFORM(QT)