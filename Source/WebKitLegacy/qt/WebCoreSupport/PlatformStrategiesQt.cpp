/*
 * Copyright (C) 2007 Staikos Computing Services Inc. <info@staikos.net>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 INdT - Instituto Nokia de Tecnologia
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

#include "PlatformStrategiesQt.h"

#include "qwebhistoryinterface.h"
#include "WebResourceLoadScheduler.h"

#include <QCoreApplication>
#include <QLocale>
#include <WebCore/AudioDestination.h>
#include <WebCore/BlobRegistryImpl.h>
#include <WebCore/IntSize.h>
#include <WebCore/Page.h>
#include <WebCore/MediaStrategy.h>
#include <wtf/MathExtras.h>
#include <wtf/NeverDestroyed.h>

using namespace WebCore;

class WebMediaStrategy final : public MediaStrategy {
private:
#if ENABLE(WEB_AUDIO)
    Ref<AudioDestination> createAudioDestination(AudioIOCallback& callback, const String& inputDeviceId,
        unsigned numberOfInputChannels, unsigned numberOfOutputChannels, float sampleRate) override
    {
        return AudioDestination::create(callback, inputDeviceId, numberOfInputChannels, numberOfOutputChannels, sampleRate);
    }
#endif
};

void PlatformStrategiesQt::initialize()
{
    static NeverDestroyed<PlatformStrategiesQt> platformStrategies;
    Q_UNUSED(platformStrategies);
}

PlatformStrategiesQt::PlatformStrategiesQt()
{
    setPlatformStrategies(this);
}

LoaderStrategy* PlatformStrategiesQt::createLoaderStrategy()
{
    return new WebResourceLoadScheduler;
}

MediaStrategy* PlatformStrategiesQt::createMediaStrategy()
{
    return new WebMediaStrategy;
}

PasteboardStrategy* PlatformStrategiesQt::createPasteboardStrategy()
{
    return nullptr;
}

class WebBlobRegistry final : public BlobRegistry {
private:
    void registerInternalFileBlobURL(const URL& url, Ref<BlobDataFileReference>&& reference, const String& path, const String& contentType) final { m_blobRegistry.registerInternalFileBlobURL(url, WTFMove(reference), contentType); }
    void registerInternalBlobURL(const URL& url, Vector<BlobPart>&& parts, const String& contentType) final { m_blobRegistry.registerInternalBlobURL(url, WTFMove(parts), contentType); }
    void registerInternalBlobURLOptionallyFileBacked(const URL& url, const URL& srcURL, RefPtr<BlobDataFileReference>&& reference, const String& contentType) final { m_blobRegistry.registerInternalBlobURLOptionallyFileBacked(url, srcURL, WTFMove(reference), contentType, { }); }
    void registerInternalBlobURLForSlice(const URL& url, const URL& srcURL, long long start, long long end, const String& contentType) final { m_blobRegistry.registerInternalBlobURLForSlice(url, srcURL, start, end, contentType); }
    void registerBlobURL(const URL& url, const URL& srcURL, const PolicyContainer& container, const std::optional<SecurityOriginData>& topOrigin) final { m_blobRegistry.registerBlobURL(url, srcURL, container, topOrigin); }
    void registerBlobURLHandle(const URL& url, const std::optional<SecurityOriginData>& topOrigin) final { m_blobRegistry.registerBlobURLHandle(url, topOrigin); }
    void unregisterBlobURL(const URL& url, const std::optional<SecurityOriginData>& topOrigin) final { m_blobRegistry.unregisterBlobURL(url, topOrigin); }
    void unregisterBlobURLHandle(const URL& url, const std::optional<SecurityOriginData>& topOrigin) final { m_blobRegistry.unregisterBlobURLHandle(url, topOrigin); }
    unsigned long long blobSize(const URL& url) final { return m_blobRegistry.blobSize(url); }
    void writeBlobsToTemporaryFilesForIndexedDB(const Vector<String>& blobURLs, CompletionHandler<void(Vector<String>&& filePaths)>&& completionHandler) final { m_blobRegistry.writeBlobsToTemporaryFilesForIndexedDB(blobURLs, WTFMove(completionHandler)); }

    BlobRegistryImpl* blobRegistryImpl() final { return &m_blobRegistry; }

    BlobRegistryImpl m_blobRegistry;
};

BlobRegistry* PlatformStrategiesQt::createBlobRegistry()
{
    return new WebBlobRegistry;
}
