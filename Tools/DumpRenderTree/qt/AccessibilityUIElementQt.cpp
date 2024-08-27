/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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
#include "AccessibilityUIElement.h"
#include <WebCore/NotImplemented.h>

AccessibilityUIElement::AccessibilityUIElement(void*)
{
    notImplemented();
}

AccessibilityUIElement AccessibilityUIElement::rowAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

// fixing these errors:
// AccessibilityUIElement::lineForIndex(int), referenced from:
  //     lineForIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::allAttributes(), referenced from:
  //     allAttributesCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::childrenCount(), referenced from:
  //     getChildrenCountCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::parentElement(), referenced from:
  //     parentElementCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::rowIndexRange(), referenced from:
  //     rowIndexRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::takeSelection(), referenced from:
  //     takeSelectionCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::boundsForRange(unsigned int, unsigned int), referenced from:
  //     boundsForRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::disclosedByRow(), referenced from:
  //     disclosedByRowCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::elementAtPoint(int, int), referenced from:
  //     elementAtPointCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::stringForRange(unsigned int, unsigned int), referenced from:
  //     stringForRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::titleUIElement(), referenced from:
  //     titleUIElementCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::getChildAtIndex(unsigned int), referenced from:
  //     childAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::removeSelection(), referenced from:
  //     removeSelectionCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::roleDescription(), referenced from:
  //     getRoleDescriptionCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfRows(), referenced from:
  //     attributesOfRowsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::columnIndexRange(), referenced from:
  //     columnIndexRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::valueDescription(), referenced from:
  //     getValueDescriptionCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::selectedTextRange(), referenced from:
  //     getSelectedTextRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfHeader(), referenced from:
  //     attributesOfHeaderCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::boolAttributeValue(OpaqueJSString*), referenced from:
  //     boolAttributeValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::computedRoleString(), referenced from:
  //     getComputedRoleStringCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::selectedRowAtIndex(unsigned int), referenced from:
  //     selectedRowAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfColumns(), referenced from:
  //     attributesOfColumnsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::cellForColumnAndRow(unsigned int, unsigned int), referenced from:
  //     cellForColumnAndRowCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::disclosedRowAtIndex(unsigned int), referenced from:
  //     disclosedRowAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isAttributeSettable(OpaqueJSString*), referenced from:
  //     isAttributeSettableCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::scrollToGlobalPoint(int, int), referenced from:
  //     scrollToGlobalPointCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::scrollToMakeVisible(), referenced from:
  //     scrollToMakeVisibleCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::x(), referenced from:
  //     getXCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::y(), referenced from:
  //     getYCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfChildren(), referenced from:
  //     attributesOfChildrenCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isAttributeSupported(OpaqueJSString*), referenced from:
  //     isAttributeSupportedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::numberAttributeValue(OpaqueJSString*), referenced from:
  //     numberAttributeValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::setSelectedTextRange(unsigned int, unsigned int), referenced from:
  //     setSelectedTextRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::stringAttributeValue(OpaqueJSString*), referenced from:
  //     stringAttributeValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::ariaOwnsElementAtIndex(unsigned int), referenced from:
  //     ariaOwnsElementAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfRowHeaders(), referenced from:
  //     attributesOfRowHeadersCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isPressActionSupported(), referenced from:
  //     isPressActionSupportedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::linkedUIElementAtIndex(unsigned int), referenced from:
  //     linkedUIElementAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::selectTextWithCriteria(OpaqueJSContext const*, OpaqueJSString*, OpaqueJSValue const*, OpaqueJSString*, OpaqueJSString*), referenced from:
  //     selectTextWithCriteriaCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::addNotificationListener(OpaqueJSValue*), referenced from:
  //     addNotificationListenerCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::ariaFlowToElementAtIndex(unsigned int), referenced from:
  //     ariaFlowToElementAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributedStringForRange(unsigned int, unsigned int), referenced from:
  //     attributedStringForRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfVisibleCells(), referenced from:
  //     attributesOfVisibleCellsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::insertionPointLineNumber(), referenced from:
  //     getInsertionPointLineNumberCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfColumnHeaders(), referenced from:
  //     attributesOfColumnHeadersCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfDocumentLinks(), referenced from:
  //     attributesOfDocumentLinksCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::ariaControlsElementAtIndex(unsigned int), referenced from:
  //     ariaControlsElementAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isDecrementActionSupported(), referenced from:
  //     isDecrementActionSupportedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isIncrementActionSupported(), referenced from:
  //     isIncrementActionSupportedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::removeNotificationListener(), referenced from:
  //     removeNotificationListenerCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::parameterizedAttributeNames(), referenced from:
  //     parameterizedAttributeNamesCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::uiElementForSearchPredicate(OpaqueJSContext const*, AccessibilityUIElement*, bool, OpaqueJSValue const*, OpaqueJSString*, bool, bool), referenced from:
  //     uiElementForSearchPredicateCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributesOfLinkedUIElements(), referenced from:
  //     attributesOfLinkedUIElementsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::scrollToMakeVisibleWithSubFocus(int, int, int, int), referenced from:
  //     scrollToMakeVisibleWithSubFocusCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::uiElementCountForSearchPredicate(OpaqueJSContext const*, AccessibilityUIElement*, bool, OpaqueJSValue const*, OpaqueJSString*, bool, bool), referenced from:
  //     uiElementCountForSearchPredicateCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::attributedStringRangeIsMisspelled(unsigned int, unsigned int), referenced from:
  //     attributedStringRangeIsMisspelledCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::url(), referenced from:
  //     getURLCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::role(), referenced from:
  //     getRoleCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::press(), referenced from:
  //     pressCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::title(), referenced from:
  //     getTitleCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::width(), referenced from:
  //     getWidthCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::height(), referenced from:
  //     getHeightCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::dismiss(), referenced from:
  //     dismissCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::subrole(), referenced from:
  //     getSubroleCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::language(), referenced from:
  //     getLanguageCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::maxValue(), referenced from:
  //     getMaxValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::minValue(), referenced from:
  //     getMinValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::rowCount(), referenced from:
  //     rowCountCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::showMenu(), referenced from:
  //     showMenuCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::decrement(), referenced from:
  //     decrementCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::increment(), referenced from:
  //     incrementCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isEnabled(), referenced from:
  //     getIsEnabledCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::takeFocus(), referenced from:
  //     takeFocusCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::AccessibilityUIElement(void*), referenced from:
  //     AccessibilityUIElement::horizontalScrollbar() const in AccessibilityUIElement.cpp.o
  //     AccessibilityUIElement::verticalScrollbar() const in AccessibilityUIElement.cpp.o
  //     AccessibilityUIElement::uiElementAttributeValue(OpaqueJSString*) const in AccessibilityUIElement.cpp.o
  //     AccessibilityUIElement::accessibilityElementForTextMarker(AccessibilityTextMarker*) in AccessibilityUIElement.cpp.o
  // AccessibilityController::rootElement(), referenced from:
  //     getRootElementCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::elementAtPoint(int, int), referenced from:
  //     getElementAtPointCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::focusedElement(), referenced from:
  //     getFocusedElementCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::setLogFocusEvents(bool), referenced from:
  //     logFocusEventsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  //     AccessibilityController::resetToConsistentState() in AccessibilityController.cpp.o
  // AccessibilityController::accessibleElementById(OpaqueJSString*), referenced from:
  //     getAccessibleElementByIdCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::addNotificationListener(OpaqueJSValue*), referenced from:
  //     addNotificationListenerCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::setLogValueChangeEvents(bool), referenced from:
  //     logValueChangeEventsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  //     AccessibilityController::resetToConsistentState() in AccessibilityController.cpp.o
  // AccessibilityController::setLogAccessibilityEvents(bool), referenced from:
  //     logAccessibilityEventsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  //     AccessibilityController::resetToConsistentState() in AccessibilityController.cpp.o
  // AccessibilityController::removeNotificationListener(), referenced from:
  //     removeNotificationListenerCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::setLogScrollingStartEvents(bool), referenced from:
  //     logScrollingStartEventsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  //     AccessibilityController::resetToConsistentState() in AccessibilityController.cpp.o
  // AccessibilityController::enableEnhancedAccessibility(bool), referenced from:
  //     enableEnhancedAccessibilityCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::enhancedAccessibilityEnabled(), referenced from:
  //     getEnhancedAccessibilityEnabledCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityController.cpp.o
  // AccessibilityController::platformResetToConsistentState(), referenced from:
  //     AccessibilityController::resetToConsistentState() in AccessibilityController.cpp.o
  // AccessibilityUIElement::isExpanded() const, referenced from:
  //     getIsExpandedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isRequired() const, referenced from:
  //     getIsRequiredCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isSelected() const, referenced from:
  //     getIsSelectedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::popupValue() const, referenced from:
  //     getPopupValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::rowHeaders(WTF::Vector<AccessibilityUIElement, 0ul, WTF::CrashOnOverflow, 16ul, WTF::FastMalloc>&) const, referenced from:
  //     rowHeadersCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isCollapsed() const, referenced from:
  //     getIsCollapsedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isFocusable() const, referenced from:
  //     getIsFocusableCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isOffScreen() const, referenced from:
  //     getIsOffScreenCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::orientation() const, referenced from:
  //     getOrientationCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isSelectable() const, referenced from:
  //     getIsSelectableCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::ariaIsGrabbed() const, referenced from:
  //     getARIAIsGrabbedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::columnHeaders(WTF::Vector<AccessibilityUIElement, 0ul, WTF::CrashOnOverflow, 16ul, WTF::FastMalloc>&) const, referenced from:
  //     columnHeadersCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::customContent() const, referenced from:
  //     getCustomContentCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::domIdentifier() const, referenced from:
  //     domIdentifierCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::selectedCells(OpaqueJSContext const*) const, referenced from:
  //     selectedCellsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::ariaDropEffects() const, referenced from:
  //     getARIADropEffectsCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isIndeterminate() const, referenced from:
  //     getIsIndeterminate(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::liveRegionStatus() const, referenced from:
  //     getLiveRegionStatusCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::hierarchicalLevel() const, referenced from:
  //     hierarchicalLevelCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isMultiSelectable() const, referenced from:
  //     getIsMultiSelectableCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::accessibilityValue() const, referenced from:
  //     getAccessibilityValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isAtomicLiveRegion() const, referenced from:
  //     getIsAtomicLiveRegionCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::liveRegionRelevant() const, referenced from:
  //     getLiveRegionRelevantCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::selectedChildAtIndex(unsigned int) const, referenced from:
  //     selectedChildAtIndexCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSValue*, unsigned long, OpaqueJSValue const* const*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::textInputMarkedRange() const, referenced from:
  //     getTextInputMarkedRangeCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::selectedChildrenCount() const, referenced from:
  //     selectedChildrenCountCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isSelectedOptionActive() const, referenced from:
  //     getIsSelectedOptionActiveCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::hasDocumentRoleAncestor() const, referenced from:
  //     hasDocumentRoleAncestorCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isInDescriptionListTerm() const, referenced from:
  //     isInDescriptionListTermCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::hasWebApplicationAncestor() const, referenced from:
  //     hasWebApplicationAncestorCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isInDescriptionListDetail() const, referenced from:
  //     isInDescriptionListDetailCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isBusy() const, referenced from:
  //     getIsBusyCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::hasPopup() const, referenced from:
  //     getHasPopupCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::helpText() const, referenced from:
  //     getHelpTextCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::intValue() const, referenced from:
  //     getIntValueCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isInCell() const, referenced from:
  //     isInCellCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::classList() const, referenced from:
  //     getClassListCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isChecked() const, referenced from:
  //     getIsCheckedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isFocused() const, referenced from:
  //     getIsFocusedCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isIgnored() const, referenced from:
  //     isIgnoredCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o
  // AccessibilityUIElement::isVisible() const, referenced from:
  //     getIsVisibleCallback(OpaqueJSContext const*, OpaqueJSValue*, OpaqueJSString*, OpaqueJSValue const**) in AccessibilityUIElement.cpp.o


