/*
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Nicholas Shanks <webkit@nickshanks.com>
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2012, 2013 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#include "config.h"
#include "CSSParser.h"

#include "CSSKeyframeRule.h"
#include "CSSParserImpl.h"
#include "CSSParserTokenRange.h"
#include "CSSPendingSubstitutionValue.h"
#include "CSSProperty.h"
#include "CSSPropertyParser.h"
#include "CSSPropertyParserState.h"
#include "CSSSelectorParser.h"
#include "CSSSupportsParser.h"
#include "CSSTokenizer.h"
#include "CSSValuePool.h"
#include "Document.h"
#include "Element.h"
#include "ImmutableStyleProperties.h"
#include "MutableStyleProperties.h"
#include "Page.h"
#include "RenderTheme.h"
#include "Settings.h"
#include "StyleRule.h"
#include "StyleSheetContents.h"
#include <wtf/NeverDestroyed.h>
#include <wtf/text/StringBuilder.h>

namespace WebCore {

void CSSParser::parseSheet(const String& string, const CSSParserContext& context, StyleSheetContents& sheet)
{
    return CSSParserImpl::parseStyleSheet(string, context, sheet);
}

void CSSParser::parseSheetForInspector(const String& string, const CSSParserContext& context, StyleSheetContents& sheet, CSSParserObserver& observer)
{
    return CSSParserImpl::parseStyleSheetForInspector(string, context, sheet, observer);
}

RefPtr<StyleRuleBase> CSSParser::parseRule(const String& string, const CSSParserContext& context, StyleSheetContents* sheet, CSSParserEnum::NestedContext nestedContext)
{
    return CSSParserImpl::parseRule(string, context, sheet, CSSParserImpl::AllowedRules::ImportRules, nestedContext);
}

RefPtr<StyleRuleKeyframe> CSSParser::parseKeyframeRule(const String& string, const CSSParserContext& context)
{
    RefPtr keyframe = CSSParserImpl::parseRule(string, context, nullptr, CSSParserImpl::AllowedRules::KeyframeRules);
    return downcast<StyleRuleKeyframe>(keyframe.get());
}

bool CSSParser::parseSupportsCondition(const String& condition, const CSSParserContext& context)
{
    CSSParserImpl parser(context, condition);
    if (!parser.tokenizer())
        return false;
    return CSSSupportsParser::supportsCondition(parser.tokenizer()->tokenRange(), parser, CSSSupportsParser::ParsingMode::AllowBareDeclarationAndGeneralEnclosed) == CSSSupportsParser::Supported;
}

CSSParser::ParseResult CSSParser::parseValue(MutableStyleProperties& declaration, CSSPropertyID propertyID, const String& string, IsImportant important, const CSSParserContext& context)
{
    ASSERT(!string.isEmpty());
    return CSSParserImpl::parseValue(declaration, propertyID, string, important, context);
}

CSSParser::ParseResult CSSParser::parseCustomPropertyValue(MutableStyleProperties& declaration, const AtomString& propertyName, const String& string, IsImportant important, const CSSParserContext& context)
{
    return CSSParserImpl::parseCustomPropertyValue(declaration, propertyName, string, important, context);
}

std::optional<CSSSelectorList> CSSParser::parseSelectorList(const String& string, const CSSParserContext& context, StyleSheetContents* styleSheet, CSSParserEnum::NestedContext nestedContext)
{
    return parseCSSSelectorList(CSSTokenizer(string).tokenRange(), context, styleSheet, nestedContext);
}

Ref<ImmutableStyleProperties> CSSParser::parseInlineStyleDeclaration(const String& string, const Element& element)
{
    return CSSParserImpl::parseInlineStyleDeclaration(string, element);
}

bool CSSParser::parseDeclaration(MutableStyleProperties& declaration, const String& string, const CSSParserContext& context)
{
    return CSSParserImpl::parseDeclarationList(&declaration, string, context);
}

void CSSParser::parseDeclarationForInspector(const String& string, const CSSParserContext& context, CSSParserObserver& observer)
{
    CSSParserImpl::parseDeclarationListForInspector(string, context, observer);
}

} // namespace WebCore
