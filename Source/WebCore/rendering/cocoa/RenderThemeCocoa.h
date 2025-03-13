/*
 * Copyright (C) 2016-2022 Apple Inc. All rights reserved.
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

#include "Icon.h"
#include "RenderTheme.h"
#include <wtf/RetainPtr.h>

OBJC_CLASS NSDateComponentsFormatter;
struct AttachmentLayout;

namespace WebCore {

class RenderThemeCocoa : public RenderTheme {
public:
    WEBCORE_EXPORT static RenderThemeCocoa& singleton();

protected:
    virtual Color pictureFrameColor(const RenderBox&);
#if ENABLE(ATTACHMENT_ELEMENT)
    int attachmentBaseline(const RenderAttachment&) const final;
    void paintAttachmentText(GraphicsContext&, AttachmentLayout*) final;
#endif

    Color platformSpellingMarkerColor(OptionSet<StyleColorOptions>) const override;
    Color platformDictationAlternativesMarkerColor(OptionSet<StyleColorOptions>) const override;
    Color platformGrammarMarkerColor(OptionSet<StyleColorOptions>) const override;

    Color controlTintColor(const RenderStyle&, OptionSet<StyleColorOptions>) const;

    void adjustCheckboxStyle(RenderStyle&, const Element*) const override;
    bool paintCheckbox(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustRadioStyle(RenderStyle&, const Element*) const override;
    bool paintRadio(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustButtonStyle(RenderStyle&, const Element*) const override;
    bool paintButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustColorWellStyle(RenderStyle&, const Element*) const override;
    bool paintColorWell(const RenderBox&, const PaintInfo&, const IntRect&) override;
    void paintColorWellDecorations(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustInnerSpinButtonStyle(RenderStyle&, const Element*) const override;
    bool paintInnerSpinButton(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustTextFieldStyle(RenderStyle&, const Element*) const override;
    bool paintTextField(const RenderBox&, const PaintInfo&, const FloatRect&) override;
    void paintTextFieldDecorations(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustTextAreaStyle(RenderStyle&, const Element*) const override;
    bool paintTextArea(const RenderBox&, const PaintInfo&, const FloatRect&) override;
    void paintTextAreaDecorations(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustMenuListStyle(RenderStyle&, const Element*) const override;
    bool paintMenuList(const RenderBox&, const PaintInfo&, const FloatRect&) override;
    void paintMenuListDecorations(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustMenuListButtonStyle(RenderStyle&, const Element*) const override;
    void paintMenuListButtonDecorations(const RenderBox&, const PaintInfo&, const FloatRect&) override;
    bool paintMenuListButton(const RenderBox&, const PaintInfo&, const IntRect&) final;

    void adjustMeterStyle(RenderStyle&, const Element*) const override;
    bool paintMeter(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustListButtonStyle(RenderStyle&, const Element*) const override;
    bool paintListButton(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustProgressBarStyle(RenderStyle&, const Element*) const override;
    bool paintProgressBar(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSliderTrackStyle(RenderStyle&, const Element*) const override;
    bool paintSliderTrack(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSliderThumbSize(RenderStyle&, const Element*) const override;
    void adjustSliderThumbStyle(RenderStyle&, const Element*) const override;
    bool paintSliderThumb(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldStyle(RenderStyle&, const Element*) const override;
    bool paintSearchField(const RenderBox&, const PaintInfo&, const FloatRect&) override;
    void paintSearchFieldDecorations(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldCancelButtonStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldCancelButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldDecorationPartStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldResultsDecorationPartStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldResultsDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldResultsButtonStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldResultsButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSwitchStyle(RenderStyle&, const Element*) const override;
    bool paintSwitchThumb(const RenderBox&, const PaintInfo&, const FloatRect&) override;
    bool paintSwitchTrack(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    bool supportsFocusRing(const RenderElement&, const RenderStyle&) const override;

#if USE(APPLE_INTERNAL_SDK)
#import <WebKitAdditions/RenderThemeCocoaAdditions.h>
#endif

private:
    void purgeCaches() override;

    bool shouldHaveCapsLockIndicator(const HTMLInputElement&) const final;

    void paintFileUploadIconDecorations(const RenderBox& inputRenderer, const RenderBox& buttonRenderer, const PaintInfo&, const IntRect&, Icon*, FileUploadDecorations) override;

    Seconds animationRepeatIntervalForProgressBar(const RenderProgress&) const final;

#if ENABLE(APPLE_PAY)
    void adjustApplePayButtonStyle(RenderStyle&, const Element*) const override;
#endif

#if ENABLE(VIDEO)
    Vector<String> mediaControlsStyleSheets(const HTMLMediaElement&) override;
    Vector<String, 2> mediaControlsScripts() override;
    String mediaControlsBase64StringForIconNameAndType(const String&, const String&) override;
    String mediaControlsFormattedStringForDuration(double) override;

    String m_mediaControlsLocalizedStringsScript;
    String m_mediaControlsScript;
    String m_mediaControlsStyleSheet;
    RetainPtr<NSDateComponentsFormatter> m_durationFormatter;
#endif // ENABLE(VIDEO)
};

}
