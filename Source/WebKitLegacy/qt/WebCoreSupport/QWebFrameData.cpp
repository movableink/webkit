/*
 * Copyright (C) 2015 The Qt Company Ltd.
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
 *
 */

#include "QWebFrameData.h"

#include "FrameLoaderClientQt.h"
#include <WebCore/Document.h>
#include <WebCore/LocalFrame.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/HTMLFormElement.h>
#include <WebCore/Page.h>

using namespace WebCore;

QWebFrameData::QWebFrameData(WebCore::Page* parentPage, WebCore::HTMLFrameOwnerElement* ownerFrameElement, const WTF::String& frameName)
    : name(frameName)
    , ownerElement(ownerFrameElement)
    , page(parentPage)
{
    // mainframe is already created in WebCore::Page, just use it.
    if (!ownerElement) {
        frame = &downcast<WebCore::LocalFrame>(parentPage->mainFrame());
    } else {
        frame = LocalFrame::createSubframe(*page, makeUniqueRef<FrameLoaderClientQt>(), WebCore::FrameIdentifier::generate(), *ownerElement);
    }
    frameLoaderClient = static_cast<FrameLoaderClientQt*>(&frame->loader().client());

    // FIXME: All of the below should probably be moved over into WebCore
    frame->tree().setSpecifiedName(AtomString(String(name)));
}
