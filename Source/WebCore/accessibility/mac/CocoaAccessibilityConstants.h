/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

//
// Attributes
//

#define AXEndTextMarkerAttribute @"AXEndTextMarker"
#define AXHasDocumentRoleAncestorAttribute @"AXHasDocumentRoleAncestor"
#define AXHasWebApplicationAncestorAttribute @"AXHasWebApplicationAncestor"
#define AXSelectedTextMarkerRangeAttribute @"AXSelectedTextMarkerRange"
#define AXStartTextMarkerAttribute @"AXStartTextMarker"
#define NSAccessibilityARIAAtomicAttribute @"AXARIAAtomic"
#define NSAccessibilityARIAColumnCountAttribute @"AXARIAColumnCount"
#define NSAccessibilityARIAColumnIndexAttribute @"AXARIAColumnIndex"
#define NSAccessibilityARIACurrentAttribute @"AXARIACurrent"
#define NSAccessibilityARIALiveAttribute @"AXARIALive"
#define NSAccessibilityARIAPosInSetAttribute @"AXARIAPosInSet"
#define NSAccessibilityARIARelevantAttribute @"AXARIARelevant"
#define NSAccessibilityARIARowCountAttribute @"AXARIARowCount"
#define NSAccessibilityARIARowIndexAttribute @"AXARIARowIndex"
#define NSAccessibilityARIASetSizeAttribute @"AXARIASetSize"
#define NSAccessibilityAccessKeyAttribute @"AXAccessKey"
#define NSAccessibilityActiveElementAttribute @"AXActiveElement"
#define NSAccessibilityBlockQuoteLevelAttribute @"AXBlockQuoteLevel"
#define NSAccessibilityBrailleLabelAttribute @"AXBrailleLabel"
#define NSAccessibilityBrailleRoleDescriptionAttribute @"AXBrailleRoleDescription"
#define NSAccessibilityCaretBrowsingEnabledAttribute @"AXCaretBrowsingEnabled"
#define NSAccessibilityChildrenInNavigationOrderAttribute @"AXChildrenInNavigationOrder"
#define NSAccessibilityDOMClassListAttribute @"AXDOMClassList"
#define NSAccessibilityDOMIdentifierAttribute @"AXDOMIdentifier"
#define NSAccessibilityDatetimeValueAttribute @"AXDateTimeValue"
#define NSAccessibilityDropEffectsAttribute @"AXDropEffects"
#define NSAccessibilityEditableAncestorAttribute @"AXEditableAncestor"
#define NSAccessibilityElementBusyAttribute @"AXElementBusy"
#define NSAccessibilityEmbeddedImageDescriptionAttribute @"AXEmbeddedImageDescription"
#define NSAccessibilityExpandedTextValueAttribute @"AXExpandedTextValue"
#define NSAccessibilityFocusableAncestorAttribute @"AXFocusableAncestor"
#define NSAccessibilityGrabbedAttribute @"AXGrabbed"
#define NSAccessibilityHasPopupAttribute @"AXHasPopup"
#define NSAccessibilityHighestEditableAncestorAttribute @"AXHighestEditableAncestor"
#define NSAccessibilityImageOverlayElementsAttribute @"AXImageOverlayElements"
#define NSAccessibilityInlineTextAttribute @"AXInlineText"
#define NSAccessibilityInvalidAttribute @"AXInvalid"
#define NSAccessibilityKeyShortcutsAttribute @"AXKeyShortcutsValue"
#define NSAccessibilityLanguageAttribute @"AXLanguage"
#define NSAccessibilityLinkRelationshipTypeAttribute @"AXLinkRelationshipType"
#define NSAccessibilityLoadingProgressAttribute @"AXLoadingProgress"
#define NSAccessibilityOwnsAttribute @"AXOwns"
#define NSAccessibilityPathAttribute @"AXPath"
#define NSAccessibilityPopupValueAttribute @"AXPopupValue"
#define NSAccessibilityPreventKeyboardDOMEventDispatchAttribute @"AXPreventKeyboardDOMEventDispatch"
#define NSAccessibilityPrimaryScreenHeightAttribute @"_AXPrimaryScreenHeight"
#define NSAccessibilityRelativeFrameAttribute @"AXRelativeFrame"
#define NSAccessibilitySelectedCellsAttribute @"AXSelectedCells"
#define NSAccessibilityTextCompletionAttribute @"AXTextCompletion"
#define NSAccessibilityTextInputMarkedRangeAttribute @"AXTextInputMarkedRange"
#define NSAccessibilityTextInputMarkedTextMarkerRangeAttribute @"AXTextInputMarkedTextMarkerRange"
#define NSAccessibilityValueAutofillAvailableAttribute @"AXValueAutofillAvailable"
#define NSAccessibilityValueAutofillTypeAttribute @"AXValueAutofillType"

