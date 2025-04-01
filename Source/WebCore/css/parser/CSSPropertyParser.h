/*
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004 - 2021 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 - 2010  Torch Mobile (Beijing) Co. Ltd. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#pragma once

#include "CSSCustomPropertySyntax.h"
#include "CSSParserContext.h"
#include "CSSParserTokenRange.h"
#include "CSSProperty.h"
#include "StyleRuleType.h"
#include <wtf/text/StringView.h>

namespace WebCore {

class CSSCustomPropertyValue;
class CSSProperty;
class StylePropertyShorthand;

namespace CSS {
struct PropertyParserState;
}

namespace Style {
class BuilderState;
}
    
// Inputs: PropertyID, isImportant bool, CSSParserTokenRange.
// Outputs: Vector of CSSProperties

class CSSPropertyParser {
    WTF_MAKE_NONCOPYABLE(CSSPropertyParser);
public:
    static bool parseValue(CSSPropertyID, IsImportant, const CSSParserTokenRange&, const CSSParserContext&, Vector<CSSProperty, 256>&, StyleRuleType);

    // Parses a longhand CSS property.
    static RefPtr<CSSValue> parseStylePropertyLonghand(CSSPropertyID, const String&, const CSSParserContext&);
    static RefPtr<CSSValue> parseStylePropertyLonghand(CSSPropertyID, const CSSParserTokenRange&, const CSSParserContext&);

    static RefPtr<CSSCustomPropertyValue> parseTypedCustomPropertyInitialValue(const AtomString&, const CSSCustomPropertySyntax&, CSSParserTokenRange, Style::BuilderState&, const CSSParserContext&);
    static RefPtr<CSSCustomPropertyValue> parseTypedCustomPropertyValue(const AtomString& name, const CSSCustomPropertySyntax&, const CSSParserTokenRange&, Style::BuilderState&, const CSSParserContext&);
    static ComputedStyleDependencies collectParsedCustomPropertyValueDependencies(const CSSCustomPropertySyntax&, const CSSParserTokenRange&, const CSSParserContext&);
    static bool isValidCustomPropertyValueForSyntax(const CSSCustomPropertySyntax&, CSSParserTokenRange, const CSSParserContext&);

    static RefPtr<CSSValue> parseCounterStyleDescriptor(CSSPropertyID, const String&, const CSSParserContext&);

private:
    CSSPropertyParser(const CSSParserTokenRange&, const CSSParserContext&, Vector<CSSProperty, 256>*, bool consumeWhitespace = true);

    // MARK: - Custom property parsing

    std::pair<RefPtr<CSSValue>, CSSCustomPropertySyntax::Type> consumeCustomPropertyValueWithSyntax(CSS::PropertyParserState&, const CSSCustomPropertySyntax&);
    RefPtr<CSSCustomPropertyValue> parseTypedCustomPropertyValue(CSS::PropertyParserState&, const AtomString& name, const CSSCustomPropertySyntax&, Style::BuilderState&);
    ComputedStyleDependencies collectParsedCustomPropertyValueDependencies(CSS::PropertyParserState&, const CSSCustomPropertySyntax&);

    // MARK: - Root parsing functions.

    // Style properties.
    bool parseStyleProperty(CSSPropertyID, IsImportant, StyleRuleType);
    RefPtr<CSSValue> parseStylePropertyLonghand(CSSPropertyID, CSS::PropertyParserState&);
    bool parseStylePropertyShorthand(CSSPropertyID, CSS::PropertyParserState&);

    // @font-face descriptors.
    bool parseFontFaceDescriptor(CSSPropertyID);

    // @font-palette-values descriptors.
    bool parseFontPaletteValuesDescriptor(CSSPropertyID);

    // @counter-style descriptors.
    bool parseCounterStyleDescriptor(CSSPropertyID);
    
    // @keyframe descriptors.
    bool parseKeyframeDescriptor(CSSPropertyID, IsImportant);

    // @page descriptors.
    bool parsePageDescriptor(CSSPropertyID, IsImportant);

    // @property descriptors.
    bool parsePropertyDescriptor(CSSPropertyID);

    // @view-transition descriptors.
    bool parseViewTransitionDescriptor(CSSPropertyID);

    // @position-try descriptors.
    bool parsePositionTryDescriptor(CSSPropertyID, IsImportant);

    // MARK: - Property Adding

    // Bottleneck where the CSSValue is added to the CSSProperty vector.
    void addProperty(CSSPropertyID longhand, CSSPropertyID shorthand, RefPtr<CSSValue>&&, IsImportant, IsImplicit = IsImplicit::No);

    // Utility functions to make adding properties more ergonomic.
    void addPropertyForCurrentShorthand(CSS::PropertyParserState&, CSSPropertyID longhand, RefPtr<CSSValue>&&, IsImplicit = IsImplicit::No);
    void addPropertyForAllLonghandsOfShorthand(CSSPropertyID shorthand, RefPtr<CSSValue>&&, IsImportant, IsImplicit = IsImplicit::No);
    void addPropertyForAllLonghandsOfCurrentShorthand(CSS::PropertyParserState&, RefPtr<CSSValue>&&, IsImplicit = IsImplicit::No);

    // MARK: - Shorthand Parsing

    bool consumeShorthandGreedily(const StylePropertyShorthand&, CSS::PropertyParserState&);
    bool consume2ValueShorthand(const StylePropertyShorthand&, CSS::PropertyParserState&);
    bool consume4ValueShorthand(const StylePropertyShorthand&, CSS::PropertyParserState&);

    bool consumeBorderShorthand(CSS::PropertyParserState&);
    bool consumeBorderInlineShorthand(CSS::PropertyParserState&);
    bool consumeBorderBlockShorthand(CSS::PropertyParserState&);

    bool consumeAnimationShorthand(const StylePropertyShorthand&, CSS::PropertyParserState&);

    bool consumeBackgroundShorthand(const StylePropertyShorthand&, CSS::PropertyParserState&);
    bool consumeBackgroundPositionShorthand(const StylePropertyShorthand&, CSS::PropertyParserState&);
    bool consumeWebkitBackgroundSizeShorthand(CSS::PropertyParserState&);

    bool consumeMaskShorthand(CSS::PropertyParserState&);
    bool consumeMaskPositionShorthand(CSS::PropertyParserState&);

    bool consumeOverflowShorthand(CSS::PropertyParserState&);

    bool consumeColumnsShorthand(CSS::PropertyParserState&);

    bool consumeGridItemPositionShorthand(const StylePropertyShorthand&, CSS::PropertyParserState&);
    bool consumeGridTemplateRowsAndAreasAndColumns(CSS::PropertyParserState&);
    bool consumeGridTemplateShorthand(CSS::PropertyParserState&);
    bool consumeGridShorthand(CSS::PropertyParserState&);
    bool consumeGridAreaShorthand(CSS::PropertyParserState&);

    bool consumeAlignShorthand(const StylePropertyShorthand&, CSS::PropertyParserState&);

    bool consumeBlockStepShorthand(CSS::PropertyParserState&);

    bool consumeFontShorthand(CSS::PropertyParserState&);
    bool consumeFontVariantShorthand(CSS::PropertyParserState&);
    bool consumeFontSynthesisShorthand(CSS::PropertyParserState&);

    bool consumeTextDecorationShorthand(CSS::PropertyParserState&);
    bool consumeTextDecorationSkipShorthand(CSS::PropertyParserState&);

    bool consumeBorderSpacingShorthand(CSS::PropertyParserState&);

    bool consumeBorderRadiusShorthand(CSS::PropertyParserState&);
    bool consumeWebkitBorderRadiusShorthand(CSS::PropertyParserState&);

    bool consumeBorderImageShorthand(CSS::PropertyParserState&);
    bool consumeWebkitBorderImageShorthand(CSS::PropertyParserState&);

    bool consumeMaskBorderShorthand(CSS::PropertyParserState&);
    bool consumeWebkitMaskBoxImageShorthand(CSS::PropertyParserState&);

    bool consumeFlexShorthand(CSS::PropertyParserState&);

    bool consumePageBreakAfterShorthand(CSS::PropertyParserState&);
    bool consumePageBreakBeforeShorthand(CSS::PropertyParserState&);
    bool consumePageBreakInsideShorthand(CSS::PropertyParserState&);

    bool consumeWebkitColumnBreakAfterShorthand(CSS::PropertyParserState&);
    bool consumeWebkitColumnBreakBeforeShorthand(CSS::PropertyParserState&);
    bool consumeWebkitColumnBreakInsideShorthand(CSS::PropertyParserState&);

    bool consumeWebkitTextOrientationShorthand(CSS::PropertyParserState&);

    bool consumeTransformOriginShorthand(CSS::PropertyParserState&);
    bool consumePerspectiveOriginShorthand(CSS::PropertyParserState&);
    bool consumePrefixedPerspectiveShorthand(CSS::PropertyParserState&);
    bool consumeOffsetShorthand(CSS::PropertyParserState&);
    bool consumeListStyleShorthand(CSS::PropertyParserState&);

    bool consumeOverscrollBehaviorShorthand(CSS::PropertyParserState&);

    bool consumeContainerShorthand(CSS::PropertyParserState&);
    bool consumeContainIntrinsicSizeShorthand(CSS::PropertyParserState&);

    bool consumeAnimationRangeShorthand(CSS::PropertyParserState&);
    bool consumeScrollTimelineShorthand(CSS::PropertyParserState&);
    bool consumeViewTimelineShorthand(CSS::PropertyParserState&);

    bool consumeLineClampShorthand(CSS::PropertyParserState&);

    bool consumeTextBoxShorthand(CSS::PropertyParserState&);

    bool consumeTextWrapShorthand(CSS::PropertyParserState&);
    bool consumeWhiteSpaceShorthand(CSS::PropertyParserState&);

    bool consumePositionTryShorthand(CSS::PropertyParserState&);

    bool consumeMarkerShorthand(CSS::PropertyParserState&);

private:
    // Inputs:
    CSSParserTokenRange m_range;
    const CSSParserContext& m_context;

    // Outputs:
    Vector<CSSProperty, 256>* m_parsedProperties;
};

CSSPropertyID cssPropertyID(StringView);
WEBCORE_EXPORT CSSValueID cssValueKeywordID(StringView);
bool isCustomPropertyName(StringView);

bool isInitialValueForLonghand(CSSPropertyID, const CSSValue&);
ASCIILiteral initialValueTextForLonghand(CSSPropertyID);
CSSValueID initialValueIDForLonghand(CSSPropertyID); // Returns CSSPropertyInvalid if not a keyword.

} // namespace WebCore
