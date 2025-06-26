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

#pragma once

#include "CachedResource.h"
#include "CachedResourceRequest.h"
#include "NetworkLoadMetrics.h"
#include <wtf/URL.h>
#include <QHash>
#include <QObject>
#include <QUrl>
#include <wtf/MonotonicTime.h>
#include <wtf/text/WTFString.h>

// Resource tracking types that are shared between WebCore and public API
// The public API header qwebresourcetypes.h defines identical types for external use
#ifndef QWEBRESOURCETYPES_H
struct QtResourceTimingInfo {
    qint64 domainLookupMs = -1;
    qint64 connectMs = -1;
    qint64 sslMs = -1;
    qint64 requestMs = -1;
    qint64 responseMs = -1;
    qint64 totalMs = 0;

    QtResourceTimingInfo() = default;
    QtResourceTimingInfo(qint64 total) : totalMs(total) { }
};

struct QtResourceRequestInfo {
    QString initiatorType;
    QString cachePolicy;
    QUrl referrer;
    bool corsEnabled = false;
    qint64 requestBodySize = 0;

    QtResourceRequestInfo() = default;
    QtResourceRequestInfo(const QString& type) : initiatorType(type) { }
};
#endif

namespace WebCore {

class LocalFrame;


struct QtResourceLoadInfo {
    QUrl url;
    QString type;
    MonotonicTime startTime;
    QtResourceRequestInfo requestInfo;
    qint64 size = 0;
    bool fromMemoryCache = false;
    bool finished = false;
    bool failed = false;

    QtResourceLoadInfo() = default;
    QtResourceLoadInfo(const QUrl& url, const QString& type, const QtResourceRequestInfo& reqInfo)
        : url(url), type(type), startTime(MonotonicTime::now()), requestInfo(reqInfo) { }
};

class ResourceLoadTrackerQt : public QObject {
    Q_OBJECT

public:
    static ResourceLoadTrackerQt& instance();

    void trackRequestStarted(const URL& url, CachedResource::Type type, const CachedResourceRequest& request, LocalFrame* frame = nullptr);
    void trackMemoryCacheHit(const URL& url, qint64 size, const NetworkLoadMetrics& metrics, LocalFrame* frame = nullptr);
    void trackRequestFinished(const CachedResource& resource, LocalFrame* frame = nullptr);
    void trackRequestFailed(const URL& url, LocalFrame* frame = nullptr);

    QHash<QUrl, QtResourceLoadInfo> currentRequests() const;
    int requestCount() const;

Q_SIGNALS:
    void resourceLoadStarted(const QUrl& url, const QString& type, const QtResourceRequestInfo& requestInfo, bool fromCache);
    void resourceLoadFinished(const QUrl& url, const QString& type, qint64 size, const QtResourceTimingInfo& timing, bool fromCache, bool success);

private:
    ResourceLoadTrackerQt();
    ~ResourceLoadTrackerQt();
    Q_DISABLE_COPY(ResourceLoadTrackerQt)

    QString resourceTypeToString(CachedResource::Type type) const;
    QString cachePolicyToString(FetchOptions::Cache cache) const;
    QtResourceRequestInfo extractRequestInfo(const CachedResourceRequest& request) const;
    QtResourceTimingInfo extractTimingInfo(const NetworkLoadMetrics& metrics, MonotonicTime startTime) const;
    void emitResourceStarted(const QUrl& url, const QString& type, const QtResourceRequestInfo& requestInfo, bool fromCache, LocalFrame* frame);
    void emitResourceFinished(const QUrl& url, const QString& type, qint64 size, const QtResourceTimingInfo& timing, bool fromCache, bool success, LocalFrame* frame);
    LocalFrame* currentFrame(LocalFrame* frame) const;

    QHash<QUrl, QtResourceLoadInfo> m_activeRequests;
};

} // namespace WebCore

Q_DECLARE_METATYPE(QtResourceTimingInfo)
Q_DECLARE_METATYPE(QtResourceRequestInfo)