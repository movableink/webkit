/*
 * This file is part of the theme implementation for form controls in WebCore.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef RenderThemeQt_h
#define RenderThemeQt_h

#include "QStyleFacade.h"
#include "StyleResolver.h"
#include "RenderTheme.h"

#include <QBrush>
#include <QPalette>
#include <QSharedPointer>
#include <QString>

#include <type_traits>

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

namespace WebCore {

class RenderProgress;
class RenderStyle;
class HTMLMediaElement;
class StylePainter;
class ScrollbarTheme;

using QtThemeFactoryFunction = std::add_pointer_t<RenderTheme&()>;

class RenderThemeQt : public RenderTheme {

public:
    RenderThemeQt(Page*);

    static void setCustomTheme(QtThemeFactoryFunction factory, ScrollbarTheme* customScrollbarTheme);
    static ScrollbarTheme* customScrollbarTheme();

    String extraDefaultStyleSheet() override;

    bool supportsHover(const RenderStyle&) const override;
    bool supportsFocusRing(const RenderStyle&) const override;

    int baselinePosition(const RenderBox&) const override;

    // A method asking if the control changes its tint when the window has focus or not.
    bool controlSupportsTints(const RenderObject&) const override;

    // A general method asking if any control tinting is supported at all.
    bool supportsControlTints() const override;

    void adjustRepaintRect(const RenderObject&, FloatRect&) override;

    // The platform selection color.
    Color platformActiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformInactiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformActiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformInactiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const override;

    Color platformFocusRingColor(OptionSet<StyleColorOptions>) const override;

    Color systemColor(CSSValueID, OptionSet<StyleColorOptions>) const override;

    int minimumMenuListSize(const RenderStyle&) const override;

    void adjustSliderThumbSize(RenderStyle&, const Element*) const override;

#if ENABLE(DATALIST_ELEMENT)
    IntSize sliderTickSize() const override;
    int sliderTickOffsetFromTrackCenter() const override;
#endif

    std::optional<Seconds> caretBlinkInterval() const override;

    bool isControlStyled(const RenderStyle&, const RenderStyle&) const override;

#if 0 //ENABLE(VIDEO)
    virtual String extraMediaControlsStyleSheet();
#endif
#if ENABLE(VIDEO)
    String mediaControlsStyleSheet() override;
    Vector<String, 2> mediaControlsScripts() override;
#endif

protected:
    bool paintCheckbox(const RenderObject&, const PaintInfo&, const FloatRect&) override;
    bool paintRadio(const RenderObject&, const PaintInfo&, const FloatRect&) override;

    void adjustTextFieldStyle(RenderStyle&, const Element*) const override;

    bool paintTextArea(const RenderObject&, const PaintInfo&, const FloatRect&) override;
    void adjustTextAreaStyle(RenderStyle&, const Element*) const override;

    void adjustMenuListStyle(RenderStyle&, const Element*) const override;

    void adjustMenuListButtonStyle(RenderStyle&, const Element*) const override;

    void adjustProgressBarStyle(RenderStyle&, const Element*) const override;
    // Returns the repeat interval of the animation for the progress bar.
    Seconds animationRepeatIntervalForProgressBar(const RenderProgress&) const override;

    void adjustSliderTrackStyle(RenderStyle&, const Element*) const override;

    void adjustSliderThumbStyle(RenderStyle&, const Element*) const override;

    bool paintSearchField(const RenderObject&, const PaintInfo&, const IntRect&) override;
    void adjustSearchFieldStyle(RenderStyle&, const Element*) const override;

    void adjustSearchFieldCancelButtonStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldCancelButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldDecorationPartStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldDecorationPart(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldResultsDecorationPartStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldResultsDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&) override;

    String m_mediaControlsStyleSheet;

#if 0 //ENABLE(VIDEO)
    virtual bool paintMediaFullscreenButton(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaPlayButton(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaMuteButton(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSeekBackButton(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSeekForwardButton(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaCurrentTime(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&);
    virtual String formatMediaControlsCurrentTime(float currentTime, float duration) const override;
    virtual String formatMediaControlsRemainingTime(float currentTime, float duration) const override;
    virtual bool hasOwnDisabledStateHandlingFor(StyleAppearance) const { return true; }

    void paintMediaBackground(QPainter*, const IntRect&) const;
    double mediaControlsBaselineOpacity() const;
    QColor getMediaControlForegroundColor(const RenderObject& = 0) const;
#endif
    virtual void computeSizeBasedOnStyle(RenderStyle&) const = 0;

    String fileListNameForWidth(const FileList*, const FontCascade&, int width, bool multipleFilesAllowed) const override;

    virtual QRect inflateButtonRect(const QRect& originalRect) const;
    virtual QRectF inflateButtonRect(const QRectF& originalRect) const;
    virtual void computeControlRect(QStyleFacade::ButtonType, QRect& originalRect) const;
    virtual void computeControlRect(QStyleFacade::ButtonType, FloatRect& originalRect) const;

    virtual void setPopupPadding(RenderStyle&) const = 0;

    virtual QSharedPointer<StylePainter> getStylePainter(const PaintInfo&) = 0;

    bool supportsFocus(StyleAppearance) const;

//    IntRect convertToPaintingRect(const RenderObject& inputRenderer, const RenderObject& partRenderer, IntRect partRect, const IntRect& localOffset) const;

    virtual QPalette colorPalette() const;

    Page* m_page;

    QString m_buttonFontFamily;

};

class StylePainter {
public:
    virtual ~StylePainter();

    bool isValid() const { return painter; }

    QPainter* painter;

protected:
    StylePainter(GraphicsContext&);

private:
    QBrush m_previousBrush;
    bool m_previousAntialiasing;

};

}

#endif // RenderThemeQt_h
