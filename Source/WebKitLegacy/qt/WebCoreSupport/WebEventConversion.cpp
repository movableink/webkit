/*
    Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2006 Zack Rusin <zack@kde.org>
    Copyright (C) 2011 Research In Motion Limited.

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

#include "WebEventConversion.h"

#include <QTouchEvent>
#include <QWheelEvent>
#include <WebCore/PlatformGestureEvent.h>
#include <WebCore/PlatformMouseEvent.h>
#include <WebCore/PlatformTouchEvent.h>
#include <WebCore/PlatformTouchPoint.h>
#include <WebCore/PlatformWheelEvent.h>
#include <wtf/WallTime.h>

namespace WebCore {

static OptionSet<PlatformEvent::Modifier> mouseEventModifiersFromQtKeyboardModifiers(Qt::KeyboardModifiers keyboardModifiers)
{
    OptionSet<PlatformEvent::Modifier> modifiers;
    if (keyboardModifiers & Qt::ShiftModifier)
        modifiers.add(PlatformEvent::Modifier::ShiftKey);
    if (keyboardModifiers & Qt::ControlModifier)
        modifiers.add(PlatformEvent::Modifier::ControlKey);
    if (keyboardModifiers & Qt::AltModifier)
        modifiers.add(PlatformEvent::Modifier::AltKey);
    if (keyboardModifiers & Qt::MetaModifier)
        modifiers.add(PlatformEvent::Modifier::MetaKey);
    return modifiers;
}

static void mouseEventTypeAndMouseButtonFromQEvent(const QEvent* event, PlatformEvent::Type& mouseEventType, MouseButton& mouseButton)
{
    switch (event->type()) {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
        mouseEventType = PlatformEvent::Type::MousePressed;
        break;
    case QEvent::MouseButtonRelease:
        mouseEventType = PlatformEvent::Type::MouseReleased;
        break;
    case QEvent::MouseMove:
        mouseEventType = PlatformEvent::Type::MouseMoved;
        break;
    default:
        ASSERT_NOT_REACHED();
        mouseEventType = PlatformEvent::Type::MouseMoved;
        break;
    }

    Qt::MouseButtons mouseButtons;

    const QMouseEvent* mouseEvent = static_cast<const QMouseEvent*>(event);
    mouseButtons = mouseEventType == PlatformEvent::Type::MouseMoved ? mouseEvent->buttons() : mouseEvent->button();


    if (mouseButtons & Qt::LeftButton)
        mouseButton = MouseButton::Left;
    else if (mouseButtons & Qt::RightButton)
        mouseButton = MouseButton::Right;
    else if (mouseButtons & Qt::MiddleButton)
        mouseButton = MouseButton::Middle;
    else
        mouseButton = MouseButton::None;
}

class WebKitPlatformMouseEvent : public PlatformMouseEvent {
public:
    WebKitPlatformMouseEvent(QInputEvent*, int clickCount);
};

WebKitPlatformMouseEvent::WebKitPlatformMouseEvent(QInputEvent* event, int clickCount)
{
    m_timestamp = WTF::WallTime::now();

    bool isContextMenuEvent = false;
#ifndef QT_NO_CONTEXTMENU
    if (event->type() == QEvent::ContextMenu) {
        isContextMenuEvent = true;
        m_type = PlatformEvent::Type::MousePressed;
        QContextMenuEvent* ce = static_cast<QContextMenuEvent*>(event);
        m_position = IntPoint(ce->pos());
        m_globalPosition = IntPoint(ce->globalPos());
        m_button = MouseButton::Right;
    }
#endif
    if (!isContextMenuEvent) {
        PlatformEvent::Type type;
        mouseEventTypeAndMouseButtonFromQEvent(event, type, m_button);
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        m_type = type;
        m_position = IntPoint(mouseEvent->pos());
        m_globalPosition = IntPoint(mouseEvent->globalPos());
    }

    m_clickCount = clickCount;
    m_modifiers = mouseEventModifiersFromQtKeyboardModifiers(event->modifiers());
}

PlatformMouseEvent convertMouseEvent(QInputEvent* event, int clickCount)
{
    return WebKitPlatformMouseEvent(event, clickCount);
}

class WebKitPlatformWheelEvent : public PlatformWheelEvent {
public:
    WebKitPlatformWheelEvent(QWheelEvent*, int wheelScrollLines);

private:
    void applyDelta(QPoint delta, int wheelScrollLines);
};

void WebKitPlatformWheelEvent::applyDelta(QPoint angleDelta, int wheelScrollLines)
{
    m_deltaX = angleDelta.x();
    m_deltaY = angleDelta.y();
    m_wheelTicksX = m_deltaX / 120.0f;
    m_wheelTicksY = m_deltaY / 120.0f;

    // Since we request the scroll delta by the pixel, convert the wheel delta to pixel delta using the standard scroll step.
    // Use the same single scroll step as QTextEdit (in QTextEditPrivate::init [h,v]bar->setSingleStep)
    static const float cDefaultQtScrollStep = 20.f;
    m_deltaX = m_wheelTicksX * wheelScrollLines * cDefaultQtScrollStep;
    m_deltaY = m_wheelTicksY * wheelScrollLines * cDefaultQtScrollStep;
}

WebKitPlatformWheelEvent::WebKitPlatformWheelEvent(QWheelEvent* e, int wheelScrollLines)
{
    m_timestamp = WallTime::now();
    m_modifiers = mouseEventModifiersFromQtKeyboardModifiers(e->modifiers());
    m_position = e->position().toPoint();
    m_globalPosition = e->globalPosition().toPoint();
    m_granularity = ScrollByPixelWheelEvent;
    m_directionInvertedFromDevice = false;
    applyDelta(e->angleDelta(), wheelScrollLines);
}

#if ENABLE(TOUCH_EVENTS)
class WebKitPlatformTouchEvent : public PlatformTouchEvent {
public:
    WebKitPlatformTouchEvent(QTouchEvent*);
};

class WebKitPlatformTouchPoint : public PlatformTouchPoint {
public:
    WebKitPlatformTouchPoint(const QTouchEvent::TouchPoint&, State);
};

WebKitPlatformTouchEvent::WebKitPlatformTouchEvent(QTouchEvent* event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
        m_type = PlatformEvent::Type::TouchStart;
        break;
    case QEvent::TouchUpdate:
        m_type = PlatformEvent::Type::TouchMove;
        break;
    case QEvent::TouchEnd:
        m_type = PlatformEvent::Type::TouchEnd;
        break;
    case QEvent::TouchCancel:
        m_type = PlatformEvent::Type::TouchCancel;
        break;
    }

    const QList<QTouchEvent::TouchPoint>& points = event->touchPoints();
    for (int i = 0; i < points.count(); ++i) {
        PlatformTouchPoint::State state = PlatformTouchPoint::TouchStateEnd;

        switch (points.at(i).state()) {
        case Qt::TouchPointReleased:
            state = PlatformTouchPoint::TouchReleased;
            break;
        case Qt::TouchPointMoved:
            state = PlatformTouchPoint::TouchMoved;
            break;
        case Qt::TouchPointPressed:
            state = PlatformTouchPoint::TouchPressed;
            break;
        case Qt::TouchPointStationary:
            state = PlatformTouchPoint::TouchStationary;
            break;
        }

        // Qt does not have a Qt::TouchPointCancelled point state, so if we receive a touch cancel event,
        // simply cancel all touch points here.
        if (m_type == PlatformEvent::Type::TouchCancel)
            state = PlatformTouchPoint::TouchCancelled;

        m_touchPoints.append(WebKitPlatformTouchPoint(points.at(i), state));
    }

    m_modifiers = mouseEventModifiersFromQtKeyboardModifiers(event->modifiers());

    m_timestamp = WallTime::now();
}

WebKitPlatformTouchPoint::WebKitPlatformTouchPoint(const QTouchEvent::TouchPoint& point, State state)
{
    // The QTouchEvent::TouchPoint API states that ids will be >= 0.
    m_id = point.id();
    m_state = state;
    m_screenPos = point.screenPos().toPoint();
    m_pos = point.pos().toPoint();
    // Qt reports touch point size as rectangles, but we will pretend it is an oval.
    QSizeF diameter = point.ellipseDiameters();
    if (diameter.isValid()) {
        m_radiusX = diameter.width() / 2;
        m_radiusY = diameter.height() / 2;
    } else {
        // http://www.w3.org/TR/2011/WD-touch-events-20110505: 1 if no value is known.
        m_radiusX = 1;
        m_radiusY = 1;
    }
    m_force = point.pressure();
    // FIXME: Support m_rotationAngle if QTouchEvent at some point supports it.
}
#endif

#if ENABLE(QT_GESTURE_EVENTS)
class WebKitPlatformGestureEvent : public PlatformGestureEvent {
public:
    WebKitPlatformGestureEvent(QGestureEventFacade*);
};

static inline PlatformEvent::Type toPlatformEventType(Qt::GestureType type)
{
    switch (type) {
    case Qt::TapGesture:
        return PlatformEvent::Type::GestureTap;
    case Qt::TapAndHoldGesture:
        return PlatformEvent::Type::GestureLongPress;
    default:
        ASSERT_NOT_REACHED();
        return PlatformEvent::Type::NoType;
    }
}

WebKitPlatformGestureEvent::WebKitPlatformGestureEvent(QGestureEventFacade* event)
{
    m_type = toPlatformEventType(event->type);
    m_globalPosition = event->globalPos;
    m_position = event->pos;
    m_timestamp = WallTime::now();
}

#endif

PlatformWheelEvent convertWheelEvent(QWheelEvent* event, int wheelScrollLines)
{
    return WebKitPlatformWheelEvent(event, wheelScrollLines);
}

#if ENABLE(TOUCH_EVENTS)
PlatformTouchEvent convertTouchEvent(QTouchEvent* event)
{
    return WebKitPlatformTouchEvent(event);
}
#endif

#if ENABLE(QT_GESTURE_EVENTS)
PlatformGestureEvent convertGesture(QGestureEventFacade* event)
{
    return WebKitPlatformGestureEvent(event);
}
#endif
}
