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

#include "config.h"
#include <wtf/SequesteredImmortalHeap.h>
#include <wtf/Compiler.h>

#if USE(PROTECTED_JIT)

namespace WTF {

void ConcurrentDecommitQueue::decommit()
{
    auto lst = acquireExclusiveCopyOfGranuleList();

    auto* curr = lst.head();
    if (!curr)
        return;

    // FIXME: this should go to a page-provider rather than the SIH
    auto& sih = SequesteredImmortalHeap::instance();

    size_t decommitPageCount { 0 };
    size_t decommitGranuleCount { 0 };
    UNUSED_VARIABLE(decommitPageCount);
    UNUSED_VARIABLE(decommitGranuleCount);

    do {
        auto* next = curr->next();
        auto pages = sih.decommitGranule(curr);

        dataLogLnIf(verbose,
            "ConcurrentDecommitQueue: decommitted granule at ",
            RawPointer(curr), " (", pages, " pages)");

        decommitPageCount += pages;
        decommitGranuleCount++;

        curr = next;
    } while (curr);

    dataLogLnIf(verbose, "ConcurrentDecommitQueue: decommitted ",
        decommitGranuleCount, " granules (", decommitPageCount, " pages)");
}

SequesteredImmortalHeap::Instance SequesteredImmortalHeap::s_instance;

}

#endif // USE(PROTECTED_JIT)
