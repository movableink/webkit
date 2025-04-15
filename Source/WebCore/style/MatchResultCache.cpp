/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "MatchResultCache.h"

#include "MatchResult.h"
#include "StyleProperties.h"
#include "StyledElement.h"

namespace WebCore {
namespace Style {

MatchResultCache::MatchResultCache() = default;
MatchResultCache::~MatchResultCache() = default;

RefPtr<const MatchResult> MatchResultCache::get(const Element& element)
{
    auto it = m_cachedMatchResults.find(element);
    if (it == m_cachedMatchResults.end())
        return { };

    auto& matchResult = *it->value;

    auto inlineStyleMatches = [&] {
        auto* styledElement = dynamicDowncast<StyledElement>(element);
        if (!styledElement || !styledElement->inlineStyle())
            return false;

        auto& inlineStyle = *styledElement->inlineStyle();

        for (auto& declaration : matchResult.authorDeclarations) {
            if (&declaration.properties.get() == &inlineStyle)
                return true;
        }
        return false;
    }();

    if (!inlineStyleMatches) {
        m_cachedMatchResults.remove(it);
        return { };
    }

    return &matchResult;
}

void MatchResultCache::update(const Element& element, const MatchResult& matchResult)
{
    // For now we cache match results if there is mutable inline style. This way we can avoid
    // selector matching when it gets mutated again.
    auto* styledElement = dynamicDowncast<StyledElement>(element);
    if (styledElement && styledElement->inlineStyle() && styledElement->inlineStyle()->isMutable())
        m_cachedMatchResults.set(element, &matchResult);
    else
        m_cachedMatchResults.remove(element);
}

}
}
