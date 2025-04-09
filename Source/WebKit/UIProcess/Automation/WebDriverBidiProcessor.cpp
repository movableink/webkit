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

#include "config.h"
#include "WebDriverBidiProcessor.h"

#if ENABLE(WEBDRIVER_BIDI)

// For AutomationError codes.
#include "AutomationProtocolObjects.h"
#include "Logging.h"
#include "PageLoadState.h"
#include "WebAutomationSession.h"
#include "WebAutomationSessionMacros.h"
#include "WebDriverBidiBackendDispatchers.h"
#include "WebDriverBidiFrontendDispatchers.h"
#include "WebDriverBidiProtocolObjects.h"
#include "WebPageProxy.h"
#include "WebProcessPool.h"
#include <JavaScriptCore/InspectorBackendDispatcher.h>
#include <JavaScriptCore/InspectorFrontendRouter.h>
#include <wtf/TZoneMallocInlines.h>
#include <wtf/text/MakeString.h>

namespace WebKit {

using namespace Inspector;
using BrowsingContext = Inspector::Protocol::BidiBrowsingContext::BrowsingContext;
using ReadinessState = Inspector::Protocol::BidiBrowsingContext::ReadinessState;
using PageLoadStrategy = Inspector::Protocol::Automation::PageLoadStrategy;

WTF_MAKE_TZONE_ALLOCATED_IMPL(WebDriverBidiProcessor);

WebDriverBidiProcessor::WebDriverBidiProcessor(WebAutomationSession& session)
    : m_session(session)
    , m_frontendRouter(FrontendRouter::create())
    , m_backendDispatcher(BackendDispatcher::create(m_frontendRouter.copyRef()))
    , m_browserDomainDispatcher(BidiBrowserBackendDispatcher::create(m_backendDispatcher, this))
    , m_browsingContextDomainDispatcher(BidiBrowsingContextBackendDispatcher::create(m_backendDispatcher, this))
    , m_scriptDomainDispatcher(BidiScriptBackendDispatcher::create(m_backendDispatcher, this))
    , m_logDomainNotifier(makeUnique<BidiLogFrontendDispatcher>(m_frontendRouter))
{
    protectedFrontendRouter()->connectFrontend(*this);
}

WebDriverBidiProcessor::~WebDriverBidiProcessor()
{
    protectedFrontendRouter()->disconnectFrontend(*this);
}

Ref<Inspector::FrontendRouter> WebDriverBidiProcessor::protectedFrontendRouter() const
{
    return m_frontendRouter;
}

Ref<Inspector::BackendDispatcher> WebDriverBidiProcessor::protectedBackendDispatcher() const
{
    return m_backendDispatcher;
}

void WebDriverBidiProcessor::processBidiMessage(const String& message)
{
    RefPtr session = m_session.get();
    if (!session) {
        LOG(Automation, "processBidiMessage of length %d not delivered, session is gone!", message.length());
        return;
    }

    LOG(Automation, "[s:%s] processBidiMessage of length %d", session->sessionIdentifier().utf8().data(), message.length());
    LOG(Automation, "%s", message.utf8().data());

    protectedBackendDispatcher()->dispatch(message);
}

void WebDriverBidiProcessor::sendBidiMessage(const String& message)
{
    RefPtr session = m_session.get();
    if (!session) {
        LOG(Automation, "sendBidiMessage of length %d not delivered, session is gone!", message.length());
        return;
    }

    LOG(Automation, "[s:%s] sendBidiMessage of length %d", session->sessionIdentifier().utf8().data(), message.length());
    LOG(Automation, "%s", message.utf8().data());

    session->sendBidiMessage(message);
}


// MARK: Inspector::FrontendChannel methods.

void WebDriverBidiProcessor::sendMessageToFrontend(const String& message)
{
    sendBidiMessage(message);
}


// MARK: Inspector::BidiBrowsingContextDispatcherHandler methods.

void WebDriverBidiProcessor::activate(const BrowsingContext& browsingContext, CommandCallback<void>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    RefPtr webPageProxy = session->webPageProxyForHandle(browsingContext);
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!webPageProxy, WindowNotFound);

    // FIXME: detect non-top level browsing contexts, returning `invalid argument`.
    session->switchToBrowsingContext(browsingContext, emptyString(), [callback = WTFMove(callback)](CommandResult<void>&& result) {
        if (!result) {
            callback(makeUnexpected(result.error()));
            return;
        }

        callback({ });
    });
}

void WebDriverBidiProcessor::close(const BrowsingContext& browsingContext, std::optional<bool>&& optionalPromptUnload, CommandCallback<void>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    // FIXME: implement `promptUnload` option.
    // FIXME: raise `invalid argument` if `browsingContext` is not a top-level traversable.

    session->closeBrowsingContext(browsingContext);

    callback({ });
}

static constexpr Inspector::Protocol::Automation::BrowsingContextPresentation defaultBrowsingContextPresentation = Inspector::Protocol::Automation::BrowsingContextPresentation::Tab;

