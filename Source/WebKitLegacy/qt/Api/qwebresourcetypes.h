/*
    Copyright (C) 2025 Michael Nutt <michael@nuttnet.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBRESOURCETYPES_H
#define QWEBRESOURCETYPES_H

#include <QtWebKit/qwebkitglobal.h>
#include <QtCore/QString>
#include <QtCore/QUrl>

// These types are duplicated from WebCore to provide a clean public API
// They have the same layout and field names so Qt signals/slots can pass them directly

struct QWEBKIT_EXPORT QtResourceTimingInfo {
    qint64 domainLookupMs = -1;
    qint64 connectMs = -1;
    qint64 sslMs = -1;
    qint64 requestMs = -1;
    qint64 responseMs = -1;
    qint64 totalMs = 0;
};

struct QWEBKIT_EXPORT QtResourceRequestInfo {
    QString initiatorType;
    QString cachePolicy;
    QUrl referrer;
    bool corsEnabled = false;
    qint64 requestBodySize = 0;
};

#endif // QWEBRESOURCETYPES_H