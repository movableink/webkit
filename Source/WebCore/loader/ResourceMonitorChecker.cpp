/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
#include "ResourceMonitorChecker.h"

#include <wtf/CryptographicallyRandomNumber.h>
#include <wtf/StdLibExtras.h>

namespace WebCore {

#if ENABLE(CONTENT_EXTENSIONS)

ResourceMonitorChecker& ResourceMonitorChecker::singleton()
{
    static MainThreadNeverDestroyed<ResourceMonitorChecker> globalChecker;
    return globalChecker;
}

ResourceMonitorChecker::ResourceMonitorChecker()
    : m_workQueue { WorkQueue::create("ResourceMonitorChecker Work Queue"_s) }
{
}

void ResourceMonitorChecker::checkEligibility(ContentExtensions::ResourceLoadInfo&& info, CompletionHandler<void(Eligibility)>&& completionHandler)
{
    ASSERT(isMainThread());

    protectedWorkQueue()->dispatch([this, info = crossThreadCopy(WTFMove(info)), completionHandler = WTFMove(completionHandler)] mutable {
        Eligibility eligibility = Eligibility::Unsure;
        {
            Locker locker { m_lock };
            eligibility = checkEligibilityWithLock(info);
        }

        callOnMainRunLoop([eligibility, completionHandler = WTFMove(completionHandler)] mutable {
            completionHandler(eligibility);
        });
    });
}

ResourceMonitor::Eligibility ResourceMonitorChecker::checkEligibilityForTesting(const ContentExtensions::ResourceLoadInfo& info)
{
    ASSERT(isMainThread());

    Locker locker { m_lock };
    return checkEligibilityWithLock(info);
}

ResourceMonitor::Eligibility ResourceMonitorChecker::checkEligibilityWithLock(const ContentExtensions::ResourceLoadInfo& info)
{
    if (!m_ruleList)
        return Eligibility::Unsure;

    auto matched = m_ruleList->processContentRuleListsForResourceMonitoring(info.resourceURL, info.mainDocumentURL, info.frameURL, info.type);
    return matched ? Eligibility::Eligible : Eligibility::NotEligible;
}

void ResourceMonitorChecker::setContentRuleList(ContentExtensions::ContentExtensionsBackend&& backend)
{
    ASSERT(isMainThread());

    Locker locker { m_lock };
    m_ruleList = makeUnique<ContentExtensions::ContentExtensionsBackend>(WTFMove(backend));
}

#endif

} // namespace WebCore