static Inspector::Protocol::Automation::BrowsingContextPresentation browsingContextPresentationFromCreateType(Inspector::Protocol::BidiBrowsingContext::CreateType createType)
{
    switch (createType) {
    case Inspector::Protocol::BidiBrowsingContext::CreateType::Tab:
        return Inspector::Protocol::Automation::BrowsingContextPresentation::Tab;
    case Inspector::Protocol::BidiBrowsingContext::CreateType::Window:
        return Inspector::Protocol::Automation::BrowsingContextPresentation::Window;
    }

    ASSERT_NOT_REACHED();
    return defaultBrowsingContextPresentation;
}

void WebDriverBidiProcessor::create(Inspector::Protocol::BidiBrowsingContext::CreateType createType, const BrowsingContext& optionalReferenceContext, std::optional<bool>&& optionalBackground, const String& optionalUserContext, CommandCallback<BrowsingContext>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    // FIXME: implement `referenceContext` option.
    // FIXME: implement `background` option.
    // FIXME: implement `userContext` option.

    session->createBrowsingContext(browsingContextPresentationFromCreateType(createType), [callback = WTFMove(callback)](CommandResultOf<BrowsingContext, Inspector::Protocol::Automation::BrowsingContextPresentation>&& result) {
        if (!result) {
            callback(makeUnexpected(result.error()));
            return;
        }

        auto [resultContext, resultPresentation] = WTFMove(result.value());
        callback(WTFMove(resultContext));
    });
}

void WebDriverBidiProcessor::getTree(const BrowsingContext& optionalRoot, std::optional<double>&& optionalMaxDepth, CommandCallback<Ref<JSON::ArrayOf<Protocol::BidiBrowsingContext::Info>>>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    // FIXME: implement `root` option.
    // FIXME: implement `maxDepth` option.

    auto infos = JSON::ArrayOf<Inspector::Protocol::BidiBrowsingContext::Info>::create();
    for (Ref process : session->protectedProcessPool()->processes()) {
        for (Ref page : process->pages()) {
            if (!page->isControlledByAutomation())
                continue;

            // FIXME: implement `parent` field.
            // FIXME: implement `children` field.
            // FIXME: implement `originalOpener` field.
            // FIXME: implement `clientWindow` field.
            // FIXME: implement `userContext` field.
            infos->addItem(Inspector::Protocol::BidiBrowsingContext::Info::create()
                .setContext(session->handleForWebPageProxy(page))
                .setUrl(page->currentURL())
                .setClientWindow("placeholder_window"_s)
                .setUserContext("placeholder_context"_s)
                .release());
        }
    }

    callback({ { WTFMove(infos) } });
}

// https://www.w3.org/TR/webdriver/#dfn-session-page-load-timeout
static constexpr Seconds defaultPageLoadTimeout = 300_s;
static constexpr ReadinessState defaultReadinessState = ReadinessState::None;

static PageLoadStrategy pageLoadStrategyFromReadinessState(ReadinessState state)
{
    switch (state) {
    case ReadinessState::None:
        return PageLoadStrategy::None;
    case ReadinessState::Interactive:
        return PageLoadStrategy::Eager;
    case ReadinessState::Complete:
        return PageLoadStrategy::Normal;
    }

    ASSERT_NOT_REACHED();
    return PageLoadStrategy::Normal;
}

void WebDriverBidiProcessor::navigate(const BrowsingContext& browsingContext, const String& url, std::optional<ReadinessState>&& optionalReadinessState, CommandCallbackOf<String, Inspector::Protocol::BidiBrowsingContext::Navigation>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    auto pageLoadStrategy = pageLoadStrategyFromReadinessState(optionalReadinessState.value_or(defaultReadinessState));
    session->navigateBrowsingContext(browsingContext, url, pageLoadStrategy, defaultPageLoadTimeout.milliseconds(), [url, callback = WTFMove(callback)](CommandResult<void>&& result) {
        if (!result) {
            callback(makeUnexpected(result.error()));
            return;
        }

        // FIXME: keep track of navigation IDs that we hand out.
        callback({ { url, "placeholder_navigation"_s } });
    });
}

void WebDriverBidiProcessor::reload(const BrowsingContext& browsingContext, std::optional<bool>&& optionalIgnoreCache, std::optional<ReadinessState>&& optionalReadinessState, CommandCallbackOf<String, Inspector::Protocol::BidiBrowsingContext::Navigation>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    // FIXME: implement `ignoreCache` option.

    auto pageLoadStrategy = pageLoadStrategyFromReadinessState(optionalReadinessState.value_or(defaultReadinessState));
    session->reloadBrowsingContext(browsingContext, pageLoadStrategy, defaultPageLoadTimeout.milliseconds(), [session = WTFMove(session), browsingContext, callback = WTFMove(callback)](CommandResult<void>&& result) {
        if (!result) {
            callback(makeUnexpected(result.error()));
            return;
        }

        ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

        RefPtr webPageProxy = session->webPageProxyForHandle(browsingContext);
        ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!webPageProxy, WindowNotFound);

        // FIXME: keep track of navigation IDs that we hand out.
        callback({ { webPageProxy->currentURL(), "placeholder_navigation"_s } });
    });
}

