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

#import "config.h"

#if PLATFORM(MAC)

#import "TestWKWebView.h"
#import <WebKit/WKWebViewPrivate.h>
#import <WebKit/WKWebViewPrivateForTesting.h>

namespace TestWebKitAPI {

TEST(CustomSwipeViewTests, WindowRelativeBoundsForCustomSwipeViews)
{
    RetainPtr customSwipeView = adoptNS([[NSView alloc] initWithFrame:NSMakeRect(0, 0, 800, 600)]);
    RetainPtr webView = adoptNS([[TestWKWebView alloc] initWithFrame:NSMakeRect(0, 0, 800, 600)]);
    [webView addSubview:customSwipeView.get()];
    [webView _setCustomSwipeViews:@[ customSwipeView.get() ]];
    [webView _setCustomSwipeViewsObscuredContentInsets:NSEdgeInsetsMake(100, 200, 50, 50)];

    __auto_type boundsForCustomSwipeView = [webView _windowRelativeBoundsForCustomSwipeViewsForTesting];
    EXPECT_EQ(boundsForCustomSwipeView.origin.x, 200.0);
    EXPECT_EQ(boundsForCustomSwipeView.origin.y, 50.0);
    EXPECT_EQ(boundsForCustomSwipeView.size.width, 550.0);
    EXPECT_EQ(boundsForCustomSwipeView.size.height, 450.0);
}

} // namespace TestWebKitAPI

#endif // PLATFORM(MAC)
