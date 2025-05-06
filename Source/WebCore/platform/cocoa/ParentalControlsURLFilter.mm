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

#import "config.h"
#import "ParentalControlsURLFilter.h"

#if HAVE(WEBCONTENTRESTRICTIONS)

#import <wtf/CompletionHandler.h>
#import <wtf/MainThread.h>
#import <wtf/URL.h>
#import <wtf/cocoa/VectorCocoa.h>

#import <pal/cocoa/WebContentRestrictionsSoftLink.h>

namespace WebCore {

#if HAVE(WEBCONTENTRESTRICTIONS_PATH_SPI)

static bool wcrBrowserEngineClientEnabled(const String& path)
{
    if (!path.isEmpty())
        return [PAL::getWCRBrowserEngineClientClass() shouldEvaluateURLsForConfigurationAtPath:path.createNSString().get()];

    return [PAL::getWCRBrowserEngineClientClass() shouldEvaluateURLs];
}

ParentalControlsURLFilter& ParentalControlsURLFilter::filterWithConfigurationPath(const String& path)
{
    // Coalesce null string into the empty string.
    String key = path.isEmpty() ? emptyString() : path;

    static MainThreadNeverDestroyed<HashMap<String, UniqueRef<ParentalControlsURLFilter>>> map;

    auto iterator = map->find(key);
    if (iterator != map->end())
        return iterator->value;

    return map->set(key, UniqueRef(*new ParentalControlsURLFilter(key))).iterator->value;
}

ParentalControlsURLFilter::ParentalControlsURLFilter(const String& configurationPath)
{
    if (wcrBrowserEngineClientEnabled(configurationPath)) {
        if (!configurationPath.isEmpty())
            m_wcrBrowserEngineClient = adoptNS([PAL::allocWCRBrowserEngineClientInstance() initWithConfigurationAtPath:configurationPath.createNSString().get()]);
        else
            m_wcrBrowserEngineClient = adoptNS([PAL::allocWCRBrowserEngineClientInstance() init]);
    }
}

#else

static bool wcrBrowserEngineClientEnabled()
{
    return [PAL::getWCRBrowserEngineClientClass() shouldEvaluateURLs];
}

ParentalControlsURLFilter& ParentalControlsURLFilter::singleton()
{
    static MainThreadNeverDestroyed<UniqueRef<ParentalControlsURLFilter>> filter = UniqueRef(*new ParentalControlsURLFilter);
    return filter.get();
}

ParentalControlsURLFilter::ParentalControlsURLFilter()
{
    if (wcrBrowserEngineClientEnabled())
        m_wcrBrowserEngineClient = adoptNS([PAL::allocWCRBrowserEngineClientInstance() init]);
}

#endif

bool ParentalControlsURLFilter::isEnabled() const
{
    return !!m_wcrBrowserEngineClient;
}

void ParentalControlsURLFilter::isURLAllowed(const URL& url, CompletionHandler<void(bool, NSData *)>&& completionHandler, const WorkQueue& completionHandlerQueue)
{
    ASSERT(isMainThread());

    if (!m_wcrBrowserEngineClient)
        return completionHandler(true, nullptr);

    [m_wcrBrowserEngineClient evaluateURL:url.createNSURL().get() withCompletion:makeBlockPtr([completionHandler = WTFMove(completionHandler)](BOOL shouldBlock, NSData *replacementData) mutable {
        ASSERT(isMainThread());

        completionHandler(!shouldBlock, replacementData);
    }).get() onCompletionQueue:completionHandlerQueue.dispatchQueue()];
}

void ParentalControlsURLFilter::allowURL(const URL& url, CompletionHandler<void(bool)>&& completionHandler)
{
    ASSERT(isMainThread());

    if (!m_wcrBrowserEngineClient)
        return completionHandler(true);

    [m_wcrBrowserEngineClient allowURL:url.createNSURL().get() withCompletion:makeBlockPtr([completionHandler = WTFMove(completionHandler)](BOOL didAllow, NSError *) mutable {
        ASSERT(isMainThread());

        completionHandler(didAllow);
    }).get()];
}

} // namespace WebCore

#endif // HAVE(WEBCONTENTRESTRICTIONS)
