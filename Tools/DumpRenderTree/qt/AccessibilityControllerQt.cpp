/*
 * Copyright (C) 2008, 2009, 2010 Apple Inc. All Rights Reserved.
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

#include "config.h"
#include "DumpRenderTree.h"
#include "AccessibilityController.h"
#include <WebCore/NotImplemented.h>

AccessibilityUIElement AccessibilityController::elementAtPoint(int x, int y)
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityController::focusedElement()
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityController::rootElement()
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityController::accessibleElementById(JSStringRef idAttributeRef)
{
    notImplemented();
    return nullptr;
}

void AccessibilityController::setLogFocusEvents(bool)
{
}

void AccessibilityController::setLogScrollingStartEvents(bool)
{
}

void AccessibilityController::setLogValueChangeEvents(bool)
{
}

void AccessibilityController::setLogAccessibilityEvents(bool)
{
}

void AccessibilityController::platformResetToConsistentState()
{
    notImplemented();
}

bool AccessibilityController::addNotificationListener(JSObjectRef functionCallback)
{
    notImplemented();
    return true;
}

void AccessibilityController::removeNotificationListener()
{
}

void AccessibilityController::enableEnhancedAccessibility(bool enable)
{
    notImplemented();
}

bool AccessibilityController::enhancedAccessibilityEnabled()
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityController::platformName() const
{
    notImplemented();
    return nullptr;
}
