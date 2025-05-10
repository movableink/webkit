/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#import "AppDelegate.h"

#import "WebViewController.h"
#import <WebKit/WKWebsiteDataStorePrivate.h>
#import <WebKit/_WKWebsiteDataStoreDelegate.h>

#import <WebKit/WebKit.h>

@interface AppDelegate () <_WKWebsiteDataStoreDelegate>
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIStoryboard *frameworkMainStoryboard = [UIStoryboard storyboardWithName:@"Main" bundle:[NSBundle bundleForClass:[AppDelegate class]]];
    WebViewController *viewController = [frameworkMainStoryboard instantiateInitialViewController];
#pragma clang diagnostic pop
    if (!viewController)
        return NO;

    WKWebsiteDataStore *dataStore = viewController.dataStore;
    [WKWebsiteDataStore _setWebPushActionHandler:^(_WKWebPushAction *action) {
        return dataStore;
    }];
    dataStore._delegate = self;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    NSURL *url = launchOptions[UIApplicationLaunchOptionsURLKey];
#pragma clang diagnostic pop

    if (url)
        viewController.initialURL = url;

    if (!self.window)
// FIXME: rdar://151039019 (Replace using 'init' in AppDelegate.m with non-deprecated method)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        self.window = [[UIWindow alloc] init];
#pragma clang diagnostic pop
    self.window.rootViewController = viewController;
    [self.window makeKeyAndVisible];

    return YES;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-implementations"
- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey, id> *)options
{
    WebViewController *controller = (WebViewController *)self.window.rootViewController;
    [controller.currentWebView loadRequest:[NSURLRequest requestWithURL:url]];
    return YES;
}
#pragma clang diagnostic pop

- (void)websiteDataStore:(WKWebsiteDataStore *)dataStore openWindow:(NSURL *)url fromServiceWorkerOrigin:(WKSecurityOrigin *)serviceWorkerOrigin completionHandler:(void (^)(WKWebView *newWebView))completionHandler
{
    WebViewController *controller = (WebViewController *)self.window.rootViewController;
    [controller addWebView];
    [controller.currentWebView loadRequest:[NSURLRequest requestWithURL:url]];
    completionHandler(controller.currentWebView);
}

@end
