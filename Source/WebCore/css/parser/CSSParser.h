/*
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
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

#include "CSSParserEnum.h"
#include <optional>
#include <utility>
#include <wtf/Forward.h>

namespace WebCore {

class CSSParserObserver;
class CSSSelectorList;
class Element;
class ImmutableStyleProperties;
class MutableStyleProperties;
class StyleRuleBase;
class StyleRuleKeyframe;
class StyleSheetContents;

struct CSSParserContext;

enum CSSPropertyID : uint16_t;
enum CSSValueID : uint16_t;

enum class IsImportant : bool;

class CSSParser {
public:
    enum class ParseResult {
        Changed,
        Unchanged,
        Error
    };

    static void parseSheet(const String&, const CSSParserContext&, StyleSheetContents&);
    static void parseSheetForInspector(const String&, const CSSParserContext&, StyleSheetContents&, CSSParserObserver&);

    static RefPtr<StyleRuleBase> parseRule(const String&, const CSSParserContext&, StyleSheetContents*, CSSParserEnum::NestedContext = { });
    static RefPtr<StyleRuleKeyframe> parseKeyframeRule(const String&, const CSSParserContext&);

    static bool parseSupportsCondition(const String&, const CSSParserContext&);

    static ParseResult parseValue(MutableStyleProperties&, CSSPropertyID, const String&, IsImportant, const CSSParserContext&);
    static ParseResult parseCustomPropertyValue(MutableStyleProperties&, const AtomString& propertyName, const String&, IsImportant, const CSSParserContext&);

    WEBCORE_EXPORT static bool parseDeclaration(MutableStyleProperties&, const String&, const CSSParserContext&);
    static Ref<ImmutableStyleProperties> parseInlineStyleDeclaration(const String&, const Element&);
    static void parseDeclarationForInspector(const String&, const CSSParserContext&, CSSParserObserver&);

    WEBCORE_EXPORT static std::optional<CSSSelectorList> parseSelectorList(const String&, const CSSParserContext&, StyleSheetContents* = nullptr, CSSParserEnum::NestedContext = { });
};

} // namespace WebCore