//
// Parameterized Attributes
//

#define AXAttributedStringForTextMarkerRangeAttribute @"AXAttributedStringForTextMarkerRange"
#define AXAttributedStringForTextMarkerRangeWithOptionsAttribute @"AXAttributedStringForTextMarkerRangeWithOptions"
#define AXBoundsForTextMarkerRangeAttribute @"AXBoundsForTextMarkerRange"
#define AXEndTextMarkerForBoundsAttribute @"AXEndTextMarkerForBounds"
#define AXIndexForTextMarkerAttribute @"AXIndexForTextMarker"
#define AXLeftLineTextMarkerRangeForTextMarkerAttribute @"AXLeftLineTextMarkerRangeForTextMarker"
#define AXLeftWordTextMarkerRangeForTextMarkerAttribute @"AXLeftWordTextMarkerRangeForTextMarker"
#define AXLengthForTextMarkerRangeAttribute @"AXLengthForTextMarkerRange"
#define AXLineForTextMarkerAttribute @"AXLineForTextMarker"
#define AXLineTextMarkerRangeForTextMarkerAttribute @"AXLineTextMarkerRangeForTextMarker"
#define AXMisspellingTextMarkerRangeAttribute @"AXMisspellingTextMarkerRange"
#define AXNextLineEndTextMarkerForTextMarkerAttribute @"AXNextLineEndTextMarkerForTextMarker"
#define AXNextParagraphEndTextMarkerForTextMarkerAttribute @"AXNextParagraphEndTextMarkerForTextMarker"
#define AXNextSentenceEndTextMarkerForTextMarkerAttribute @"AXNextSentenceEndTextMarkerForTextMarker"
#define AXNextTextMarkerForTextMarkerAttribute @"AXNextTextMarkerForTextMarker"
#define AXNextWordEndTextMarkerForTextMarkerAttribute @"AXNextWordEndTextMarkerForTextMarker"
#define AXParagraphTextMarkerRangeForTextMarkerAttribute @"AXParagraphTextMarkerRangeForTextMarker"
#define AXPreviousLineStartTextMarkerForTextMarkerAttribute @"AXPreviousLineStartTextMarkerForTextMarker"
#define AXPreviousParagraphStartTextMarkerForTextMarkerAttribute @"AXPreviousParagraphStartTextMarkerForTextMarker"
#define AXPreviousSentenceStartTextMarkerForTextMarkerAttribute @"AXPreviousSentenceStartTextMarkerForTextMarker"
#define AXPreviousTextMarkerForTextMarkerAttribute @"AXPreviousTextMarkerForTextMarker"
#define AXPreviousWordStartTextMarkerForTextMarkerAttribute @"AXPreviousWordStartTextMarkerForTextMarker"
#define AXRightLineTextMarkerRangeForTextMarkerAttribute @"AXRightLineTextMarkerRangeForTextMarker"
#define AXRightWordTextMarkerRangeForTextMarkerAttribute @"AXRightWordTextMarkerRangeForTextMarker"
#define AXSentenceTextMarkerRangeForTextMarkerAttribute @"AXSentenceTextMarkerRangeForTextMarker"
#define AXStartTextMarkerForBoundsAttribute @"AXStartTextMarkerForBounds"
#define AXStringForTextMarkerRangeAttribute @"AXStringForTextMarkerRange"
#define AXStyleTextMarkerRangeForTextMarkerAttribute @"AXStyleTextMarkerRangeForTextMarker"
#define AXTextMarkerForIndexAttribute @"AXTextMarkerForIndex"
#define AXTextMarkerForPositionAttribute @"AXTextMarkerForPosition" // FIXME: should be AXTextMarkerForPoint.
#define AXTextMarkerIsValidAttribute @"AXTextMarkerIsValid"
#define AXTextMarkerRangeForLineAttribute @"AXTextMarkerRangeForLine"
#define AXTextMarkerRangeForTextMarkersAttribute @"AXTextMarkerRangeForTextMarkers"
#define AXTextMarkerRangeForUIElementAttribute @"AXTextMarkerRangeForUIElement"
#define AXTextMarkerRangeForUnorderedTextMarkersAttribute @"AXTextMarkerRangeForUnorderedTextMarkers"
#define AXUIElementForTextMarkerAttribute @"AXUIElementForTextMarker"
#define NSAccessibilityUIElementsForSearchPredicateParameterizedAttribute @"AXUIElementsForSearchPredicate"

