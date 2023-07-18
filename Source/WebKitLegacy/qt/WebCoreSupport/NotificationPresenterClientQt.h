/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NotificationPresenterClientQt_h
#define NotificationPresenterClientQt_h

#include "QtPlatformPlugin.h"
#include <QMultiHash>
#include <QScopedPointer>
#include <wtf/CompletionHandler.h>
#include <WebCore/NotificationClient.h>
#include <WebCore/NotificationData.h>
#include <WebCore/NotificationPermission.h>
#include <WebCore/Timer.h>
#include <WebCore/LocalFrame.h>
#include <wtf/UUID.h>

class QWebFrameAdapter;
class QWebPageAdapter;

namespace WebCore {

class Document;
class Frame;
class ScriptExecutionContext;

class NotificationWrapper final : public QObject, public QWebNotificationData {
    Q_OBJECT
public:
    NotificationWrapper(NotificationData&&);
    ~NotificationWrapper() { }

    void close();
    void sendDisplayEvent();
    const QString title() const final;
    const QString message() const final;
    const QUrl iconUrl() const final;
    const QUrl openerPageUrl() const final;

    const NotificationData& notification() { return m_notification; };

public Q_SLOTS:
    void notificationClosed();
    void notificationClicked();

private:
    std::unique_ptr<QWebNotificationPresenter> m_presenter;
    const NotificationData m_notification;
    Timer m_closeTimer;
    Timer m_displayEventTimer;

    friend class NotificationPresenterClientQt;
};

#if ENABLE(NOTIFICATIONS)

typedef QHash <WTF::UUID, NotificationWrapper*> NotificationsQueue;

class NotificationPresenterClientQt final : public NotificationClient {
public:
    NotificationPresenterClientQt();
    ~NotificationPresenterClientQt();

    /* WebCore::NotificationClient interface */
    bool show(ScriptExecutionContext&, NotificationData&&, RefPtr<NotificationResources>&&, CompletionHandler<void()>&&) override;
    void cancel(NotificationData&&) override;
    void notificationObjectDestroyed(NotificationData&&) override;
    void notificationControllerDestroyed() override;
    void requestPermission(ScriptExecutionContext&, PermissionHandler&&) override;
    NotificationClient::Permission checkPermission(ScriptExecutionContext*) override;

    void cancel(NotificationWrapper*);

    void setNotificationsAllowedForFrame(LocalFrame*, bool allowed);

    static bool dumpNotification;

    void addClient() { m_clientCount++; }
#ifndef QT_NO_SYSTEMTRAYICON
    bool hasSystemTrayIcon() const { return !m_systemTrayIcon.isNull(); }
    void setSystemTrayIcon(QObject* icon) { m_systemTrayIcon.reset(icon); }
#endif
    void removeClient();
    static NotificationPresenterClientQt* notificationPresenter();

    void notificationClicked(NotificationWrapper*);
    void notificationClicked(const QString& title);
    void sendDisplayEvent(NotificationWrapper*);

    void clearCachedPermissions();

private:
    void displayNotification(NotificationData&&);
    void removeReplacedNotificationFromQueue(const NotificationData&);
    void dumpReplacedIdText(const NotificationData&);
    void dumpShowText(const NotificationData&);
    void detachNotification(WTF::UUID);
    QWebPageAdapter* toPage(ScriptExecutionContext&);
    QWebFrameAdapter* toFrame(ScriptExecutionContext&);

    int m_clientCount;
    struct CallbacksInfo {
        QWebFrameAdapter* m_frame;
        QList<PermissionHandler> m_permissionHandlers;
    };
    QHash<ScriptExecutionContext*,  CallbacksInfo > m_pendingPermissionRequests;
    QHash<ScriptExecutionContext*, NotificationClient::Permission> m_cachedPermissions;

    NotificationsQueue m_notifications;
    QtPlatformPlugin m_platformPlugin;
#ifndef QT_NO_SYSTEMTRAYICON
    QScopedPointer<QObject> m_systemTrayIcon;
#endif
};

#endif // ENABLE(NOTIFICATIONS)

}

#endif // NotificationPresenterClientQt_h
