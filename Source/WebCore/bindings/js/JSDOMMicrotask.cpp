/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSDOMMicrotask.h"

#include "JSDOMExceptionHandling.h"
#include "JSExecState.h"
#include "ScriptExecutionContext.h"
#include <JavaScriptCore/Debugger.h>
#include <JavaScriptCore/JSGlobalObject.h>
#include <JavaScriptCore/JSObjectInlines.h>
#include <JavaScriptCore/StrongInlines.h>
#include <JavaScriptCore/VMTrapsInlines.h>

namespace WebCore {
using namespace JSC;

class JSDOMMicrotask final : public Microtask {
public:
    JSDOMMicrotask(VM& vm, JSDOMGlobalObject* globalObject, JSObject* job)
        : m_globalObject { vm, globalObject }
        , m_job { vm, job }
    {
    }

    JSGlobalObject* globalObject() const final
    {
        return m_globalObject.get();
    }

    void run() final;

private:
    Strong<JSDOMGlobalObject> m_globalObject;
    Strong<JSObject> m_job;
};

Ref<Microtask> createJSDOMMicrotask(VM& vm, JSDOMGlobalObject* globalObject, JSObject* job)
{
    return adoptRef(*new JSDOMMicrotask(vm, globalObject, job));
}

void JSDOMMicrotask::run()
{
    VM& vm = m_globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    JSObject* job = m_job.get();


    auto* lexicalGlobalObject = job->globalObject();
    auto* context = jsCast<JSDOMGlobalObject*>(lexicalGlobalObject)->scriptExecutionContext();
    if (!context || context->activeDOMObjectsAreSuspended() || context->activeDOMObjectsAreStopped())
        return;

    if (UNLIKELY(!scope.clearExceptionExceptTermination()))
        return;

    auto callData = JSC::getCallData(job);
    if (UNLIKELY(!scope.clearExceptionExceptTermination()))
        return;
    ASSERT(callData.type != CallData::Type::None);

    if (UNLIKELY(m_globalObject->hasDebugger())) {
        JSC::DeferTerminationForAWhile deferTerminationForAWhile(vm);
        m_globalObject->debugger()->willRunMicrotask(m_globalObject.get(), identifier());
        scope.clearException();
    }

    NakedPtr<JSC::Exception> returnedException = nullptr;
    if (LIKELY(!vm.hasPendingTerminationException())) {
        JSExecState::profiledCall(lexicalGlobalObject, JSC::ProfilingReason::Microtask, job, callData, jsUndefined(), ArgList(), returnedException);
        if (returnedException)
            reportException(lexicalGlobalObject, returnedException);
        scope.clearExceptionExceptTermination();
    }

    if (UNLIKELY(m_globalObject->hasDebugger())) {
        JSC::DeferTerminationForAWhile deferTerminationForAWhile(vm);
        m_globalObject->debugger()->didRunMicrotask(m_globalObject.get(), identifier());
        scope.clearException();
    }
}

} // namespace WebCore
