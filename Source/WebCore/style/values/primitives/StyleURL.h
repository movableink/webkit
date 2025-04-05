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

#include "CSSURL.h"
#include "StyleValueTypes.h"

namespace WebCore {
namespace Style {

struct URL {
    WTF::URL resolved;

    static URL none() { return { .resolved = { } }; }
    bool isNone() const { return resolved.isNull(); }

    bool operator==(const URL&) const = default;
};

template<size_t I> const auto& get(const URL& value)
{
    return value.resolved;
}

// MARK: Conversion

// Special conversion function for use by filters code.
URL toStyleWithBaseURL(const CSS::URL&, const WTF::URL& baseURL);

template<> struct ToCSS<URL> { auto operator()(const URL&, const RenderStyle&) -> CSS::URL; };
template<> struct ToStyle<CSS::URL> { auto operator()(const CSS::URL&, const BuilderState&) -> URL; };

// MARK: Logging

TextStream& operator<<(TextStream&, const URL&);

} // namespace Style
} // namespace WebCore

DEFINE_TUPLE_LIKE_CONFORMANCE(WebCore::Style::URL, 1)
