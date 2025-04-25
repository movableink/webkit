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

#include "BidiBrowserAgent.h"
#include "BidiBrowsingContextAgent.h"
#include "BidiScriptAgent.h"
#include "Logging.h"
#include "WebAutomationSession.h"
#include <JavaScriptCore/InspectorBackendDispatcher.h>
#include <JavaScriptCore/InspectorFrontendRouter.h>
#include <wtf/TZoneMallocInlines.h>
#include <wtf/text/MakeString.h>

namespace WebKit {

using namespace Inspector;

WTF_MAKE_TZONE_ALLOCATED_IMPL(WebDriverBidiProcessor);

WebDriverBidiProcessor::WebDriverBidiProcessor(WebAutomationSession& session)
    : m_session(session)
    , m_frontendRouter(FrontendRouter::create())
    , m_backendDispatcher(BackendDispatcher::create(m_frontendRouter.copyRef()))
    , m_browserAgent(makeUnique<BidiBrowserAgent>(session, m_backendDispatcher))
    , m_browsingContextAgent(makeUnique<BidiBrowsingContextAgent>(session, m_backendDispatcher))
    , m_scriptAgent(makeUnique<BidiScriptAgent>(session, m_backendDispatcher))
    , m_browsingContextDomainNotifier(makeUnique<BidiBrowsingContextFrontendDispatcher>(m_frontendRouter))
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

} // namespace WebKit

#endif // ENABLE(WEBDRIVER_BIDI)
