/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2011 Robert Hogan <robert@roberthogan.net>
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
#include "ThirdPartyCookiesQt.h"

#include "Document.h"
#include "NetworkStorageSession.h"
#include "RegistrableDomain.h"

#include <QList>
#include <QNetworkCookie>
#include <QNetworkCookieJar>

namespace WebCore {

bool thirdPartyCookiePolicyPermits(const NetworkStorageSession* storageSession, const QUrl& url, const QUrl& firstPartyUrl)
{
    if (!storageSession)
        return true;

    QNetworkCookieJar* jar = storageSession->cookieJar();
    if (!jar)
        return true;

    if (firstPartyUrl.isEmpty())
        return true;

    if (RegistrableDomain(URL(url)).matches(firstPartyUrl))
        return true;

    switch (storageSession->thirdPartyCookiePolicy()) {
    case ThirdPartyCookiePolicy::Allow:
        return true;
    case ThirdPartyCookiePolicy::Block:
        return false;
    case ThirdPartyCookiePolicy::AllowWithExistingCookies: {
        QList<QNetworkCookie> cookies = jar->cookiesForUrl(url);
        return !cookies.isEmpty();
    }
    default:
        break;
    }
    return false;
}

}
