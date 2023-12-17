/*
    Copyright (C) 2009 Jakub Wieczorek <faw217@gmail.com>

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

#include "qwebplugindatabase_p.h"

using namespace WebCore;

/*!
    \internal
    \typedef QWebPluginInfo::MimeType
    \since 4.6
    \brief Represents a single MIME type supported by a plugin.
*/

/*!
    \class QWebPluginInfo
    \internal
    \since 4.6
    \brief The QWebPluginInfo class represents a single Netscape plugin.

    A QWebPluginInfo object represents a Netscape plugin picked up by WebKit
    and included in the plugin database. This class contains information about
    the plugin, such as its name(), description(), a list of MIME types that it
    supports (can be accessed with mimeTypes()) and the path of the plugin
    file.

    Plugins can be enabled and disabled with setEnabled(). If a plugin is
    disabled, it will not be used by WebKit to handle supported MIME types. To
    check if a plugin is enabled or not, use enabled().

    \sa QWebPluginDatabase

    \deprecated Plugins have been removed from WebKit.
*/

/*!
    Constructs a null QWebPluginInfo.

    \deprecated Plugins have been removed from WebKit.
*/
QWebPluginInfo::QWebPluginInfo()
{
}


/*!
    Contructs a copy of \a other.

    \deprecated Plugins have been removed from WebKit.
*/
QWebPluginInfo::QWebPluginInfo(const QWebPluginInfo& other)
{
}

/*!
    Destroys the plugin info.

    \deprecated Plugins have been removed from WebKit.
*/
QWebPluginInfo::~QWebPluginInfo()
{
}

/*!
    Returns the name of the plugin.

    \sa description()

    \deprecated Plugins have been removed from WebKit.
*/
QString QWebPluginInfo::name() const
{
    return QString();
}

/*!
    Returns the description of the plugin.

    \sa name()

    \deprecated Plugins have been removed from WebKit.
*/
QString QWebPluginInfo::description() const
{
    return QString();
}

/*!
    Returns a list of MIME types supported by the plugin.

    \sa supportsMimeType()

    \deprecated Plugins have been removed from WebKit.
*/
QList<QWebPluginInfo::MimeType> QWebPluginInfo::mimeTypes() const
{
    return m_mimeTypes;
}

/*!
    Returns true if the plugin supports a specific \a mimeType; otherwise
    returns false.

    \sa mimeTypes()

    \deprecated Plugins have been removed from WebKit.
*/
bool QWebPluginInfo::supportsMimeType(const QString& mimeType) const
{
    return false;
}

/*!
    Returns an absolute path to the plugin file.

    \deprecated Plugins have been removed from WebKit.
*/
QString QWebPluginInfo::path() const
{
    return QString();
}

/*!
    Returns true if the plugin is a null plugin; otherwise returns false.

    \deprecated Plugins have been removed from WebKit.
*/
bool QWebPluginInfo::isNull() const
{
    return true;
}

/*!
    Enables or disables the plugin, depending on the \a enabled parameter.

    Disabled plugins will not be picked up by WebKit when looking for a plugin
    supporting a particular MIME type.

    \sa isEnabled()

    \deprecated Plugins have been removed from WebKit.
*/
void QWebPluginInfo::setEnabled(bool enabled)
{
    return;
}

/*!
    Returns true if the plugin is enabled; otherwise returns false.

    \sa setEnabled()

    \deprecated Plugins have been removed from WebKit.
*/
bool QWebPluginInfo::isEnabled() const
{
    return false;
}

/*!
    Returns true if this plugin info is the same as the \a other plugin info.

    \deprecated Plugins have been removed from WebKit.
*/
bool QWebPluginInfo::operator==(const QWebPluginInfo& other) const
{
    return false;
}

/*!
    Returns true if this plugin info is different from the \a other plugin info.

    \deprecated Plugins have been removed from WebKit.
*/
bool QWebPluginInfo::operator!=(const QWebPluginInfo& other) const
{
    return true;
}

/*!
    Assigns the \a other plugin info to this plugin info, and returns a reference
    to this plugin info.

    \deprecated Plugins have been removed from WebKit.
*/
QWebPluginInfo &QWebPluginInfo::operator=(const QWebPluginInfo& other)
{
    if (this == &other)
        return *this;

    m_mimeTypes = other.m_mimeTypes;

    return *this;
}

/*!
    \class QWebPluginDatabase
    \internal
    \since 4.6
    \brief The QWebPluginDatabase class provides an interface for managing
    Netscape plugins used by WebKit in QWebPages.

    The QWebPluginDatabase class is a database of Netscape plugins that are used
    by WebKit. The plugins are picked up by WebKit by looking up a set of search paths.
    The default set can be accessed using defaultSearchPaths(). The search paths
    can be changed, see searchPaths() and setSearchPaths(). Additional search paths
    can also be added using addSearchPath().

    The plugins that have been detected are exposed by the plugins() method.
    The list contains QWebPlugin objects that hold both the metadata and the MIME
    types that are supported by particular plugins.

    WebKit specifies a plugin for a MIME type by looking for the first plugin that
    supports the specific MIME type. To get a plugin, that is used by WebKit to
    handle a specific MIME type, you can use the pluginForMimeType() function.

    To change the way of resolving MIME types ambiguity, you can explicitly set
    a preferred plugin for a specific MIME type, using setPreferredPluginForMimeType().

    \sa QWebPluginInfo, QWebSettings::pluginDatabase()

    \deprecated Plugins have been removed from WebKit.
*/

QWebPluginDatabase::QWebPluginDatabase(QObject* parent)
    : QObject(parent)
{
    qWarning("Plugins have been removed from WebKit.");
}

QWebPluginDatabase::~QWebPluginDatabase()
{
}

/*!
    Returns a list of plugins installed in the search paths.

    This list will contain disabled plugins, although they will not be used by
    WebKit.

    \sa pluginForMimeType()

    \deprecated Plugins have been removed from WebKit.
*/
QList<QWebPluginInfo> QWebPluginDatabase::plugins() const
{
    qWarning("Plugins have been removed from WebKit.");
    QList<QWebPluginInfo> qwebplugins;
    return qwebplugins;
}

/*!
    Refreshes the plugin database, adds new plugins that have been found and removes
    the ones that are no longer available in the search paths.

    You can call this function when the set of plugins installed in the search paths
    changes. You do not need to call this function when changing search paths,
    in that case WebKit automatically refreshes the database.

    \deprecated Plugins have been removed from WebKit.
*/
void QWebPluginDatabase::refresh()
{
    qWarning("Plugins have been removed from WebKit.");
}

/*!
    Returns the plugin that is currently used by WebKit for a given \a mimeType.

    \sa setPreferredPluginForMimeType()

    \deprecated Plugins have been removed from WebKit.
*/
QWebPluginInfo QWebPluginDatabase::pluginForMimeType(const QString& mimeType)
{
    qWarning("Plugins have been removed from WebKit.");
    return QWebPluginInfo();
}

/*!
    Changes the preferred plugin for a given \a mimeType to \a plugin. The \a plugin
    has to support the given \a mimeType, otherwise the setting will have no effect.

    Calling the function with a null \a plugin resets the setting.

    \sa pluginForMimeType()

    \deprecated Plugins have been removed from WebKit.
*/
void QWebPluginDatabase::setPreferredPluginForMimeType(const QString& mimeType, const QWebPluginInfo& plugin)
{
}
