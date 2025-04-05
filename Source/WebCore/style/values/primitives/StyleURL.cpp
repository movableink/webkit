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

#include "config.h"
#include "StyleURL.h"

#include "Document.h"
#include "StyleBuilderState.h"
#include <wtf/text/TextStream.h>

namespace WebCore {
namespace Style {

// Possible states/transitions:
//
// 1. empty (empty string)
//      CSS:    [.specified = "", .resolved = "" ]
//      Style:  [.resolved = ""]
// 2. local (starts with #)
//      CSS:    [.specified = "#foo", .resolved = "#foo"]
//      Style:  [.resolved = "#foo"]
// 3. relative, in stylesheet
//      CSS:    [.specified = "foo/bar.png", .resolved = { stylesheet-base-url, "foo/bar.png" }]
//      Style:  [.resolved = { stylesheet-base-url, "foo/bar.png" }]
// 4. relative, in document with base-url
//      CSS:    [.specified = "foo/bar.png", .resolved = { "foo/bar.png", document-base-url }]
//      Style:  [.resolved = { document-base-url, "foo/bar.png" }]
// 5. relative, in document without base-url
//      CSS:    [.specified = "foo/bar.png", .resolved = null-url]
//      Style:  [.resolved = "foo/bar.png"]

auto toStyleWithBaseURL(const CSS::URL& url, const WTF::URL& baseURL) -> URL
{
    if (url.resolved.isNull()) {
        return {
            .resolved = WTF::URL { baseURL, url.specified },
        };
    }

    return {
        .resolved = url.resolved,
    };
}

auto ToCSS<URL>::operator()(const URL& url, const RenderStyle&) -> CSS::URL
{
    return {
        .specified = url.resolved.string(),
        .resolved = url.resolved,
    };
}

auto ToStyle<CSS::URL>::operator()(const CSS::URL& url, const BuilderState& state) -> URL
{
    return toStyleWithBaseURL(url, state.document().baseURL());
}

// MARK: - Logging

TextStream& operator<<(TextStream& ts, const URL& value)
{
    return ts << "url(\""_s << value.resolved << "\""_s << ")"_s;
}

} // namespace Style
} // namespace WebCore