// MARK: Inspector::BidiBrowserDispatcherHandler methods.

Inspector::Protocol::ErrorStringOr<void> WebDriverBidiProcessor::close()
{
    RefPtr session = m_session.get();
    SYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    session->terminate();

    return { };
}

// MARK: Log domain.

void WebDriverBidiProcessor::logEntryAdded(const String& level, const String& source, const String& message, double timestamp, const String& type, const String& method)
{
    m_logDomainNotifier->entryAdded(level, source, message, timestamp, type, method);
}

// MARK: Inspector::BidiScriptDispatcherHandler methods.

void WebDriverBidiProcessor::callFunction(const String& functionDeclaration, bool awaitPromise, Ref<JSON::Object>&& target, RefPtr<JSON::Array>&& arguments, std::optional<Protocol::BidiScript::ResultOwnership>&&, RefPtr<JSON::Object>&& optionalSerializationOptions, RefPtr<JSON::Object>&& optionalThis, std::optional<bool>&& optionalUserActivation, CommandCallbackOf<Protocol::BidiScript::EvaluateResultType, String, RefPtr<Protocol::BidiScript::RemoteValue>, RefPtr<Protocol::BidiScript::ExceptionDetails>>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    // FIXME: handle non-BrowsingContext obtained from `Target`.
    std::optional<BrowsingContext> browsingContext = target->getString("context"_s);
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!browsingContext, InvalidParameter);
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session->webPageProxyForHandle(*browsingContext), WindowNotFound);

    // FIXME: handle `awaitPromise` option.
    // FIXME: handle `resultOwnership` option.
    // FIXME: handle `serializationOptions` option.
    // FIXME: handle custom `this` option.
    // FIXME: handle `userActivation` option.

    Ref<JSON::Array> argumentsArray = arguments ? arguments.releaseNonNull() : JSON::Array::create();

    session->evaluateJavaScriptFunction(*browsingContext, emptyString(), functionDeclaration, WTFMove(argumentsArray), false, optionalUserActivation.value_or(false), std::nullopt, [callback = WTFMove(callback)](Inspector::CommandResult<String>&& result) {
        auto evaluateResultType = result.has_value() ? Inspector::Protocol::BidiScript::EvaluateResultType::Success : Inspector::Protocol::BidiScript::EvaluateResultType::Exception;
        auto resultObject = Inspector::Protocol::BidiScript::RemoteValue::create()
            .setType(Inspector::Protocol::BidiScript::RemoteValueType::Object)
            .release();

        // FIXME: handle serializing different RemoteValue types as JSON.
        if (result)
            resultObject->setValue(JSON::Value::create(WTFMove(result.value())));

        // FIXME: keep track of realm IDs that we hand out.
        callback({ { evaluateResultType, "placeholder_realm"_s, WTFMove(resultObject), nullptr } });
    });
}

void WebDriverBidiProcessor::evaluate(const String& expression, bool awaitPromise, Ref<JSON::Object>&& target, std::optional<Protocol::BidiScript::ResultOwnership>&&, RefPtr<JSON::Object>&& optionalSerializationOptions, std::optional<bool>&& optionalUserActivation, CommandCallbackOf<Protocol::BidiScript::EvaluateResultType, String, RefPtr<Protocol::BidiScript::RemoteValue>, RefPtr<Protocol::BidiScript::ExceptionDetails>>&& callback)
{
    RefPtr session = m_session.get();
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session, InternalError);

    // FIXME: handle non-BrowsingContext obtained from `Target`.
    std::optional<BrowsingContext> browsingContext = target->getString("context"_s);
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!browsingContext, InvalidParameter);
    ASYNC_FAIL_WITH_PREDEFINED_ERROR_IF(!session->webPageProxyForHandle(*browsingContext), WindowNotFound);

    // FIXME: handle `awaitPromise` option.
    // FIXME: handle `resultOwnership` option.
    // FIXME: handle `serializationOptions` option.

    String functionDeclaration = makeString("function() {\n return "_s, expression, "; \n}"_s);
    session->evaluateJavaScriptFunction(*browsingContext, emptyString(), functionDeclaration, JSON::Array::create(), false, optionalUserActivation.value_or(false), std::nullopt, [callback = WTFMove(callback)](Inspector::CommandResult<String>&& result) {
        auto evaluateResultType = result.has_value() ? Inspector::Protocol::BidiScript::EvaluateResultType::Success : Inspector::Protocol::BidiScript::EvaluateResultType::Exception;
        auto resultObject = Inspector::Protocol::BidiScript::RemoteValue::create()
            .setType(Inspector::Protocol::BidiScript::RemoteValueType::Object)
            .release();

        // FIXME: handle serializing different RemoteValue types as JSON here.
        if (result)
            resultObject->setValue(JSON::Value::create(WTFMove(result.value())));

        // FIXME: keep track of realm IDs that we hand out.
        callback({ { evaluateResultType, "placeholder_realm"_s, WTFMove(resultObject), nullptr } });
    });
}

} // namespace WebKit

#endif // ENABLE(WEBDRIVER_BIDI)
