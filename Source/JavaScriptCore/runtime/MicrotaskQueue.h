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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "JSCJSValue.h"
#include "Microtask.h"
#include "SlotVisitorMacros.h"
#include <wtf/Deque.h>
#include <wtf/SentinelLinkedList.h>

namespace JSC {

class MicrotaskQueue;
class VM;

class QueuedTask {
    WTF_MAKE_TZONE_ALLOCATED(QueuedTask);
    friend class MicrotaskQueue;
public:
    static constexpr unsigned maxArguments = 4;

    QueuedTask(MicrotaskIdentifier identifier, JSValue job, JSValue argument0, JSValue argument1, JSValue argument2, JSValue argument3)
        : m_identifier(identifier)
        , m_job(job)
        , m_arguments { argument0, argument1, argument2, argument3 }
    {
    }

    void run();

    MicrotaskIdentifier identifier() const { return m_identifier; }

private:
    MicrotaskIdentifier m_identifier;
    JSValue m_job;
    JSValue m_arguments[maxArguments];
};

class MicrotaskQueue final : public BasicRawSentinelNode<MicrotaskQueue> {
    WTF_MAKE_TZONE_ALLOCATED(MicrotaskQueue);
    WTF_MAKE_NONCOPYABLE(MicrotaskQueue);
public:
    MicrotaskQueue(VM&);

    QueuedTask dequeue()
    {
        if (m_markedBefore)
            --m_markedBefore;
        return m_queue.takeFirst();
    }

    void enqueue(QueuedTask&& task)
    {
        m_queue.append(WTFMove(task));
    }

    bool isEmpty() const
    {
        return m_queue.isEmpty();
    }

    size_t size() const { return m_queue.size(); }

    void clear()
    {
        m_queue.clear();
        m_markedBefore = 0;
    }

    void beginMarking()
    {
        m_markedBefore = 0;
    }

    DECLARE_VISIT_AGGREGATE;

private:
    Deque<QueuedTask, 8> m_queue;
    size_t m_markedBefore { 0 };
};

} // namespace JSC
