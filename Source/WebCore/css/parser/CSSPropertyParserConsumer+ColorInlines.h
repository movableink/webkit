/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "CSSParserFastPaths.h"
#include "CSSPlatformColorResolutionState.h"
#include "CSSPropertyParserConsumer+Color.h"

namespace WebCore {
namespace CSSPropertyParserHelpers {

// MARK: <color> parsing (raw)

template<typename F> WebCore::Color parseColorRaw(const String& string, const CSSParserContext& context, NOESCAPE const F& lazySlowPathOptionsFunctor)
{
    if (auto color = CSSParserFastPaths::parseSimpleColor(string, context))
        return *color;

    // To avoid doing anything unnecessary before the fast path can run, callers bundle up
    // a functor to generate the slow path parameters.
    auto [options, eagerResolutionState, eagerResolutionDelegate] = lazySlowPathOptionsFunctor();

    // If a delegate is provided, hook it up to the context here. By having it live on the stack,
    // we avoid allocating it.
    if (eagerResolutionDelegate)
        eagerResolutionState.delegate = &eagerResolutionDelegate.value();

    return parseColorRawSlow(string, context, options, eagerResolutionState);
}

} // namespace CSSPropertyParserHelpers
} // namespace WebCore
