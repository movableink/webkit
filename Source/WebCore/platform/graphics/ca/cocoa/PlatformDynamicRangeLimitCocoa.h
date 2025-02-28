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

#pragma once

#if PLATFORM(COCOA)

#import "PlatformDynamicRangeLimit.h"

#if HAVE(SUPPORT_HDR_DISPLAY_APIS)
#import <pal/spi/cocoa/QuartzCoreSPI.h>
#endif // HAVE(SUPPORT_HDR_DISPLAY_APIS)

namespace WebCore {

#if HAVE(SUPPORT_HDR_DISPLAY_APIS)
inline CADynamicRange platformDynamicRangeLimitString(PlatformDynamicRangeLimit platformDynamicRangeLimit)
{
    // FIXME: Unstage, see follow-up to rdar://145750574
    static CADynamicRange const WebKitCADynamicRangeStandard = (id) CFSTR ("standard");
    static CADynamicRange const WebKitCADynamicRangeConstrainedHigh = (id) CFSTR ("constrainedHigh");
    static CADynamicRange const WebKitCADynamicRangeHigh = (id) CFSTR ("high");

    constexpr auto betweenStandardAndConstrainedHigh = (PlatformDynamicRangeLimit::standard().value() + PlatformDynamicRangeLimit::constrainedHigh().value()) / 2;
    if (platformDynamicRangeLimit.value() < betweenStandardAndConstrainedHigh)
        return WebKitCADynamicRangeStandard;

    constexpr auto betweenConstrainedHighAndHigh = (PlatformDynamicRangeLimit::constrainedHigh().value() + PlatformDynamicRangeLimit::noLimit().value()) / 2;
    if (platformDynamicRangeLimit.value() < betweenConstrainedHighAndHigh)
        return WebKitCADynamicRangeConstrainedHigh;

    return WebKitCADynamicRangeHigh;
}
#endif // HAVE(SUPPORT_HDR_DISPLAY_APIS) else

} // namespace WebCore

#endif // PLATFORM(COCOA)