#define kAXConvertRelativeFrameParameterizedAttribute @"AXConvertRelativeFrame"

//
// Actions
//

#define NSAccessibilityScrollToVisibleAction @"AXScrollToVisible"

//
// Attributed string attribute names
//

#define AXDidSpellCheckAttribute @"AXDidSpellCheck"

//
// Notifications
//

#define NSAccessibilityCurrentStateChangedNotification @"AXCurrentStateChanged"
#define NSAccessibilityLiveRegionChangedNotification @"AXLiveRegionChanged"
#define NSAccessibilityLiveRegionCreatedNotification @"AXLiveRegionCreated"
#define NSAccessibilityTextInputMarkingSessionBeganNotification @"AXTextInputMarkingSessionBegan"
#define NSAccessibilityTextInputMarkingSessionEndedNotification @"AXTextInputMarkingSessionEnded"

#define kAXDraggingDestinationDragAcceptedNotification CFSTR("AXDraggingDestinationDragAccepted")
#define kAXDraggingDestinationDragNotAcceptedNotification CFSTR("AXDraggingDestinationDragNotAccepted")
#define kAXDraggingDestinationDropAllowedNotification CFSTR("AXDraggingDestinationDropAllowed")
#define kAXDraggingDestinationDropNotAllowedNotification CFSTR("AXDraggingDestinationDropNotAllowed")
#define kAXDraggingSourceDragBeganNotification CFSTR("AXDraggingSourceDragBegan")
#define kAXDraggingSourceDragEndedNotification CFSTR("AXDraggingSourceDragEnded")

//
// Additional attributes in text change notifications
//

#define NSAccessibilityTextStateChangeTypeKey @"AXTextStateChangeType"
#define NSAccessibilityTextStateSyncKey @"AXTextStateSync"
#define NSAccessibilityTextSelectionDirection @"AXTextSelectionDirection"
#define NSAccessibilityTextSelectionGranularity @"AXTextSelectionGranularity"
#define NSAccessibilityTextSelectionChangedFocus @"AXTextSelectionChangedFocus"
#define NSAccessibilityTextEditType @"AXTextEditType"
#define NSAccessibilityTextChangeValues @"AXTextChangeValues"
#define NSAccessibilityTextChangeValue @"AXTextChangeValue"
#define NSAccessibilityTextChangeValueLength @"AXTextChangeValueLength"
#define NSAccessibilityTextChangeValueStartMarker @"AXTextChangeValueStartMarker"
#define NSAccessibilityTextChangeElement @"AXTextChangeElement"

//
// For use with NSAccessibilitySelectTextWithCriteriaParameterizedAttribute
//

#define NSAccessibilityIntersectionWithSelectionRangeAttribute @"AXIntersectionWithSelectionRange"
#define NSAccessibilitySelectTextActivity @"AXSelectTextActivity"
#define NSAccessibilitySelectTextActivityFindAndReplace @"AXSelectTextActivityFindAndReplace"
#define NSAccessibilitySelectTextActivityFindAndSelect @"AXSelectTextActivityFindAndSelect"
#define NSAccessibilitySelectTextAmbiguityResolution @"AXSelectTextAmbiguityResolution"
#define NSAccessibilitySelectTextAmbiguityResolutionClosestAfterSelection @"AXSelectTextAmbiguityResolutionClosestAfterSelection"
#define NSAccessibilitySelectTextAmbiguityResolutionClosestBeforeSelection @"AXSelectTextAmbiguityResolutionClosestBeforeSelection"
#define NSAccessibilitySelectTextAmbiguityResolutionClosestToSelection @"AXSelectTextAmbiguityResolutionClosestToSelection"
#define NSAccessibilitySelectTextReplacementString @"AXSelectTextReplacementString"
#define NSAccessibilitySelectTextSearchStrings @"AXSelectTextSearchStrings"
#define NSAccessibilitySelectTextWithCriteriaParameterizedAttribute @"AXSelectTextWithCriteria"

