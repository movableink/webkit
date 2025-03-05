/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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
#include "JSExecState.h"

#include "EventLoop.h"
#include "JSDOMExceptionHandling.h"
#include "RejectedPromiseTracker.h"
#include "ScriptExecutionContext.h"
#include "WorkerGlobalScope.h"
#include <JavaScriptCore/VMTrapsInlines.h>

namespace WebCore {

void JSExecState::didLeaveScriptContext(JSC::JSGlobalObject* lexicalGlobalObject)
{
    auto context = executionContext(lexicalGlobalObject);
    if (!context)
        return;
    context->eventLoop().performMicrotaskCheckpoint();
}

JSC::JSValue functionCallHandlerFromAnyThread(JSC::JSGlobalObject* lexicalGlobalObject, JSC::JSValue functionObject, const JSC::CallData& callData, JSC::JSValue thisValue, const JSC::ArgList& args, NakedPtr<JSC::Exception>& returnedException)
{
    return JSExecState::call(lexicalGlobalObject, functionObject, callData, thisValue, args, returnedException);
}

JSC::JSValue evaluateHandlerFromAnyThread(JSC::JSGlobalObject* lexicalGlobalObject, const JSC::SourceCode& source, JSC::JSValue thisValue, NakedPtr<JSC::Exception>& returnedException)
{
    return JSExecState::evaluate(lexicalGlobalObject, source, thisValue, returnedException);
}

ScriptExecutionContext* executionContext(JSC::JSGlobalObject* globalObject)
{
    if (!globalObject || !globalObject->inherits<JSDOMGlobalObject>())
        return nullptr;
    return JSC::jsCast<JSDOMGlobalObject*>(globalObject)->scriptExecutionContext();
}

RefPtr<ScriptExecutionContext> protectedExecutionContext(JSC::JSGlobalObject* globalObject)
{
    return executionContext(globalObject);
}

void JSExecState::runTask(JSC::JSGlobalObject* globalObject, JSC::QueuedTask& task)
{
    JSExecState currentState(globalObject);

    auto& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    if (UNLIKELY(!task.job().isObject()))
        return;

    auto* job = JSC::asObject(task.job());

    if (UNLIKELY(!scope.clearExceptionExceptTermination()))
        return;

    auto* lexicalGlobalObject = job->globalObject();
    auto callData = JSC::getCallData(job);
    if (UNLIKELY(!scope.clearExceptionExceptTermination()))
        return;
    ASSERT(callData.type != CallData::Type::None);

    unsigned count = 0;
    for (auto argument : task.arguments()) {
        if (!argument)
            break;
        ++count;
    }

    if (UNLIKELY(globalObject->hasDebugger())) {
        JSC::DeferTerminationForAWhile deferTerminationForAWhile(vm);
        globalObject->debugger()->willRunMicrotask(globalObject, task.identifier());
        scope.clearException();
    }

    NakedPtr<JSC::Exception> returnedException = nullptr;
    if (LIKELY(!vm.hasPendingTerminationException())) {
        JSExecState::profiledCall(lexicalGlobalObject, JSC::ProfilingReason::Microtask, job, callData, JSC::jsUndefined(), JSC::ArgList { std::bit_cast<JSC::EncodedJSValue*>(task.arguments().data()), count }, returnedException);
        if (returnedException)
            reportException(lexicalGlobalObject, returnedException);
        scope.clearExceptionExceptTermination();
    }

    if (UNLIKELY(globalObject->hasDebugger())) {
        JSC::DeferTerminationForAWhile deferTerminationForAWhile(vm);
        globalObject->debugger()->didRunMicrotask(globalObject, task.identifier());
        scope.clearException();
    }
}


} // namespace WebCore
