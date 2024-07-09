/*
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2019 Konstantin Tokarev <annulen@yandex.ru>

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

#include "config.h"
#include "QStyleHelpers.h"

#include "Document.h"
#include "CSSComputedStyleDeclaration.h"
#include "CSSPropertyParser.h"
#include "StyleResolver.h"
#include "StyleScope.h"
#include "StyledElement.h"

namespace WebCore {

namespace QStyleHelpers {

static const StyleProperties* inlineStyle(const Element& element)
{
    if (!is<StyledElement>(element))
        return nullptr;
    return downcast<StyledElement>(element).inlineStyle();
}

String getInlineStyleProperty(const Element& element, StringView propertyName)
{
    auto propID = cssPropertyID(propertyName);
    if (propID == CSSPropertyInvalid)
        return nullString();

    const auto* style = inlineStyle(element);
    if (!style)
        return nullString();

    return style->getPropertyValue(propID);
}

String getCascadedStyleProperty(const Element& element, StringView propertyName)
{
    auto propID = cssPropertyID(propertyName);
    if (propID == CSSPropertyInvalid)
        return nullString();

    const auto* style = inlineStyle(element);
    if (style && style->propertyIsImportant(propID))
        return style->getPropertyValue(propID);

    // We are going to resolve the style property by walking through the
    // list of non-inline matched CSS rules for the element, looking for
    // the highest priority definition.

    // Get an array of matched CSS rules for the given element sorted
    // by importance and inheritance order. This include external CSS
    // declarations, as well as embedded and inline style declarations.

    Document& document = element.document();
    Vector<RefPtr<const StyleRule>> rules = document.styleScope().resolver().styleRulesForElement(&element, Style::Resolver::AuthorCSSRules);
    for (int i = rules.size(); i > 0; --i) {
        if (!rules[i - 1]->isStyleRule())
            continue;
        const StyleRule* styleRule = static_cast<const StyleRule*>(rules[i - 1].get());

        if (styleRule->properties().propertyIsImportant(propID))
            return styleRule->properties().getPropertyValue(propID);

        if (!style || style->getPropertyValue(propID).isEmpty())
            style = &styleRule->properties();
    }

    if (!style)
        return QString();
    return style->getPropertyValue(propID);
}

String getComputedStyleProperty(Element& element, StringView propertyName)
{
    auto propID = cssPropertyID(propertyName);
    if (propID == CSSPropertyInvalid)
        return nullString();

    return getComputedStyleProperty(element, propID);
}

String getComputedStyleProperty(Element& element, CSSPropertyID propID)
{
    return CSSComputedStyleDeclaration::create(element, CSSComputedStyleDeclaration::AllowVisited::Yes)->getPropertyValue(propID);
}

void setInlineStyleProperty(Element& element, StringView propertyName, const String& value, bool important)
{
    auto propID = cssPropertyID(propertyName);
    if (propID == CSSPropertyInvalid)
        return;

    if (!is<StyledElement>(element))
        return;
    downcast<StyledElement>(element).setInlineStyleProperty(propID, value, important);
}

} // namespace QStyleHelpers

} // namespace WebCore