#define kAXSelectTextActivityFindAndCapitalize @"AXSelectTextActivityFindAndCapitalize"
#define kAXSelectTextActivityFindAndLowercase @"AXSelectTextActivityFindAndLowercase"
#define kAXSelectTextActivityFindAndUppercase @"AXSelectTextActivityFindAndUppercase"

//
// Text search
//

/* Performs a text search with the given parameters.
 Returns an NSArray of text marker ranges of the search hits.
 */
#define NSAccessibilitySearchTextWithCriteriaParameterizedAttribute @"AXSearchTextWithCriteria"

// NSArray of strings to search for.
#define NSAccessibilitySearchTextSearchStrings @"AXSearchTextSearchStrings"

// NSString specifying the start point of the search: selection, begin or end.
#define NSAccessibilitySearchTextStartFrom @"AXSearchTextStartFrom"

// Values for SearchTextStartFrom.
#define NSAccessibilitySearchTextStartFromBegin @"AXSearchTextStartFromBegin"
#define NSAccessibilitySearchTextStartFromSelection @"AXSearchTextStartFromSelection"
#define NSAccessibilitySearchTextStartFromEnd @"AXSearchTextStartFromEnd"

// NSString specifying the direction of the search: forward, backward, closest, all.
#define NSAccessibilitySearchTextDirection @"AXSearchTextDirection"

// Values for SearchTextDirection.
#define NSAccessibilitySearchTextDirectionForward @"AXSearchTextDirectionForward"
#define NSAccessibilitySearchTextDirectionBackward @"AXSearchTextDirectionBackward"
#define NSAccessibilitySearchTextDirectionClosest @"AXSearchTextDirectionClosest"
#define NSAccessibilitySearchTextDirectionAll @"AXSearchTextDirectionAll"

//
// Text operations
//

// Performs an operation on the given text.
#define NSAccessibilityTextOperationParameterizedAttribute @"AXTextOperation"

// Text on which to perform operation.
#define NSAccessibilityTextOperationMarkerRanges @"AXTextOperationMarkerRanges"

// The type of operation to be performed: select, replace, capitalize....
#define NSAccessibilityTextOperationType @"AXTextOperationType"

// Values for TextOperationType.
#define NSAccessibilityTextOperationSelect @"TextOperationSelect"
#define NSAccessibilityTextOperationReplace @"TextOperationReplace"
#define NSAccessibilityTextOperationReplacePreserveCase @"TextOperationReplacePreserveCase"
#define NSAccessibilityTextOperationCapitalize @"Capitalize"
#define NSAccessibilityTextOperationLowercase @"Lowercase"
#define NSAccessibilityTextOperationUppercase @"Uppercase"

// Replacement text for operation replace.
#define NSAccessibilityTextOperationReplacementString @"AXTextOperationReplacementString"

// Array of replacement text for operation replace. The array should contain
// the same number of items as the number of text operation ranges.
#define NSAccessibilityTextOperationIndividualReplacementStrings @"AXTextOperationIndividualReplacementStrings"

// Boolean specifying whether a smart replacement should be performed.
#define NSAccessibilityTextOperationSmartReplace @"AXTextOperationSmartReplace"

//
// Math attributes
//

#define NSAccessibilityMathBaseAttribute @"AXMathBase"
#define NSAccessibilityMathFencedCloseAttribute @"AXMathFencedClose"
#define NSAccessibilityMathFencedOpenAttribute @"AXMathFencedOpen"
#define NSAccessibilityMathFractionDenominatorAttribute @"AXMathFractionDenominator"
#define NSAccessibilityMathFractionNumeratorAttribute @"AXMathFractionNumerator"
#define NSAccessibilityMathLineThicknessAttribute @"AXMathLineThickness"
#define NSAccessibilityMathOverAttribute @"AXMathOver"
#define NSAccessibilityMathPostscriptsAttribute @"AXMathPostscripts"
#define NSAccessibilityMathPrescriptsAttribute @"AXMathPrescripts"
#define NSAccessibilityMathRootIndexAttribute @"AXMathRootIndex"
#define NSAccessibilityMathRootRadicandAttribute @"AXMathRootRadicand"
#define NSAccessibilityMathSubscriptAttribute @"AXMathSubscript"
#define NSAccessibilityMathSuperscriptAttribute @"AXMathSuperscript"
#define NSAccessibilityMathUnderAttribute @"AXMathUnder"
