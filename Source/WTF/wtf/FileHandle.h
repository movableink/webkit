/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <optional>
#include <wtf/Forward.h>
#include <wtf/Markable.h>
#include <wtf/OptionSet.h>

#if OS(WINDOWS)
#include <wtf/win/Win32Handle.h>
#endif

namespace WTF {

namespace FileSystemImpl {

#if OS(WINDOWS)
typedef HANDLE PlatformFileHandle;
const PlatformFileHandle invalidPlatformFileHandle = reinterpret_cast<HANDLE>(-1);
typedef FILE_ID_128 PlatformFileID;

struct Win32HandleMarkableTraits {
    constexpr static bool isEmptyValue(HANDLE value) { return value == invalidPlatformFileHandle; }
    constexpr static HANDLE emptyValue() { return invalidPlatformFileHandle; }
};
using PlatformHandleTraits = Win32HandleMarkableTraits;
#else
typedef int PlatformFileHandle;
const PlatformFileHandle invalidPlatformFileHandle = -1;
typedef ino_t PlatformFileID;
using PlatformHandleTraits = IntegralMarkableTraits<int, invalidPlatformFileHandle>;
#endif

enum class FileSeekOrigin {
    Beginning,
    Current,
    End,
};

enum class FileLockMode {
    Shared = 1 << 0,
    Exclusive = 1 << 1,
    Nonblocking = 1 << 2,
};

class FileHandle {
public:
    WTF_EXPORT_PRIVATE FileHandle();

    static FileHandle adopt(PlatformFileHandle handle, OptionSet<FileLockMode> lockMode = { })
    {
        return FileHandle { handle, lockMode };
    }

    WTF_EXPORT_PRIVATE FileHandle(FileHandle&&);
    WTF_EXPORT_PRIVATE FileHandle& operator=(FileHandle&&);

    WTF_EXPORT_PRIVATE ~FileHandle();

    PlatformFileHandle platformHandle() const { return m_handle.unsafeValue(); }
    bool isValid() const { return !!m_handle; }
    explicit operator bool() const { return isValid(); }

    WTF_EXPORT_PRIVATE std::optional<uint64_t> write(std::span<const uint8_t>);
    WTF_EXPORT_PRIVATE std::optional<uint64_t> read(std::span<uint8_t>);
    WTF_EXPORT_PRIVATE std::optional<Vector<uint8_t>> readAll();
    WTF_EXPORT_PRIVATE bool truncate(int64_t offset);
    WTF_EXPORT_PRIVATE std::optional<uint64_t> size();

    // Returns the resulting offset from the beginning of the file if successful, -1 otherwise.
    WTF_EXPORT_PRIVATE int64_t seek(int64_t offset, FileSeekOrigin);

    WTF_EXPORT_PRIVATE bool flush();
    WTF_EXPORT_PRIVATE std::optional<PlatformFileID> id();

private:
    WTF_EXPORT_PRIVATE FileHandle(PlatformFileHandle, OptionSet<FileLockMode>);

#if USE(FILE_LOCK)
    bool lock(OptionSet<FileLockMode>);
#endif
    void close();

    Markable<PlatformFileHandle, PlatformHandleTraits> m_handle;
};

} // namespace FileSystemImpl

} // namespace WTF

namespace FileSystem = WTF::FileSystemImpl;
