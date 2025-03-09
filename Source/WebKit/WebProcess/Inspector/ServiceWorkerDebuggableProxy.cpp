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

#include "config.h"
#include "ServiceWorkerDebuggableProxy.h"

#include "Logging.h"
#include "WebProcessProxy.h"
#include "WebSWContextManagerConnectionMessages.h"
#include <JavaScriptCore/RemoteConnectionToTarget.h>

#if ENABLE(REMOTE_INSPECTOR)

#include <wtf/TZoneMallocInlines.h>

namespace WebKit {

WTF_MAKE_TZONE_ALLOCATED_IMPL(ServiceWorkerDebuggableProxy);

using namespace Inspector;

Ref<ServiceWorkerDebuggableProxy> ServiceWorkerDebuggableProxy::create(const String& url, WebCore::ServiceWorkerIdentifier identifier, WebProcessProxy& webProcessProxy)
{
    return adoptRef(*new ServiceWorkerDebuggableProxy(url, identifier, webProcessProxy));
}

ServiceWorkerDebuggableProxy::ServiceWorkerDebuggableProxy(const String& url, WebCore::ServiceWorkerIdentifier identifier, WebProcessProxy& webProcessProxy)
    : m_scopeURL(url)
    , m_identifier(identifier)
    , m_webProcessProxy(webProcessProxy)
{
}

void ServiceWorkerDebuggableProxy::connect(FrontendChannel& channel, bool, bool)
{
    RELEASE_LOG(Inspector, "ServiceWorkerDebuggableProxy::connect");
    if (RefPtr webProcessProxy = m_webProcessProxy.get())
        webProcessProxy->send(Messages::WebSWContextManagerConnection::ConnectToInspector(m_identifier), 0);
}

void ServiceWorkerDebuggableProxy::disconnect(FrontendChannel& channel)
{
    RELEASE_LOG(Inspector, "ServiceWorkerDebuggableProxy::disconnect");
    if (RefPtr webProcessProxy = m_webProcessProxy.get())
        webProcessProxy->send(Messages::WebSWContextManagerConnection::DisconnectFromInspector(m_identifier), 0);
}

void ServiceWorkerDebuggableProxy::dispatchMessageFromRemote(String&& message)
{
    RELEASE_LOG(Inspector, "ServiceWorkerDebuggableProxy::dispatchMessageFromRemote");
    if (RefPtr webProcessProxy = m_webProcessProxy.get())
        webProcessProxy->send(Messages::WebSWContextManagerConnection::DispatchMessageFromInspector(m_identifier, WTFMove(message)), 0);
}

} // namespace WebKit

#endif // ENABLE(REMOTE_INSPECTOR)
