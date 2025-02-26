/*
 * Copyright (C) 2024-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <wtf/Platform.h>

#if USE(PROTECTED_JIT)

#include <cstddef>
#include <cstdint>
#include <mach/mach_vm.h>
#include <mach/vm_map.h>
#include <mach/vm_param.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wtf/Assertions.h>
#include <wtf/Atomics.h>
#include <wtf/Compiler.h>
#include <wtf/DataLog.h>
#include <wtf/Lock.h>
#include <wtf/Threading.h>

#if OS(DARWIN)
#include <System/pthread_machdep.h>
#endif

namespace WTF {

class SequesteredImmortalHeap {
    static constexpr bool verbose { false };
    static constexpr pthread_key_t key = __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY0;
    static constexpr size_t sequesteredImmortalHeapSlotSize { 16 * KB };
public:
    static constexpr size_t slotSize { 128 };
    static constexpr size_t numSlots { 64 };

    enum class AllocationFailureMode {
        Assert,
        ReturnNull
    };

    static SequesteredImmortalHeap& instance()
    {
        // FIXME: this storage is not contained within the sequestered region
        static std::once_flag onceFlag;
        auto ptr = reinterpret_cast<SequesteredImmortalHeap*>(&s_instance);
        std::call_once(onceFlag, [] {
            storeStoreFence();
            new (&s_instance) SequesteredImmortalHeap();
        });
        return *ptr;
    }

    template <typename T> requires (sizeof(T) <= slotSize)
    T* allocateAndInstall()
    {
        T* slot = nullptr;
        {
            WTF::Locker locker { m_scavengerLock };
            ASSERT(!getUnchecked());
            // FIXME: implement resizing to a larger capacity
            RELEASE_ASSERT(m_nextFreeIndex < numSlots);

            WTF_ALLOW_UNSAFE_BUFFER_USAGE_BEGIN
            void* buff = &(m_slots[m_nextFreeIndex++]);
            slot = new (buff) T();
            WTF_ALLOW_UNSAFE_BUFFER_USAGE_END
        }
        _pthread_setspecific_direct(key, reinterpret_cast<void*>(slot));
        pthread_key_init_np(key, nullptr);

        dataLogIf(verbose, "SequesteredImmortalHeap: thread (", Thread::current(), ") allocated slot ", instance().m_nextFreeIndex - 1, " (", slot, ")");
        return slot;
    }

    void* getSlot()
    {
        return getUnchecked();
    }

    int computeSlotIndex(void* slotPtr)
    {
        auto slot = reinterpret_cast<uintptr_t>(slotPtr);
        auto arrayBase = reinterpret_cast<uintptr_t>(m_slots.begin());
        auto arrayBound = reinterpret_cast<uintptr_t>(m_slots.begin()) + sizeof(m_slots);
        ASSERT_UNUSED(arrayBound, slot >= arrayBase && slot < arrayBound);
        return static_cast<int>((slot - arrayBase) / slotSize);
    }

    static void scavenge()
    {
        // FIXME: provide hook for libpas scavenger
    }

    template<AllocationFailureMode mode>
    void* mapPages(size_t bytes)
    {
        void* memory = mmap(nullptr, bytes, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANON, -1, 0);
        if (UNLIKELY(memory == MAP_FAILED)) {
            if constexpr (mode == AllocationFailureMode::ReturnNull)
                return nullptr;
            RELEASE_ASSERT_NOT_REACHED();
        }
        return memory;
    }

private:
    SequesteredImmortalHeap()
    {
        RELEASE_ASSERT(!(reinterpret_cast<uintptr_t>(this) % sequesteredImmortalHeapSlotSize));
        static_assert(sizeof(*this) <= sequesteredImmortalHeapSlotSize);

        auto flags = VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE | VM_FLAGS_PERMANENT;
        auto prots = VM_PROT_READ | VM_PROT_WRITE;
        auto* self = reinterpret_cast<mach_vm_address_t*>(this);
        mach_vm_map(mach_task_self(), self, sizeof(*this), sequesteredImmortalHeapSlotSize - 1, flags, MEMORY_OBJECT_NULL, 0, false, prots, prots, VM_INHERIT_DEFAULT);

        // Cannot use dataLog here as it takes a lock
        if constexpr (verbose)
            fprintf(stderr, "SequesteredImmortalHeap: initialized by thread (%u)\n", Thread::current().uid());
    }

    static void* getUnchecked()
    {
        return _pthread_getspecific_direct(key);
    }

    struct alignas(WTF::Lock) LockSlot {
        std::array<std::byte, sizeof(WTF::Lock)> m_bytes;
        WTF::Lock& asLock()
        {
            return *reinterpret_cast<WTF::Lock*>(this);
        }
        void initialize()
        {
            new (this) WTF::Lock();
        }
    };

    struct alignas(slotSize) Slot {
        std::array<std::byte, slotSize> data;
    };

    struct alignas(sequesteredImmortalHeapSlotSize) Instance {
        std::byte data[sequesteredImmortalHeapSlotSize];
    };

    WTF::Lock m_scavengerLock { };
    size_t m_nextFreeIndex { };
    std::array<Slot, numSlots> m_slots { };

    static Instance s_instance;
};

}

#endif // USE(PROTECTED_JIT)
