/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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

#ifndef GeolocationClientQt_h
#define GeolocationClientQt_h

#include <QGeoPositionInfo>
#include <QObject>
#include <WebCore/GeolocationClient.h>
#include <WebCore/GeolocationPositionData.h>
#include <wtf/RefPtr.h>
#include <wtf/TZoneMalloc.h>

QT_BEGIN_NAMESPACE
class QGeoPositionInfoSource;
QT_END_NAMESPACE

class QWebPageAdapter;

namespace WebCore {

// This class provides an implementation of a GeolocationClient for QtWebkit.
class GeolocationClientQt final : public QObject, public GeolocationClient {
    WTF_MAKE_TZONE_ALLOCATED(GeolocationClientQt);
    WTF_OVERRIDE_DELETE_FOR_CHECKED_PTR(GeolocationClientQt);

    Q_OBJECT
public:
    GeolocationClientQt(const QWebPageAdapter*);
    ~GeolocationClientQt() override;

    bool operator==(const WebCore::GeolocationClientQt& other) const {
        return m_webPage == other.m_webPage;
    }

    void geolocationDestroyed() override;
    void startUpdating(const String& authorizationToken, bool enableHighAccuracy) override;
    void stopUpdating() override;

    void setEnableHighAccuracy(bool) override;
    std::optional<GeolocationPositionData> lastPosition() override { return m_lastPosition; }

    void requestPermission(Geolocation&) override;
    void cancelPermissionRequest(Geolocation&) override;

private Q_SLOTS:
    void positionUpdated(const QGeoPositionInfo&);

private:
    const QWebPageAdapter* m_webPage;
    std::optional<GeolocationPositionData> m_lastPosition;
    QGeoPositionInfoSource* m_location;
};

} // namespace WebCore

#endif // GeolocationClientQt_h