double AccessibilityUIElement::clickPointX()
{
    notImplemented();
    return 0.0f;
}

double AccessibilityUIElement::clickPointY()
{
    notImplemented();
    return 0.0f;
}

int AccessibilityUIElement::columnCount()
{
    notImplemented();
    return 0;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::description()
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::stringValue()
{
    notImplemented();
    return nullptr;
}

void AccessibilityUIElement::addSelection()
{
    notImplemented();
}

int AccessibilityUIElement::indexInTable()
{
    notImplemented();
    return 0;
}

unsigned AccessibilityUIElement::indexOfChild(AccessibilityUIElement*)
{
    notImplemented();
    return 0;
}

int AccessibilityUIElement::lineForIndex(int)
{
    notImplemented();
    return 0;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::allAttributes()
{
    notImplemented();
    return nullptr;
}

int AccessibilityUIElement::childrenCount()
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::parentElement()
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::rowIndexRange()
{
    notImplemented();
    return nullptr;
}

void AccessibilityUIElement::takeSelection()
{
    notImplemented();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::boundsForRange(unsigned, unsigned)
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityUIElement::disclosedByRow()
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityUIElement::elementAtPoint(int, int)
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::stringForRange(unsigned, unsigned)
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityUIElement::titleUIElement()
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityUIElement::getChildAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

void AccessibilityUIElement::removeSelection()
{
    notImplemented();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::roleDescription()
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfRows()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::columnIndexRange()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::valueDescription()
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::selectedTextRange()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfHeader()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

bool AccessibilityUIElement::boolAttributeValue(JSStringRef)
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::computedRoleString()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

AccessibilityUIElement AccessibilityUIElement::selectedRowAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfColumns()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

AccessibilityUIElement AccessibilityUIElement::cellForColumnAndRow(unsigned, unsigned)
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityUIElement::disclosedRowAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

bool AccessibilityUIElement::isAttributeSettable(JSStringRef)
{
    notImplemented();
    return false;
}

void AccessibilityUIElement::scrollToGlobalPoint(int, int)
{
    notImplemented();
}

void AccessibilityUIElement::scrollToMakeVisible()
{
    notImplemented();
}

double AccessibilityUIElement::x()
{
    notImplemented();
    return 0.0f;
}

double AccessibilityUIElement::y()
{
    notImplemented();
    return 0.0f;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfChildren()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

bool AccessibilityUIElement::isAttributeSupported(JSStringRef)
{
    notImplemented();
    return false;
}

double AccessibilityUIElement::numberAttributeValue(JSStringRef)
{
    notImplemented();
    return 0.0f;
}

void AccessibilityUIElement::setSelectedTextRange(unsigned, unsigned)
{
    notImplemented();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::stringAttributeValue(JSStringRef)
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityUIElement::ariaOwnsElementAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfRowHeaders()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

bool AccessibilityUIElement::isPressActionSupported()
{
    notImplemented();
    return false;
}

AccessibilityUIElement AccessibilityUIElement::linkedUIElementAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::selectTextWithCriteria(JSContextRef, JSStringRef, JSValueRef, JSStringRef, JSStringRef)
{
    notImplemented();
    return nullptr;
}

bool AccessibilityUIElement::addNotificationListener(JSObjectRef)
{
    notImplemented();
    return false;
}

AccessibilityUIElement AccessibilityUIElement::ariaFlowToElementAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributedStringForRange(unsigned, unsigned)
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfVisibleCells()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

int AccessibilityUIElement::insertionPointLineNumber()
{
    notImplemented();
    return 0;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfColumnHeaders()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfDocumentLinks()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

AccessibilityUIElement AccessibilityUIElement::ariaControlsElementAtIndex(unsigned)
{
    notImplemented();
    return nullptr;
}

bool AccessibilityUIElement::isDecrementActionSupported()
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isIncrementActionSupported()
{
    notImplemented();
    return false;
}

void AccessibilityUIElement::removeNotificationListener()
{
    notImplemented();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::parameterizedAttributeNames()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

AccessibilityUIElement AccessibilityUIElement::uiElementForSearchPredicate(JSContextRef, AccessibilityUIElement*, bool, JSValueRef, JSStringRef, bool, bool)
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::attributesOfLinkedUIElements()
{
    notImplemented();
    return JSRetainPtr<JSStringRef>();
}

void AccessibilityUIElement::scrollToMakeVisibleWithSubFocus(int, int, int, int)
{
    notImplemented();
}

unsigned AccessibilityUIElement::uiElementCountForSearchPredicate(JSContextRef, AccessibilityUIElement*, bool, JSValueRef, JSStringRef, bool, bool)
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::attributedStringRangeIsMisspelled(unsigned, unsigned)
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::url()
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::role()
{
    notImplemented();
    return nullptr;
}

void AccessibilityUIElement::press()
{
    notImplemented();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::title()
{
    notImplemented();
    return nullptr;
}

double AccessibilityUIElement::width()
{
    notImplemented();
    return 0.0f;
}

double AccessibilityUIElement::height()
{
    notImplemented();
    return 0.0f;
}

void AccessibilityUIElement::dismiss()
{
    notImplemented();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::subrole()
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::language()
{
    notImplemented();
    return nullptr;
}

double AccessibilityUIElement::maxValue()
{
    notImplemented();
    return 0.0f;
}

double AccessibilityUIElement::minValue()
{
    notImplemented();
    return 0.0f;
}

int AccessibilityUIElement::rowCount()
{
    notImplemented();
    return 0;
}

void AccessibilityUIElement::showMenu()
{
    notImplemented();
}

void AccessibilityUIElement::decrement()
{
    notImplemented();
}

void AccessibilityUIElement::increment()
{
    notImplemented();
}

bool AccessibilityUIElement::isEnabled()
{
    notImplemented();
    return false;
}

void AccessibilityUIElement::takeFocus()
{
    notImplemented();
}

bool AccessibilityUIElement::isExpanded() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isRequired() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isSelected() const
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::popupValue() const
{
    notImplemented();
    return nullptr;
}

void AccessibilityUIElement::rowHeaders(Vector<AccessibilityUIElement>&) const
{
    notImplemented();
}

bool AccessibilityUIElement::isCollapsed() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isFocusable() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isOffScreen() const
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::orientation() const
{
    notImplemented();
    return nullptr;
}

bool AccessibilityUIElement::isSelectable() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::ariaIsGrabbed() const
{
    notImplemented();
    return false;
}

void AccessibilityUIElement::columnHeaders(Vector<AccessibilityUIElement>&) const
{
    notImplemented();
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::customContent() const
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::domIdentifier() const
{
    notImplemented();
    return nullptr;
}

JSValueRef AccessibilityUIElement::selectedCells(JSContextRef) const
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::ariaDropEffects() const
{
    notImplemented();
    return nullptr;
}

bool AccessibilityUIElement::isIndeterminate() const
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::liveRegionStatus() const
{
    notImplemented();
    return nullptr;
}

int AccessibilityUIElement::hierarchicalLevel() const
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isMultiSelectable() const
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::accessibilityValue() const
{
    notImplemented();
    return nullptr;
}

bool AccessibilityUIElement::isAtomicLiveRegion() const
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::liveRegionRelevant() const
{
    notImplemented();
    return nullptr;
}

AccessibilityUIElement AccessibilityUIElement::selectedChildAtIndex(unsigned) const
{
    notImplemented();
    return nullptr;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::textInputMarkedRange() const
{
    notImplemented();
    return nullptr;
}

unsigned AccessibilityUIElement::selectedChildrenCount() const
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isSelectedOptionActive() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::hasDocumentRoleAncestor() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isInDescriptionListTerm() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::hasWebApplicationAncestor() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isInDescriptionListDetail() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isBusy() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::hasPopup() const
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::helpText() const
{
    notImplemented();
    return nullptr;
}

double AccessibilityUIElement::intValue() const
{
    notImplemented();
    return 0.0f;
}

bool AccessibilityUIElement::isInCell() const
{
    notImplemented();
    return false;
}

JSRetainPtr<JSStringRef> AccessibilityUIElement::classList() const
{
    notImplemented();
    return nullptr;
}

bool AccessibilityUIElement::isChecked() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isFocused() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isIgnored() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isVisible() const
{
    notImplemented();
    return false;
}
