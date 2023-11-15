/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
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

#include "DragClientQt.h"

#include <WebCore/ChromeClient.h>
#include <WebCore/DataTransfer.h>
#include <WebCore/DragController.h>
#include <WebCore/EventHandler.h>
#include <WebCore/Frame.h>
#include <WebCore/Page.h>
#include <WebCore/Pasteboard.h>
#include <WebCore/PlatformMouseEvent.h>
#include <WebCore/SharedBuffer.h>

#include <QDrag>
#include <QMimeData>
#include <WebCore/QWebPageClient.h>

namespace WebCore {

static inline Qt::DropActions dragOperationsToDropActions(OptionSet<DragOperation> op)
{
    Qt::DropActions result = Qt::IgnoreAction;
    if (op.contains(DragOperation::Copy))
        result = Qt::CopyAction;
    if (op.contains(DragOperation::Move))
        result |= Qt::MoveAction;
    if (op.contains(DragOperation::Generic))
        result |= Qt::MoveAction;
    if (op.contains(DragOperation::Link))
        result |= Qt::LinkAction;
    return result;
}

static inline DragOperation dropActionToDragOperation(Qt::DropActions action)
{
    DragOperation result = DragOperation::Generic;
    if (action & Qt::CopyAction)
        result = DragOperation::Copy;
    if (action & Qt::LinkAction)
        result = DragOperation::Link;
    if (action & Qt::MoveAction)
        result = DragOperation::Move;
    return result;
}

void DragClientQt::willPerformDragDestinationAction(DragDestinationAction, const DragData&)
{
}

OptionSet<DragSourceAction> DragClientQt::dragSourceActionMaskForPoint(const IntPoint&)
{
    return WebCore::anyDragSourceAction();
}

void DragClientQt::willPerformDragSourceAction(DragSourceAction, const IntPoint&, DataTransfer&)
{
}

void DragClientQt::startDrag(DragItem dragItem, DataTransfer& dataTransfer, LocalFrame& frame)
{
#if ENABLE(DRAG_SUPPORT)
    DragImageRef dragImage = dragItem.image.get();
    auto dragImageOrigin = dragItem.dragLocationInContentCoordinates;
    auto eventPos = dragItem.eventPositionInContentCoordinates;
    QMimeData* clipboardData = dataTransfer.pasteboard().clipboardData();
    dataTransfer.pasteboard().invalidateWritableData();
    PlatformPageClient pageClient = m_chromeClient->platformPageClient();
    QObject* view = pageClient ? pageClient->ownerWidget() : 0;
    if (view) {
        QDrag* drag = new QDrag(view);
        if (!dragImage.isNull()) {
            drag->setPixmap(QPixmap::fromImage(WTFMove(dragImage)));
            drag->setHotSpot(IntPoint(eventPos - dragImageOrigin));
        } else if (clipboardData && clipboardData->hasImage())
            drag->setPixmap(qvariant_cast<QPixmap>(clipboardData->imageData()));
        OptionSet<DragOperation> dragOperationMask = dataTransfer.sourceOperationMask();
        drag->setMimeData(clipboardData);
        Qt::DropAction actualDropAction = drag->exec(dragOperationsToDropActions(dragOperationMask));

        // Send dragEnd event
        PlatformMouseEvent me(m_chromeClient->screenToRootView(QCursor::pos()), QCursor::pos(), MouseButton::Left, PlatformEvent::Type::MouseMoved, 0, { }, WallTime::now(), ForceAtClick, SyntheticClickType::NoTap, WebCore::mousePointerID);
        frame.eventHandler().dragSourceEndedAt(me, dropActionToDragOperation(actualDropAction));
    }
    frame.page()->dragController().dragEnded();
#endif
}


} // namespace WebCore
