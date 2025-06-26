/*
 * Copyright (C) 2025 Michael Nutt <michael@nuttnet.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "ResourceLoadTrackerQt.h"

#include "CachedResource.h"
#include "CachedResourceRequest.h"
#include "LocalFrame.h"
#include "Page.h"
#include "QWebPageClient.h"
#include <QUrl>
#include <wtf/MainThread.h>

namespace WebCore {

ResourceLoadTrackerQt::ResourceLoadTrackerQt()
{
}

ResourceLoadTrackerQt::~ResourceLoadTrackerQt()
{
}

ResourceLoadTrackerQt& ResourceLoadTrackerQt::instance()
{
    static ResourceLoadTrackerQt instance;
    return instance;
}

void ResourceLoadTrackerQt::trackRequestStarted(const URL& url, CachedResource::Type type, const CachedResourceRequest& request, LocalFrame* frame)
{
    if (url.isEmpty())
        return;

    QUrl qurl = url;
    QString typeString = resourceTypeToString(type);

    // Extract detailed request information
    QtResourceRequestInfo requestInfo = extractRequestInfo(request);

    QtResourceLoadInfo info(qurl, typeString, requestInfo);
    m_activeRequests[qurl] = info;

    emitResourceStarted(qurl, typeString, requestInfo, false, frame);
}

void ResourceLoadTrackerQt::trackMemoryCacheHit(const URL& url, qint64 size, const NetworkLoadMetrics& metrics, LocalFrame* frame)
{
    if (url.isEmpty())
        return;

    QUrl qurl = url;

    auto it = m_activeRequests.find(qurl);
    if (it != m_activeRequests.end()) {
        it->size = size;
        it->fromMemoryCache = true;
        it->finished = true;

        // Extract detailed timing information
        QtResourceTimingInfo timingInfo = extractTimingInfo(metrics, it->startTime);

        QString typeString = it->type;
        QtResourceRequestInfo requestInfo = it->requestInfo;
        m_activeRequests.erase(it);

        emitResourceFinished(qurl, typeString, size, timingInfo, true, true, frame);
    } else {
        // Memory cache hit without prior start tracking (can happen in some edge cases)
        QString typeString = QStringLiteral("unknown");

        QtResourceRequestInfo requestInfo(QStringLiteral("unknown"));
        QtResourceTimingInfo timingInfo = extractTimingInfo(metrics, MonotonicTime::now());

        emitResourceStarted(qurl, typeString, requestInfo, true, frame);
        emitResourceFinished(qurl, typeString, size, timingInfo, true, true, frame);
    }
}

void ResourceLoadTrackerQt::trackRequestFinished(const CachedResource& resource, LocalFrame* frame)
{
    QUrl qurl = resource.url();

    auto it = m_activeRequests.find(qurl);
    if (it != m_activeRequests.end()) {
        it->size = resource.encodedSize();
        it->finished = true;
        it->failed = (resource.status() == CachedResource::Status::LoadError || resource.status() == CachedResource::Status::DecodeError);

        // Extract timing info from resource's NetworkLoadMetrics if available
        QtResourceTimingInfo timingInfo;
        auto networkMetrics = const_cast<CachedResource&>(resource).takeNetworkLoadMetrics();
        if (networkMetrics) {
            timingInfo = extractTimingInfo(*networkMetrics, it->startTime);
        } else {
            // Fallback to simple duration calculation
            MonotonicTime endTime = MonotonicTime::now();
            timingInfo.totalMs = (endTime - it->startTime).millisecondsAs<qint64>();
        }

        QString typeString = it->type;
        qint64 size = it->size;
        bool fromCache = it->fromMemoryCache;
        bool success = !it->failed;

        m_activeRequests.erase(it);

        emitResourceFinished(qurl, typeString, size, timingInfo, fromCache, success, frame);
    }
}

void ResourceLoadTrackerQt::trackRequestFailed(const URL& url, LocalFrame* frame)
{
    if (url.isEmpty())
        return;

    QUrl qurl = url;

    auto it = m_activeRequests.find(qurl);
    if (it != m_activeRequests.end()) {
        MonotonicTime endTime = MonotonicTime::now();
        qint64 durationMs = (endTime - it->startTime).millisecondsAs<qint64>();

        QtResourceTimingInfo timingInfo(durationMs);
        QString typeString = it->type;
        qint64 size = it->size;
        bool fromCache = it->fromMemoryCache;

        m_activeRequests.erase(it);

        emitResourceFinished(qurl, typeString, size, timingInfo, fromCache, false, frame);
    }
}

QHash<QUrl, QtResourceLoadInfo> ResourceLoadTrackerQt::currentRequests() const
{
    return m_activeRequests;
}

int ResourceLoadTrackerQt::requestCount() const
{
    return m_activeRequests.size();
}

QString ResourceLoadTrackerQt::resourceTypeToString(CachedResource::Type type) const
{
    switch (type) {
    case CachedResource::Type::MainResource:
        return QStringLiteral("document");
    case CachedResource::Type::ImageResource:
        return QStringLiteral("image");
    case CachedResource::Type::CSSStyleSheet:
        return QStringLiteral("stylesheet");
    case CachedResource::Type::Script:
        return QStringLiteral("script");
    case CachedResource::Type::FontResource:
        return QStringLiteral("font");
    case CachedResource::Type::SVGFontResource:
        return QStringLiteral("svg-font");
    case CachedResource::Type::MediaResource:
        return QStringLiteral("media");
#if ENABLE(MODEL_ELEMENT)
    case CachedResource::Type::EnvironmentMapResource:
        return QStringLiteral("environment-map");
    case CachedResource::Type::ModelResource:
        return QStringLiteral("model");
#endif
    case CachedResource::Type::RawResource:
        return QStringLiteral("xhr");
    case CachedResource::Type::Icon:
        return QStringLiteral("icon");
    case CachedResource::Type::Beacon:
        return QStringLiteral("beacon");
    case CachedResource::Type::Ping:
        return QStringLiteral("ping");
#if ENABLE(XSLT)
    case CachedResource::Type::XSLStyleSheet:
        return QStringLiteral("xsl");
#endif
    case CachedResource::Type::LinkPrefetch:
        return QStringLiteral("prefetch");
#if ENABLE(VIDEO)
    case CachedResource::Type::TextTrackResource:
        return QStringLiteral("track");
#endif
#if ENABLE(APPLICATION_MANIFEST)
    case CachedResource::Type::ApplicationManifest:
        return QStringLiteral("manifest");
#endif
    case CachedResource::Type::SVGDocumentResource:
        return QStringLiteral("svg");
    }

    return QStringLiteral("unknown");
}

QString ResourceLoadTrackerQt::cachePolicyToString(FetchOptions::Cache cache) const
{
    switch (cache) {
    case FetchOptions::Cache::Default:
        return QStringLiteral("default");
    case FetchOptions::Cache::NoStore:
        return QStringLiteral("no-store");
    case FetchOptions::Cache::Reload:
        return QStringLiteral("reload");
    case FetchOptions::Cache::NoCache:
        return QStringLiteral("no-cache");
    case FetchOptions::Cache::ForceCache:
        return QStringLiteral("force-cache");
    case FetchOptions::Cache::OnlyIfCached:
        return QStringLiteral("only-if-cached");
    }

    return QStringLiteral("unknown");
}

QtResourceRequestInfo ResourceLoadTrackerQt::extractRequestInfo(const CachedResourceRequest& request) const
{
    QtResourceRequestInfo info;

    // Extract initiator type
    info.initiatorType = QString(request.initiatorType().string());

    // Extract cache policy
    info.cachePolicy = cachePolicyToString(request.options().cache);

    // Extract referrer
    if (!request.resourceRequest().httpReferrer().isEmpty()) {
        info.referrer = QUrl(request.resourceRequest().httpReferrer());
    }

    // Extract CORS info
    info.corsEnabled = (request.options().mode == FetchOptions::Mode::Cors);

    // Extract request body size if available
    if (request.resourceRequest().httpBody()) {
        info.requestBodySize = request.resourceRequest().httpBody()->lengthInBytes();
    }

    return info;
}

QtResourceTimingInfo ResourceLoadTrackerQt::extractTimingInfo(const NetworkLoadMetrics& metrics, MonotonicTime startTime) const
{
    QtResourceTimingInfo info;

    // Extract detailed timing breakdown from NetworkLoadMetrics (W3C Resource Timing API)
    if (metrics.domainLookupStart.secondsSinceEpoch().value() > 0 && metrics.domainLookupEnd.secondsSinceEpoch().value() > 0) {
        info.domainLookupMs = (metrics.domainLookupEnd - metrics.domainLookupStart).millisecondsAs<qint64>();
    }

    if (metrics.connectStart.secondsSinceEpoch().value() > 0 && metrics.connectEnd.secondsSinceEpoch().value() > 0) {
        info.connectMs = (metrics.connectEnd - metrics.connectStart).millisecondsAs<qint64>();
    }

    if (metrics.secureConnectionStart.secondsSinceEpoch().value() > 0 && metrics.connectEnd.secondsSinceEpoch().value() > 0) {
        info.sslMs = (metrics.connectEnd - metrics.secureConnectionStart).millisecondsAs<qint64>();
    }

    if (metrics.requestStart.secondsSinceEpoch().value() > 0 && metrics.responseStart.secondsSinceEpoch().value() > 0) {
        info.requestMs = (metrics.responseStart - metrics.requestStart).millisecondsAs<qint64>();
    }

    if (metrics.responseStart.secondsSinceEpoch().value() > 0 && metrics.responseEnd.secondsSinceEpoch().value() > 0) {
        info.responseMs = (metrics.responseEnd - metrics.responseStart).millisecondsAs<qint64>();
    }

    // Calculate total timing
    if (metrics.fetchStart.secondsSinceEpoch().value() > 0 && metrics.responseEnd.secondsSinceEpoch().value() > 0) {
        info.totalMs = (metrics.responseEnd - metrics.fetchStart).millisecondsAs<qint64>();
    } else {
        // Fallback to our tracked timing
        MonotonicTime endTime = MonotonicTime::now();
        info.totalMs = (endTime - startTime).millisecondsAs<qint64>();
    }

    return info;
}

void ResourceLoadTrackerQt::emitResourceStarted(const QUrl& url, const QString& type, const QtResourceRequestInfo& requestInfo, bool fromCache, LocalFrame* frame)
{
    if (!isMainThread()) {
        callOnMainThread([this, url, type, requestInfo, fromCache, frame]() {
            emitResourceStarted(url, type, requestInfo, fromCache, frame);
        });
        return;
    }

    Q_EMIT resourceLoadStarted(url, type, requestInfo, fromCache);
}

void ResourceLoadTrackerQt::emitResourceFinished(const QUrl& url, const QString& type, qint64 size, const QtResourceTimingInfo& timing, bool fromCache, bool success, LocalFrame* frame)
{
    if (!isMainThread()) {
        callOnMainThread([this, url, type, size, timing, fromCache, success, frame]() {
            emitResourceFinished(url, type, size, timing, fromCache, success, frame);
        });
        return;
    }

    Q_EMIT resourceLoadFinished(url, type, size, timing, fromCache, success);
}


LocalFrame* ResourceLoadTrackerQt::currentFrame(LocalFrame* frame) const
{
    // Helper to get the current frame context
    return frame;
}

} // namespace WebCore

#include "moc_ResourceLoadTrackerQt.cpp"