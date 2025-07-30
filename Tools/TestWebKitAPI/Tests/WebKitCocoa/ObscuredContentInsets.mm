/*
 * Copyright (C) 2016-2024 Apple Inc. All rights reserved.
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

#import "AppKitSPI.h"
#import "DeprecatedGlobalValues.h"
#import "PlatformUtilities.h"
#import "TestCocoa.h"
#import "TestNavigationDelegate.h"
#import "TestWKWebView.h"
#import <WebCore/ColorCocoa.h>
#import <WebCore/ColorSerialization.h>
#import <WebKit/WKPreferencesPrivate.h>
#import <WebKit/WKWebViewConfigurationPrivate.h>
#import <WebKit/WKWebViewPrivate.h>
#import <WebKit/WKWebViewPrivateForTesting.h>
#import <wtf/RetainPtr.h>

@interface NSView (ContentInsetFillSPI)
@property (nonatomic, readonly) NSColor *captureColor;
@end

@interface WKWebView (ObscuredContentInsets) <NSScrollViewSeparatorTrackingAdapter>
@end

@interface FullscreenChangeMessageHandler : NSObject <WKScriptMessageHandler>
@end

@implementation FullscreenChangeMessageHandler
- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message
{
    NSString *bodyString = (NSString *)[message body];
    if ([bodyString isEqualToString:@"fullscreenchange"])
        receivedFullscreenChangeMessage = true;
    else if ([bodyString isEqualToString:@"load"])
        receivedLoadedMessage = true;
}
@end

namespace TestWebKitAPI {

TEST(TopContentInset, Fullscreen)
{
    RetainPtr<WKWebViewConfiguration> configuration = adoptNS([[WKWebViewConfiguration alloc] init]);
    [configuration preferences].elementFullscreenEnabled = YES;
    RetainPtr<FullscreenChangeMessageHandler> handler = adoptNS([[FullscreenChangeMessageHandler alloc] init]);
    [[configuration userContentController] addScriptMessageHandler:handler.get() name:@"fullscreenChangeHandler"];
    RetainPtr<WKWebView> webView = adoptNS([[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) configuration:configuration.get()]);
    [webView _setTopContentInset:10];
    [webView _setAutomaticallyAdjustsContentInsets:NO];

    RetainPtr<NSWindow> window = adoptNS([[NSWindow alloc] initWithContentRect:[webView frame] styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:NO]);
    [[window contentView] addSubview:webView.get()];
    [window makeKeyAndOrderFront:nil];

    NSURLRequest *request = [NSURLRequest requestWithURL:[NSBundle.test_resourcesBundle URLForResource:@"FullscreenTopContentInset" withExtension:@"html"]];
    [webView loadRequest:request];
    TestWebKitAPI::Util::run(&receivedLoadedMessage);

    NSEvent *event = [NSEvent mouseEventWithType:NSEventTypeLeftMouseDown location:NSMakePoint(5, 5) modifierFlags:0 timestamp:0 windowNumber:window.get().windowNumber context:0 eventNumber:0 clickCount:0 pressure:0];
    [webView mouseDown:event];

    TestWebKitAPI::Util::run(&receivedFullscreenChangeMessage);
    ASSERT_EQ(window.get().screen.frame.size.width, webView.get().frame.size.width);
    ASSERT_EQ(window.get().screen.frame.size.height + webView.get()._topContentInset, webView.get().frame.size.height);

    receivedFullscreenChangeMessage = false;
    [webView mouseDown:event];
    TestWebKitAPI::Util::run(&receivedFullscreenChangeMessage);
    ASSERT_EQ(10, webView.get()._topContentInset);
}

TEST(TopContentInset, AutomaticAdjustment)
{
    RetainPtr<WKWebViewConfiguration> configuration = adoptNS([[WKWebViewConfiguration alloc] init]);
    RetainPtr<WKWebView> webView = adoptNS([[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 400, 400) configuration:configuration.get()]);

    RetainPtr<NSWindow> window = adoptNS([[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 400) styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView) backing:NSBackingStoreBuffered defer:NO]);
    [[window contentView] addSubview:webView.get()];
    [window makeKeyAndOrderFront:nil];

    [webView loadHTMLString:@"" baseURL:nil];
    [webView _test_waitForDidFinishNavigation];

    ASSERT_GT(webView.get()._topContentInset, 10);
}

TEST(TopContentInset, AutomaticAdjustmentDisabled)
{
    RetainPtr<WKWebViewConfiguration> configuration = adoptNS([[WKWebViewConfiguration alloc] init]);
    RetainPtr<WKWebView> webView = adoptNS([[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 400, 400) configuration:configuration.get()]);
    [webView _setAutomaticallyAdjustsContentInsets:NO];

    RetainPtr<NSWindow> window = adoptNS([[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 400) styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView) backing:NSBackingStoreBuffered defer:NO]);
    [[window contentView] addSubview:webView.get()];
    [window makeKeyAndOrderFront:nil];

    [webView loadHTMLString:@"" baseURL:nil];
    [webView _test_waitForDidFinishNavigation];

    ASSERT_EQ(webView.get()._topContentInset, 0);
}

TEST(TopContentInset, AutomaticAdjustmentDoesNotAffectInsetViews)
{
    RetainPtr<WKWebViewConfiguration> configuration = adoptNS([[WKWebViewConfiguration alloc] init]);
    RetainPtr<WKWebView> webView = adoptNS([[WKWebView alloc] initWithFrame:NSMakeRect(100, 100, 200, 200) configuration:configuration.get()]);

    RetainPtr<NSWindow> window = adoptNS([[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 400) styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView) backing:NSBackingStoreBuffered defer:NO]);
    [[window contentView] addSubview:webView.get()];
    [window makeKeyAndOrderFront:nil];

    [webView loadHTMLString:@"" baseURL:nil];
    [webView _test_waitForDidFinishNavigation];

    ASSERT_EQ(webView.get()._topContentInset, 0);
}

TEST(ObscuredContentInsets, SetAndGetObscuredContentInsets)
{
    RetainPtr webView = adoptNS([[TestWKWebView alloc] initWithFrame:NSMakeRect(0, 0, 800, 600)]);
    [webView _setAutomaticallyAdjustsContentInsets:NO];

    auto initialInsets = NSEdgeInsetsMake(100, 150, 0, 10);
    [webView _setObscuredContentInsets:initialInsets immediate:NO];
    [webView synchronouslyLoadTestPageNamed:@"simple"];
    EXPECT_TRUE(NSEdgeInsetsEqual([webView _obscuredContentInsets], initialInsets));

    auto finalInsets = NSEdgeInsetsMake(50, 100, 0, 10);
    [webView _setObscuredContentInsets:finalInsets immediate:NO];
    EXPECT_TRUE(NSEdgeInsetsEqual([webView _obscuredContentInsets], finalInsets));
}

TEST(ObscuredContentInsets, ScrollViewFrameWithObscuredInsets)
{
    RetainPtr webView = adoptNS([[TestWKWebView alloc] initWithFrame:NSMakeRect(0, 0, 800, 600)]);
    [webView _setAutomaticallyAdjustsContentInsets:NO];

    [webView _setObscuredContentInsets:NSEdgeInsetsMake(100, 150, 30, 10) immediate:NO];
    [webView synchronouslyLoadTestPageNamed:@"simple"];

    EXPECT_EQ([webView scrollViewFrame], NSMakeRect(150, 0, 640, 600));
}

#if ENABLE(CONTENT_INSET_BACKGROUND_FILL)

TEST(ObscuredContentInsets, ResizeContentInsetFillView)
{
    RetainPtr webView = adoptNS([[TestWKWebView alloc] initWithFrame:NSMakeRect(0, 0, 400, 600)]);
    [webView _setAutomaticallyAdjustsContentInsets:NO];
    [webView _setObscuredContentInsets:NSEdgeInsetsMake(100, 100, 0, 0) immediate:NO];
    [webView synchronouslyLoadTestPageNamed:@"simple"];
    [webView waitForNextPresentationUpdate];
    EXPECT_EQ(NSMakeRect(0, 0, 400, 100), [webView _contentInsetFillViewForTesting].frame);

    [webView setFrame:NSMakeRect(0, 0, 800, 600)];
    [webView waitForNextVisibleContentRectUpdate];
    [webView waitForNextPresentationUpdate];
    EXPECT_EQ(NSMakeRect(0, 0, 800, 100), [webView _contentInsetFillViewForTesting].frame);
}

TEST(ObscuredContentInsets, ContentInsetFillCaptureColor)
{
    RetainPtr webView = adoptNS([[TestWKWebView alloc] initWithFrame:NSMakeRect(0, 0, 600, 400)]);
    [webView _setAutomaticallyAdjustsContentInsets:NO];
    [webView _setObscuredContentInsets:NSEdgeInsetsMake(100, 0, 0, 0) immediate:NO];
    [webView synchronouslyLoadTestPageNamed:@"simple"];
    [webView waitForNextPresentationUpdate];

    auto initialColor = WebCore::colorFromCocoaColor([[webView _contentInsetFillViewForTesting] captureColor]);

    [webView stringByEvaluatingJavaScript:@"document.body.style.backgroundColor = '#222'"];
    [webView waitForNextPresentationUpdate];

    auto finalColor = WebCore::colorFromCocoaColor([[webView _contentInsetFillViewForTesting] captureColor]);

    EXPECT_EQ(WebCore::serializationForCSS(initialColor), "rgb(255, 255, 255)"_s);
    EXPECT_EQ(WebCore::serializationForCSS(finalColor), "rgb(34, 34, 34)"_s);
}

#endif // ENABLE(CONTENT_INSET_BACKGROUND_FILL)

} // namespace TestWebKitAPI

#endif
