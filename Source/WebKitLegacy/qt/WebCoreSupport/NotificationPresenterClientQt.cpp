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

#include "NotificationPresenterClientQt.h"

#include "QWebFrameAdapter.h"
#include "QWebPageAdapter.h"
#include "qwebkitglobal.h"

#include <WebCore/Document.h>
#include <WebCore/Event.h>
#include <WebCore/EventNames.h>
#include <WebCore/Frame.h>
#include <WebCore/Notification.h>
#include <WebCore/NotificationData.h>
#include <WebCore/Page.h>
#include <WebCore/ScriptExecutionContext.h>
#include <WebCore/FrameDestructionObserverInlines.h>
#include <wtf/UUID.h>

namespace WebCore {

#if ENABLE(NOTIFICATIONS)

const Seconds notificationTimeout = 10_s;

bool NotificationPresenterClientQt::dumpNotification = false;

NotificationPresenterClientQt* s_notificationPresenter = 0;

NotificationPresenterClientQt* NotificationPresenterClientQt::notificationPresenter()
{
    if (s_notificationPresenter)
        return s_notificationPresenter;

    s_notificationPresenter = new NotificationPresenterClientQt();
    return s_notificationPresenter;
}

#endif

NotificationWrapper::NotificationWrapper(NotificationData&& notification)
    : m_notification(WTFMove(notification))
    , m_closeTimer(*this, &NotificationWrapper::close)
    , m_displayEventTimer(*this, &NotificationWrapper::sendDisplayEvent)
{
#if ENABLE(NOTIFICATIONS)
    m_presenter = nullptr;
#endif
}

void NotificationWrapper::close()
{
#if ENABLE(NOTIFICATIONS)
    NotificationPresenterClientQt::notificationPresenter()->cancel(this);
#endif
}

void NotificationWrapper::sendDisplayEvent()
{
#if ENABLE(NOTIFICATIONS)
    NotificationPresenterClientQt::notificationPresenter()->sendDisplayEvent(this);
#endif
}

const QString NotificationWrapper::title() const
{
#if ENABLE(NOTIFICATIONS)
    return m_notification.title;
#endif
    return QString();
}

const QString NotificationWrapper::message() const
{
#if ENABLE(NOTIFICATIONS)
    return m_notification.body;
#endif
    return QString();
}

const QUrl NotificationWrapper::iconUrl() const
{
#if ENABLE(NOTIFICATIONS)
    return QUrl(m_notification.iconURL);
#endif
    return QUrl();
}

const QUrl NotificationWrapper::openerPageUrl() const
{
    QUrl url;
#if ENABLE(NOTIFICATIONS)
    // FIXME: No longer possible to get scriptExecutionContext here
    //
    // Notification
    // NotificationData* notification = NotificationPresenterClientQt::notificationPresenter()->notificationForWrapper(this);
    // if (notification) {
    //     if (notification->scriptExecutionContext())
    //         url = static_cast<Document*>(notification->scriptExecutionContext())->page()->mainFrame().document()->url();
    // }
#endif
    return url;
}

void NotificationWrapper::notificationClicked()
{
#if ENABLE(NOTIFICATIONS)
    NotificationPresenterClientQt::notificationPresenter()->notificationClicked(this);
#endif
}

void NotificationWrapper::notificationClosed()
{
#if ENABLE(NOTIFICATIONS)
    NotificationPresenterClientQt::notificationPresenter()->cancel(this);
#endif
}

#if ENABLE(NOTIFICATIONS)

NotificationPresenterClientQt::NotificationPresenterClientQt() : m_clientCount(0)
{
}

NotificationPresenterClientQt::~NotificationPresenterClientQt()
{
    while (!m_notifications.isEmpty()) {
        NotificationsQueue::Iterator iter = m_notifications.begin();
        detachNotification(iter.key());
    }
}

void NotificationPresenterClientQt::removeClient()
{
    m_clientCount--;
    if (!m_clientCount) {
        s_notificationPresenter = 0;
        delete this;
    }
}

bool NotificationPresenterClientQt::show(ScriptExecutionContext& context, NotificationData&& notification, RefPtr<NotificationResources>&&, CompletionHandler<void()>&& completionHandler)
{
    if (!notification.tag.isEmpty())
        removeReplacedNotificationFromQueue(notification);
    if (dumpNotification)
        dumpShowText(notification);

    displayNotification(WTFMove(notification));
    completionHandler();

    return true;
}

void NotificationPresenterClientQt::displayNotification(NotificationData&& notification)
{
    NotificationWrapper* wrapper = new NotificationWrapper(WTFMove(notification));
    m_notifications.insert(notification.notificationID, wrapper);

    QString title = notification.title;
    QString message = notification.body;

    if (m_platformPlugin.plugin() && m_platformPlugin.plugin()->supportsExtension(QWebKitPlatformPlugin::Notifications))
        wrapper->m_presenter = m_platformPlugin.createNotificationPresenter();

    if (!wrapper->m_presenter) {
#ifndef QT_NO_SYSTEMTRAYICON
        if (!dumpNotification)
            wrapper->m_closeTimer.startOneShot(notificationTimeout);
#endif
    }

    wrapper->m_displayEventTimer.startOneShot(0_s);

    // Make sure the notification was not cancelled during handling the display event
    if (!m_notifications.contains(notification.notificationID))
        return;

    if (wrapper->m_presenter) {
        wrapper->connect(wrapper->m_presenter.get(), SIGNAL(notificationClosed()), wrapper, SLOT(notificationClosed()), Qt::QueuedConnection);
        wrapper->connect(wrapper->m_presenter.get(), SIGNAL(notificationClicked()), wrapper, SLOT(notificationClicked()));
        wrapper->m_presenter->showNotification(wrapper);
        return;
    }

#ifndef QT_NO_SYSTEMTRAYICON
    wrapper->connect(m_systemTrayIcon.data(), SIGNAL(messageClicked()), wrapper, SLOT(notificationClicked()));
    QMetaObject::invokeMethod(m_systemTrayIcon.data(), "show");
    QMetaObject::invokeMethod(m_systemTrayIcon.data(), "showMessage", Q_ARG(QString, notification.title), Q_ARG(QString, notification.body));
#endif
}

void NotificationPresenterClientQt::cancel(NotificationData&& notification)
{
    if (dumpNotification)
        printf("DESKTOP NOTIFICATION CLOSED: %s\n", QString(notification.title).toUtf8().constData());

    NotificationsQueue::Iterator iter = m_notifications.find(notification.notificationID);
    if (iter != m_notifications.end()) {
        cancel(iter.value());
        detachNotification(notification.notificationID);
    }
}

void NotificationPresenterClientQt::cancel(NotificationWrapper* wrapper)
{
    Notification::ensureOnNotificationThread(wrapper->notification(), [](auto* notification) {
       if (notification)
           notification->dispatchCloseEvent();
    });
}

void NotificationPresenterClientQt::notificationClicked(NotificationWrapper* wrapper)
{
    Notification::ensureOnNotificationThread(wrapper->notification(), [](auto* notification) {
        if (notification) {
            UserGestureIndicator gestureIndicator(IsProcessingUserGesture::Yes);
            notification->dispatchClickEvent();
        }
    });
}

void NotificationPresenterClientQt::notificationClicked(const QString& title)
{
    if (!dumpNotification)
        return;
    NotificationsQueue::ConstIterator end = m_notifications.end();
    NotificationsQueue::ConstIterator iter = m_notifications.begin();
    NotificationWrapper* notificationWrapper = 0;
    while (iter != end) {
        notificationWrapper = iter.value();
        QString notificationTitle = notificationWrapper->title();
        if (notificationTitle == title)
            break;
        iter++;
    }

    if (notificationWrapper)
        notificationClicked(notificationWrapper);
}

void NotificationPresenterClientQt::notificationObjectDestroyed(NotificationData&& notification)
{
    // Called from ~Notification(), Remove the entry from the notifications list and delete the icon.
    NotificationsQueue::Iterator iter = m_notifications.find(notification.notificationID);
    if (iter != m_notifications.end())
        delete m_notifications.take(notification.notificationID);
}

void NotificationPresenterClientQt::notificationControllerDestroyed()
{
}

void NotificationPresenterClientQt::requestPermission(ScriptExecutionContext& context, PermissionHandler&& permissionHandler)
{
    if (dumpNotification)
        printf("DESKTOP NOTIFICATION PERMISSION REQUESTED: %s\n", QString(context.securityOrigin()->toString()).toUtf8().constData());

    NotificationClient::Permission permission = checkPermission(&context);
    if (permission != NotificationClient::Permission::Default) {
        permissionHandler(permission);
        return;
    }

    // QTFIXME
    permissionHandler(Permission::Denied);
}

NotificationClient::Permission NotificationPresenterClientQt::checkPermission(ScriptExecutionContext* context)
{
    return m_cachedPermissions.value(context, NotificationClient::Permission::Default);
}

void NotificationPresenterClientQt::setNotificationsAllowedForFrame(LocalFrame* frame, bool allowed)
{
    ASSERT(frame->document());
    if (!frame->document())
        return;

    NotificationClient::Permission permission = allowed ? NotificationClient::Permission::Granted : NotificationClient::Permission::Denied;
    m_cachedPermissions.insert(frame->document(), permission);
}

void NotificationPresenterClientQt::sendDisplayEvent(NotificationWrapper* wrapper)
{
    Notification::ensureOnNotificationThread(wrapper->notification(), [](auto* notification) {
       if (notification)
           notification->dispatchShowEvent();
    });
}

void NotificationPresenterClientQt::clearCachedPermissions()
{
    m_cachedPermissions.clear();
}

void NotificationPresenterClientQt::removeReplacedNotificationFromQueue(const NotificationData& notification)
{
    NotificationWrapper* oldNotificationWrapper = 0;
    NotificationsQueue::Iterator end = m_notifications.end();
    NotificationsQueue::Iterator iter = m_notifications.begin();

    while (iter != end) {
        if (iter.value()->notification().tag == notification.tag) {
            oldNotificationWrapper = iter.value();
            break;
        }
        iter++;
    }

    if (oldNotificationWrapper) {
        if (dumpNotification)
            dumpReplacedIdText(oldNotificationWrapper->notification());
        cancel(oldNotificationWrapper);
        detachNotification(oldNotificationWrapper->notification().notificationID);
    }
}

void NotificationPresenterClientQt::detachNotification(WTF::UUID notificationID)
{
    delete m_notifications.take(notificationID);
}

void NotificationPresenterClientQt::dumpReplacedIdText(const NotificationData& notification)
{
    printf("REPLACING NOTIFICATION %s\n", QString(notification.title).toUtf8().constData());
}

void NotificationPresenterClientQt::dumpShowText(const NotificationData& notification)
{
    printf("DESKTOP NOTIFICATION:%s icon %s, title %s, text %s\n",
        notification.direction == NotificationDirection::Rtl ? "(RTL)" : "",
        QString(notification.iconURL).toUtf8().constData(), QString(notification.title).toUtf8().constData(),
        QString(notification.body).toUtf8().constData());
}

QWebPageAdapter* NotificationPresenterClientQt::toPage(ScriptExecutionContext& context)
{
    if (context.isWorkerGlobalScope())
        return 0;

    Document* document = static_cast<Document*>(&context);

    Page* page = document->page();
    if (!page)
        return 0;

    return QWebPageAdapter::kit(page);
}

QWebFrameAdapter* NotificationPresenterClientQt::toFrame(ScriptExecutionContext& context)
{
    if (context.isWorkerGlobalScope())
        return 0;

    Document* document = static_cast<Document*>(&context);
    if (!document || !document->frame())
        return 0;

    return QWebFrameAdapter::kit(document->frame());
}

#endif // ENABLE(NOTIFICATIONS)
}

#include "moc_NotificationPresenterClientQt.cpp"
