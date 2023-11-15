/*
    Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2008, 2010 Holger Hans Peter Freyther
    Copyright (C) 2009 Dirk Schulze <krit@webkit.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "FontCascade.h"

#include "Font.h"

#include "FontDescription.h"
#include "GlyphBuffer.h"
#include "Gradient.h"
#include "GraphicsTypes.h"
#include "GraphicsContextQt.h"
#include "GraphicsContextStateSaver.h"
#include "NotImplemented.h"
#include "Pattern.h"
#include "ShadowBlur.h"
#include "TextRun.h"

#include <QBrush>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QTextLayout>
#include <qalgorithms.h>

#include <limits.h>
#include <unicode/uchar.h>
//#include <QDebug>

namespace WebCore {

template <typename CharacterType>
static inline String toNormalizedQStringImpl(const CharacterType* characters, unsigned length)
{
    QString normalized;
    normalized.reserve(length);

    for (unsigned i = 0; i < length; ++i)
        normalized.append(QChar(FontCascade::normalizeSpaces(characters[i])));

    return normalized;
}

static const QString toNormalizedQString(const TextRun& run)
{
    return run.is8Bit()
        ? toNormalizedQStringImpl(run.characters8(), run.length())
        : toNormalizedQStringImpl(run.characters16(), run.length());
}

static QTextLine setupLayout(QTextLayout* layout, const TextRun& style)
{
    int flags = style.rtl() ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight;
    if (style.expansion())
        flags |= Qt::TextJustificationForced;
    layout->setFlags(flags);
    layout->beginLayout();
    QTextLine line = layout->createLine();
    line.setLineWidth(INT_MAX/256);
    if (style.expansion())
        line.setLineWidth(line.naturalTextWidth() + style.expansion());
    layout->endLayout();
    return line;
}

static QPen fillPenForContext(GraphicsContext& ctx)
{
    // QTFIXME: QBrush should set a transform, probably
    if (ctx.fillGradient())
        return QPen(ctx.fillGradient()->createBrush(), 0);

    if (ctx.fillPattern()) {
        return QPen(QBrush(ctx.fillPattern()->createPlatformPattern()), 0);
    }

    return QPen(QColor(ctx.fillColor()), 0);
}

static QPen strokePenForContext(GraphicsContext& ctx)
{
    // QTFIXME: QBrush should set a transform, probably
    if (ctx.strokeGradient())
        return QPen(ctx.strokeGradient()->createBrush(), ctx.strokeThickness());

    if (ctx.strokePattern()) {
        QBrush brush(ctx.strokePattern()->createPlatformPattern());
        return QPen(brush, ctx.strokeThickness());
    }

    return QPen(QColor(ctx.strokeColor()), ctx.strokeThickness());
}

static QPainterPath pathForGlyphs(const QGlyphRun& glyphRun, const QPointF& offset)
{
    QPainterPath path;
    const QRawFont rawFont(glyphRun.rawFont());
    const QVector<quint32> glyphIndices = glyphRun.glyphIndexes();
    const QVector<QPointF> positions = glyphRun.positions();
    for (int i = 0; i < glyphIndices.size(); ++i) {
        QPainterPath glyphPath = rawFont.pathForGlyph(glyphIndices.at(i));
        glyphPath.translate(positions.at(i) + offset);
        path.addPath(glyphPath);
    }
    return path;
}

static void drawQtGlyphRun(GraphicsContext& context, const QGlyphRun& qtGlyphRun, const QPointF& point, qreal baseLineOffset)
{
    QPainter* painter = context.platformContext()->painter();

    QPainterPath textStrokePath;
    if (context.textDrawingMode() & TextDrawingMode::Stroke)
        textStrokePath = pathForGlyphs(qtGlyphRun, point);

    if (context.hasDropShadow()) {
        const GraphicsContextState& state = context.state();
        if (context.mustUseShadowBlur()) {
            const auto shadow = context.dropShadow();
            ASSERT(shadow);
            ShadowBlur contextShadow(*shadow, context.shadowsIgnoreTransforms());
            const qreal width = qtGlyphRun.boundingRect().width();
            const QRawFont& font = qtGlyphRun.rawFont();
            const qreal height = font.ascent() + font.descent();
            const QRectF boundingRect(point.x(), point.y() - font.ascent() + baseLineOffset, width, height);

            contextShadow.drawShadowLayer(context.getCTM(), context.clipBounds(), boundingRect,
                [state, point, qtGlyphRun, textStrokePath](GraphicsContext& shadowContext)
                {
                    QPainter* shadowPainter = shadowContext.platformContext()->painter();
                    shadowPainter->setPen(QColor(0, 0, 0));

                    if (shadowContext.textDrawingMode() & TextDrawingMode::Fill)
                        shadowPainter->drawGlyphRun(point, qtGlyphRun);
                    else if (shadowContext.textDrawingMode() & TextDrawingMode::Stroke)
                        shadowPainter->strokePath(textStrokePath, shadowPainter->pen());
                },
                [&context](ImageBuffer& layerImage, const FloatPoint& layerOrigin, const FloatSize& layerSize)
                {
                    GraphicsContextStateSaver shadowStateSaver(context);
                    context.clearDropShadow();
                    context.drawImageBuffer(layerImage, FloatRect(roundedIntPoint(layerOrigin), layerSize), FloatRect(FloatPoint(), layerSize), ImagePaintingOptions { context.compositeOperation() });
                });
        } else {
            const auto shadow = context.dropShadow();
            ASSERT(shadow);
            QPen previousPen = painter->pen();
            painter->setPen(shadow->color);
            const QPointF offset(shadow->offset.width(), shadow->offset.height());
            painter->translate(offset);
            if (context.textDrawingMode() & TextDrawingMode::Fill)
                painter->drawGlyphRun(point, qtGlyphRun);
            else if (context.textDrawingMode() & TextDrawingMode::Stroke)
                painter->strokePath(textStrokePath, painter->pen());
            painter->translate(-offset);
            painter->setPen(previousPen);
        }
    }

    if (context.textDrawingMode() & TextDrawingMode::Stroke)
        painter->strokePath(textStrokePath, strokePenForContext(context));

    if (context.textDrawingMode() & TextDrawingMode::Fill) {
        QPen previousPen = painter->pen();
        painter->setPen(fillPenForContext(context));
        painter->drawGlyphRun(point, qtGlyphRun);
        painter->setPen(previousPen);
    }
}

void FontCascade::drawComplexText(GraphicsContext& ctx, const TextRun& run, const FloatPoint& point, int from, int to) const
{
    QString string = toNormalizedQString(run);

    QTextLayout layout(string);
    layout.setRawFont(rawFont());
    initFormatForTextLayout(&layout, run);
    QTextLine line = setupLayout(&layout, run);
    const QPointF adjustedPoint(point.x(), point.y() - line.ascent());

    QList<QGlyphRun> runs = line.glyphRuns(from, to - from);
    Q_FOREACH(QGlyphRun glyphRun, runs)
        drawQtGlyphRun(ctx, glyphRun, adjustedPoint, line.ascent());
}

#if 0
float FontCascade::floatWidthForComplexText(const TextRun& run, HashSet<const Font*>*, GlyphOverflow*) const
{
    if (!primaryFont().platformData().size())
        return 0;

    if (!run.length())
        return 0;

    if (run.length() == 1 && treatAsSpace(run[0]))
        return primaryFont().spaceWidth() + run.expansion();
    QString string = toNormalizedQString(run);

    QTextLayout layout(string);
    layout.setRawFont(rawFont());
    initFormatForTextLayout(&layout, run);
    QTextLine line = setupLayout(&layout, run);
    float x1 = line.cursorToX(0);
    float x2 = line.cursorToX(run.length());
    float width = qAbs(x2 - x1);

    return width + run.expansion();
}

int FontCascade::offsetForPositionForComplexText(const TextRun& run, float position, bool) const
{
    QString string = toNormalizedQString(run);

    QTextLayout layout(string);
    layout.setRawFont(rawFont());
    initFormatForTextLayout(&layout, run);
    QTextLine line = setupLayout(&layout, run);
    return line.xToCursor(position);
}

void FontCascade::adjustSelectionRectForComplexText(const TextRun& run, LayoutRect& selectionRect, unsigned from, unsigned to) const
{
    QString string = toNormalizedQString(run);

    QTextLayout layout(string);
    layout.setRawFont(rawFont());
    initFormatForTextLayout(&layout, run);
    QTextLine line = setupLayout(&layout, run);

    float x1 = line.cursorToX(from);
    float x2 = line.cursorToX(to);
    if (x2 < x1)
        qSwap(x1, x2);

    selectionRect.move(x1, 0);
    selectionRect.setWidth(x2 - x1);
}
#endif

void FontCascade::initFormatForTextLayout(QTextLayout* layout, const TextRun& run) const
{
    QTextLayout::FormatRange range;
    // WebCore expects word-spacing to be ignored on leading spaces contrary to what Qt does.
    // To avoid word-spacing on any leading spaces, we exclude them from FormatRange which
    // word-spacing along with other options would be applied to. This is safe since the other
    // formatting options does not affect spaces.
    unsigned length = run.length();
    for (range.start = 0; range.start < length && treatAsSpace(run[range.start]); ++range.start) { }
    range.length = length - range.start;

    if (m_wordSpacing && !run.spacingDisabled())
        range.format.setFontWordSpacing(m_wordSpacing);
    if (m_letterSpacing && !run.spacingDisabled())
        range.format.setFontLetterSpacing(m_letterSpacing);
    if (enableKerning())
        range.format.setFontKerning(true);
    if (isSmallCaps())
        range.format.setFontCapitalization(QFont::SmallCaps);

    if (range.format.propertyCount() && range.length) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        layout->setFormats(QVector<QTextLayout::FormatRange>() << range);
#else
        layout->setAdditionalFormats(QList<QTextLayout::FormatRange>() << range);
#endif
    }
}

//bool FontCascade::canReturnFallbackFontsForComplexText()
//{
//    return false;
//}

void FontCascade::drawGlyphs(GraphicsContext& context, const Font& font, const GlyphBufferGlyph* glyphs, const GlyphBufferAdvance* advances, unsigned numGlyphs, const FloatPoint& point, FontSmoothingMode)
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    if (!font.platformData().size())
        return;

    if (context.paintingDisabled())
        return;

    bool shouldFill = context.textDrawingMode().contains(TextDrawingMode::Fill);
    bool shouldStroke = context.textDrawingMode().contains(TextDrawingMode::Stroke);

    if (!shouldFill && !shouldStroke)
        return;

    QVector<quint32> glyphIndexes;
    QVector<QPointF> positions;

    glyphIndexes.reserve(numGlyphs);
    positions.reserve(numGlyphs);
    const QRawFont& rawFont = font.getQtRawFont();

    float width = 0;

    for (int i = 0; i < numGlyphs; ++i) {
        Glyph glyph = glyphs[i];
        float advance = advances[i].x();
        if (!glyph)
            continue;
        glyphIndexes.append(glyph);
        positions.append(QPointF(width, 0));
        width += advance;
    }

    QGlyphRun qtGlyphs;
    qtGlyphs.setGlyphIndexes(glyphIndexes);
    qtGlyphs.setPositions(positions);
    qtGlyphs.setRawFont(rawFont);

    drawQtGlyphRun(context, qtGlyphs, point, /* baselineOffset = */0);
}


//bool FontCascade::canExpandAroundIdeographsInComplexText()
//{
//    return false;
//}

QFont FontCascade::syntheticFont() const
{
    QRawFont rawFont(primaryFont().getQtRawFont());
    QFont f(rawFont.familyName());
    if (rawFont.pixelSize())
        f.setPointSizeF(rawFont.pixelSize());
    f.setWeight(rawFont.weight());
    f.setStyle(rawFont.style());
    if (m_letterSpacing)
        f.setLetterSpacing(QFont::AbsoluteSpacing, m_letterSpacing);
    if (m_wordSpacing)
        f.setWordSpacing(m_wordSpacing);
    return f;
}


QRawFont FontCascade::rawFont() const
{
    return primaryFont().getQtRawFont();
}

}
