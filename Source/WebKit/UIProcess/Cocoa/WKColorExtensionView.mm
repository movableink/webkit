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
#import "WKColorExtensionView.h"

@interface WKColorExtensionView () <CAAnimationDelegate>
@end

@implementation WKColorExtensionView {
    BOOL _isVisible;
    BOOL _isDoneFadingIn;
    __weak id<WKColorExtensionViewDelegate> _delegate;
    RetainPtr<WebCore::CocoaColor> _targetColor;
}

- (instancetype)initWithFrame:(CGRect)frame delegate:(id<WKColorExtensionViewDelegate>)delegate
{
    if (!(self = [super initWithFrame:frame]))
        return nil;

    _delegate = delegate;
    return self;
}

- (void)fadeToColor:(WebCore::CocoaColor *)color
{
    [self _fadeToColor:color visible:YES];
}

- (void)fadeOut
{
    [self _fadeToColor:[WebCore::CocoaColor clearColor] visible:NO];
}

- (void)_fadeToColor:(WebCore::CocoaColor *)color visible:(BOOL)visible
{
    if (!visible && !_isVisible)
        return;

    BOOL wasVisible = std::exchange(_isVisible, visible);
    if (wasVisible && !visible) {
        [_delegate colorExtensionViewWillFadeOut:self];
        _isDoneFadingIn = NO;
    }

    if (visible)
        self.hidden = NO;

    static constexpr auto animationDuration = 0.1;

    RetainPtr fromColor = [self.layer backgroundColor];
    RetainPtr toColor = [color CGColor];
    _targetColor = color;
    self.layer.backgroundColor = toColor.get();

    RetainPtr animation = [CABasicAnimation animationWithKeyPath:@"backgroundColor"];
    [animation setFromValue:(__bridge id)fromColor.get()];
    [animation setToValue:(__bridge id)toColor.get()];
    [animation setDuration:animationDuration];
    [animation setFillMode:kCAFillModeForwards];
    [animation setRemovedOnCompletion:NO];
    [animation setDelegate:self];

    [self.layer addAnimation:animation.get() forKey:@"WKColorExtensionViewFade"];
}

- (void)animationDidStop:(CAAnimation *)animation finished:(BOOL)finished
{
    if (!finished)
        return;

    if (!_isVisible) {
        self.hidden = YES;
        return;
    }

    if (!std::exchange(_isDoneFadingIn, YES))
        [_delegate colorExtensionViewDidFadeIn:self];
}

- (BOOL)isHiddenOrFadingOut
{
    return self.hidden || !_isVisible;
}

- (void)cancelFadeAnimation
{
    if (!_targetColor)
        return;

    [self.layer removeAnimationForKey:@"WKColorExtensionViewFade"];
    self.layer.backgroundColor = [_targetColor CGColor];
    self.hidden = !_isVisible;
}

@end
