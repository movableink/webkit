/*
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Charles Samuels <charles@kde.org>
 * Copyright (C) 2008, 2009 Holger Hans Peter Freyther
 * Copyright (C) 2010 University of Szeged
 *
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "Cursor.h"

#include "Image.h"
#include "IntPoint.h"

#include "NotImplemented.h"

#include <QImage>
#include <QPixmap>
#include <stdio.h>
#include <stdlib.h>

#undef CopyCursor

namespace WebCore {

#ifndef QT_NO_CURSOR
static std::optional<QCursor> createCustomCursor(Image* image, const IntPoint& hotSpot)
{
    QImage nativeImage = image->nativeImageForCurrentFrame()->platformImage();
    if (nativeImage.isNull())
        return std::nullopt;
    IntPoint effectiveHotSpot = determineHotSpot(image, hotSpot);
    return QCursor(QPixmap::fromImage(WTFMove(nativeImage)), effectiveHotSpot.x(), effectiveHotSpot.y());
}
#endif

void Cursor::ensurePlatformCursor() const
{
#ifndef QT_NO_CURSOR
    if (m_platformCursor)
        return;

    switch (m_type) {
    case Type::Pointer:
        m_platformCursor = QCursor(Qt::ArrowCursor);
        break;
    case Type::Cross:
        m_platformCursor = QCursor(Qt::CrossCursor);
        break;
    case Type::Hand:
        m_platformCursor = QCursor(Qt::PointingHandCursor);
        break;
    case Type::IBeam:
        m_platformCursor = QCursor(Qt::IBeamCursor);
        break;
    case Type::Wait:
        m_platformCursor = QCursor(Qt::WaitCursor);
        break;
    case Type::Help:
        m_platformCursor = QCursor(Qt::WhatsThisCursor);
        break;
    case Type::EastResize:
    case Type::EastPanning:
        m_platformCursor = QCursor(Qt::SizeHorCursor);
        break;
    case Type::NorthResize:
    case Type::NorthPanning:
        m_platformCursor = QCursor(Qt::SizeVerCursor);
        break;
    case Type::NorthEastResize:
    case Type::NorthEastPanning:
        m_platformCursor = QCursor(Qt::SizeBDiagCursor);
        break;
    case Type::NorthWestResize:
    case Type::NorthWestPanning:
        m_platformCursor = QCursor(Qt::SizeFDiagCursor);
        break;
    case Type::SouthResize:
    case Type::SouthPanning:
        m_platformCursor = QCursor(Qt::SizeVerCursor);
        break;
    case Type::SouthEastResize:
    case Type::SouthEastPanning:
        m_platformCursor = QCursor(Qt::SizeFDiagCursor);
        break;
    case Type::SouthWestResize:
    case Type::SouthWestPanning:
        m_platformCursor = QCursor(Qt::SizeBDiagCursor);
        break;
    case Type::WestResize:
    case Type::WestPanning:
        m_platformCursor = QCursor(Qt::SizeHorCursor);
        break;
    case Type::NorthSouthResize:
        m_platformCursor = QCursor(Qt::SizeVerCursor);
        break;
    case Type::EastWestResize:
        m_platformCursor = QCursor(Qt::SizeHorCursor);
        break;
    case Type::NorthEastSouthWestResize:
        m_platformCursor = QCursor(Qt::SizeBDiagCursor);
        break;
    case Type::NorthWestSouthEastResize:
        m_platformCursor = QCursor(Qt::SizeFDiagCursor);
        break;
    case Type::ColumnResize:
        m_platformCursor = QCursor(Qt::SplitHCursor);
        break;
    case Type::RowResize:
        m_platformCursor = QCursor(Qt::SplitVCursor);
        break;
    case Type::MiddlePanning:
    case Type::Move:
        m_platformCursor = QCursor(Qt::SizeAllCursor);
        break;
    case Type::None:
        m_platformCursor = QCursor(Qt::BlankCursor);
        break;
    case Type::NoDrop:
    case Type::NotAllowed:
        m_platformCursor = QCursor(Qt::ForbiddenCursor);
        break;
    case Type::Grab:
    case Type::Grabbing:
        notImplemented();
        m_platformCursor = QCursor(Qt::ArrowCursor);
        break;
    case Type::VerticalText:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/verticalTextCursor.png")), 7, 7);
        break;
    case Type::Cell:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/cellCursor.png")), 7, 7);
        break;
    case Type::ContextMenu:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/contextMenuCursor.png")), 3, 2);
        break;
    case Type::Alias:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/aliasCursor.png")), 11, 3);
        break;
    case Type::Progress:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/progressCursor.png")), 3, 2);
        break;
    case Type::Copy:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/copyCursor.png")), 3, 2);
        break;
    case Type::ZoomIn:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/zoomInCursor.png")), 7, 7);
        break;
    case Type::ZoomOut:
        m_platformCursor = QCursor(QPixmap(QStringLiteral(":/webkit/resources/zoomOutCursor.png")), 7, 7);
        break;
    case Type::Custom:
        m_platformCursor = createCustomCursor(m_image.get(), m_hotSpot);
        if (!m_platformCursor)
            m_platformCursor = QCursor(Qt::ArrowCursor);
        break;
    default:
        ASSERT_NOT_REACHED();
    }
#endif
}

}

// vim: ts=4 sw=4 et
