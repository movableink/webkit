/*
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2006 Allan Sandfeld Jensen <sandfeld@kde.org>
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2008 Dirk Schulze <vbs85@gmx.de>
 * Copyright (C) 2010, 2011 Sencha, Inc.
 * Copyright (C) 2011 Andreas Kling <kling@webkit.org>
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "GraphicsContextQt.h"

#if OS(WINDOWS)
#include <windows.h>
#endif

#include "AffineTransform.h"
#include "Color.h"
#include "DisplayListRecorder.h"
#include "FloatConversion.h"
#include "FontCascade.h"
#include "Gradient.h"
#include "GraphicsContextStateSaver.h"
#include "ImageBuffer.h"
#include "NativeImageQt.h"
#include "Path.h"
#include "Pattern.h"
#include "ShadowBlur.h"
#include "TransformationMatrix.h"
#include "TransparencyLayer.h"

#include <QBrush>
#include <QGradient>
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPixmap>
#include <QPixmapCache>
#include <QPolygonF>
#include <QStack>
#include <QUrl>
#include <QVector>
#include <private/qhexstring_p.h>
#include <private/qpdf_p.h>
#include <wtf/MathExtras.h>
#include <wtf/URL.h>

#if OS(WINDOWS)
QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP, int hbitmapFormat = 0);
QT_END_NAMESPACE

enum HBitmapFormat {
    HBitmapNoAlpha,
    HBitmapPremultipliedAlpha,
    HBitmapAlpha
};
#endif

namespace WebCore {

static inline QPainter::CompositionMode toQtCompositionMode(CompositeOperator op)
{
    switch (op) {
    case CompositeOperator::Clear:
        return QPainter::CompositionMode_Clear;
    case CompositeOperator::Copy:
        return QPainter::CompositionMode_Source;
    case CompositeOperator::SourceOver:
        return QPainter::CompositionMode_SourceOver;
    case CompositeOperator::SourceIn:
        return QPainter::CompositionMode_SourceIn;
    case CompositeOperator::SourceOut:
        return QPainter::CompositionMode_SourceOut;
    case CompositeOperator::SourceAtop:
        return QPainter::CompositionMode_SourceAtop;
    case CompositeOperator::DestinationOver:
        return QPainter::CompositionMode_DestinationOver;
    case CompositeOperator::DestinationIn:
        return QPainter::CompositionMode_DestinationIn;
    case CompositeOperator::DestinationOut:
        return QPainter::CompositionMode_DestinationOut;
    case CompositeOperator::DestinationAtop:
        return QPainter::CompositionMode_DestinationAtop;
    case CompositeOperator::XOR:
        return QPainter::CompositionMode_Xor;
    case CompositeOperator::PlusDarker:
        // there is no exact match, but this is the closest
        return QPainter::CompositionMode_Darken;
    case CompositeOperator::PlusLighter:
        return QPainter::CompositionMode_Plus;
    case CompositeOperator::Difference:
        return QPainter::CompositionMode_Difference;
    default:
        ASSERT_NOT_REACHED();
    }

    return QPainter::CompositionMode_SourceOver;
}

static inline QPainter::CompositionMode toQtCompositionMode(BlendMode op)
{
    switch (op) {
    case BlendMode::Normal:
        return QPainter::CompositionMode_SourceOver;
    case BlendMode::Multiply:
        return QPainter::CompositionMode_Multiply;
    case BlendMode::Screen:
        return QPainter::CompositionMode_Screen;
    case BlendMode::Overlay:
        return QPainter::CompositionMode_Overlay;
    case BlendMode::Darken:
        return QPainter::CompositionMode_Darken;
    case BlendMode::Lighten:
        return QPainter::CompositionMode_Lighten;
    case BlendMode::ColorDodge:
        return QPainter::CompositionMode_ColorDodge;
    case BlendMode::ColorBurn:
        return QPainter::CompositionMode_ColorBurn;
    case BlendMode::HardLight:
        return QPainter::CompositionMode_HardLight;
    case BlendMode::SoftLight:
        return QPainter::CompositionMode_SoftLight;
    case BlendMode::Difference:
        return QPainter::CompositionMode_Difference;
    case BlendMode::Exclusion:
        return QPainter::CompositionMode_Exclusion;
    case BlendMode::PlusLighter:
        return QPainter::CompositionMode_Plus;
    case BlendMode::PlusDarker:
        // there is no exact match, but this is the closest
        return QPainter::CompositionMode_Darken;
    case BlendMode::Hue:
    case BlendMode::Saturation:
    case BlendMode::Color:
    case BlendMode::Luminosity:
        // Not supported.
        break;
    default:
        ASSERT_NOT_REACHED();
    }

    return QPainter::CompositionMode_SourceOver;
}

static inline Qt::PenCapStyle toQtLineCap(LineCap lc)
{
    switch (lc) {
    case LineCap::Butt:
        return Qt::FlatCap;
    case LineCap::Round:
        return Qt::RoundCap;
    case LineCap::Square:
        return Qt::SquareCap;
    default:
        ASSERT_NOT_REACHED();
    }

    return Qt::FlatCap;
}

static inline Qt::PenJoinStyle toQtLineJoin(LineJoin lj)
{
    switch (lj) {
    case LineJoin::Miter:
        return Qt::SvgMiterJoin;
    case LineJoin::Round:
        return Qt::RoundJoin;
    case LineJoin::Bevel:
        return Qt::BevelJoin;
    default:
        ASSERT_NOT_REACHED();
    }

    return Qt::SvgMiterJoin;
}

static Qt::PenStyle toQPenStyle(StrokeStyle style)
{
    switch (style) {
    case StrokeStyle::NoStroke:
        return Qt::NoPen;
        break;
    case StrokeStyle::SolidStroke:
    case StrokeStyle::DoubleStroke:
    case StrokeStyle::WavyStroke:
        return Qt::SolidLine;
        break;
    case StrokeStyle::DottedStroke:
        return Qt::DotLine;
        break;
    case StrokeStyle::DashedStroke:
        return Qt::DashLine;
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    return Qt::NoPen;
}

static inline Qt::FillRule toQtFillRule(WindRule rule)
{
    switch (rule) {
    case WindRule::EvenOdd:
        return Qt::OddEvenFill;
    case WindRule::NonZero:
        return Qt::WindingFill;
    default:
        ASSERT_NOT_REACHED();
    }
    return Qt::OddEvenFill;
}

class GraphicsContextPlatformPrivate {
    WTF_MAKE_NONCOPYABLE(GraphicsContextPlatformPrivate); WTF_MAKE_FAST_ALLOCATED;
public:
    GraphicsContextPlatformPrivate(QPainter*, const QColor& initialSolidColor);
    ~GraphicsContextPlatformPrivate();

    inline QPainter* p() const
    {
        if (layers.isEmpty())
            return painter;
        return &layers.top()->painter;
    }

    bool antiAliasingForRectsAndLines;

    QStack<TransparencyLayer*> layers;
    // Counting real layers. Required by isInTransparencyLayer() calls
    // For example, layers with valid alphaMask are not real layers
    int layerCount;

    // reuse this brush for solid color (to prevent expensive QBrush construction)
    QBrush solidColor;

    bool initialSmoothPixmapTransformHint;

    QRectF clipBoundingRect() const
    {
        return p()->clipBoundingRect();
    }

    void takeOwnershipOfPlatformContext() { platformContextIsOwned = true; }

private:
    QPainter* painter;
    bool platformContextIsOwned;
};

GraphicsContextPlatformPrivate::GraphicsContextPlatformPrivate(QPainter* p, const QColor& initialSolidColor)
    : antiAliasingForRectsAndLines(false)
    , layerCount(0)
    , solidColor(initialSolidColor)
    , initialSmoothPixmapTransformHint(false)
    , painter(p)
    , platformContextIsOwned(false)
{
    if (!painter)
        return;

    // Use the default the QPainter was constructed with.
    antiAliasingForRectsAndLines = painter->testRenderHint(QPainter::Antialiasing);

    // Used for default image interpolation quality.
    initialSmoothPixmapTransformHint = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    painter->setRenderHint(QPainter::Antialiasing, true);

}

GraphicsContextPlatformPrivate::~GraphicsContextPlatformPrivate()
{
    if (!platformContextIsOwned)
        return;

    QPaintDevice* device = painter->device();
    painter->end();
    delete painter;
    delete device;
}

GraphicsContextQt::GraphicsContextQt(QPainter* p)
{
    m_data = new GraphicsContextPlatformPrivate(p, fillColor());

    // solidColor is initialized with the fillColor().
    p->setBrush(m_data->solidColor);

    QPen pen(p->pen());
    pen.setColor(strokeColor());
    pen.setJoinStyle(toQtLineJoin(LineJoin::Miter));
    pen.setCapStyle(Qt::FlatCap);
    p->setPen(pen);
}

GraphicsContextQt::~GraphicsContextQt()
{
    if (m_data) {
        while (!m_data->layers.isEmpty())
            endTransparencyLayer();
    }

    delete m_data;
}

bool GraphicsContextQt::hasPlatformContext() const
{
    return true;
}

GraphicsContextQt* GraphicsContextQt::platformContext() const
{
    return const_cast<GraphicsContextQt*>(this);
}

QPainter* GraphicsContextQt::painter() const
{
    return m_data->p();
}

AffineTransform GraphicsContextQt::getCTM(IncludeDeviceScale includeScale) const
{
    if (paintingDisabled())
        return AffineTransform();

    const QTransform& matrix = (includeScale == DefinitelyIncludeDeviceScale)
        ? painter()->combinedTransform()
        : painter()->worldTransform();
    return AffineTransform(matrix.m11(), matrix.m12(), matrix.m21(),
                           matrix.m22(), matrix.dx(), matrix.dy());
}

void GraphicsContextQt::save(GraphicsContextState::Purpose purpose)
{
    GraphicsContext::save(purpose);
    if (!m_data->layers.isEmpty() && !m_data->layers.top()->alphaMask.isNull())
        ++m_data->layers.top()->saveCounter;
    m_data->p()->save();
}

void GraphicsContextQt::restore(GraphicsContextState::Purpose purpose)
{
    if (!stackSize())
        return;

    GraphicsContext::restore(purpose);
    if (!m_data->layers.isEmpty() && !m_data->layers.top()->alphaMask.isNull())
        if (!--m_data->layers.top()->saveCounter)
            popTransparencyLayerInternal();

    m_data->p()->restore();
}

// Draws a filled rectangle with a stroked border.
// This is only used to draw borders (real fill is done via fillRect), and
// thus it must not cast any shadow.
void GraphicsContextQt::drawRect(const FloatRect& rect, float borderThickness)
{
    ASSERT(!rect.isEmpty());

    QPainter* p = m_data->p();
    const bool antiAlias = p->testRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, m_data->antiAliasingForRectsAndLines);

    // strokeThickness() is disregarded
    QPen oldPen(p->pen());
    QPen newPen(oldPen);
    newPen.setWidthF(borderThickness);
    p->setPen(newPen);

    p->drawRect(rect);

    p->setPen(oldPen);
    p->setRenderHint(QPainter::Antialiasing, antiAlias);
}

// This is only used to draw borders.
// Must not cast any shadow.
void GraphicsContextQt::drawLine(const FloatPoint& point1, const FloatPoint& point2)
{
    if (paintingDisabled())
        return;

    if (strokeStyle() == StrokeStyle::NoStroke)
        return;

    const Color& strokeColor = this->strokeColor();
    float thickness = strokeThickness();
    bool isVerticalLine = (point1.x() + thickness == point2.x());
    float strokeWidth = isVerticalLine ? point2.y() - point1.y() : point2.x() - point1.x();
    if (!thickness || !strokeWidth)
        return;

    QPainter* p = painter();
    const bool savedAntiAlias = p->testRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, m_data->antiAliasingForRectsAndLines);

    StrokeStyle strokeStyle = this->strokeStyle();
    float cornerWidth = 0;
    bool drawsDashedLine = strokeStyle == StrokeStyle::DottedStroke || strokeStyle == StrokeStyle::DashedStroke;

    if (drawsDashedLine) {
        p->save();
        // Figure out end points to ensure we always paint corners.
        cornerWidth = strokeStyle == StrokeStyle::DottedStroke ? thickness : std::min(2 * thickness, std::max(thickness, strokeWidth / 3));

        if (isVerticalLine) {
            p->fillRect(FloatRect(point1.x(), point1.y(), thickness, cornerWidth), strokeColor);
            p->fillRect(FloatRect(point1.x(), point2.y() - cornerWidth, thickness, cornerWidth),  strokeColor);
        } else {
            p->fillRect(FloatRect(point1.x(), point1.y(), cornerWidth, thickness), strokeColor);
            p->fillRect(FloatRect(point2.x() - cornerWidth, point1.y(), cornerWidth, thickness), strokeColor);
        }

        strokeWidth -= 2 * cornerWidth;
        float patternWidth = strokeStyle == StrokeStyle::DottedStroke ? thickness : std::min(3 * thickness, std::max(thickness, strokeWidth / 3));
        // Check if corner drawing sufficiently covers the line.
        if (strokeWidth <= patternWidth + 1) {
            p->restore();
            return;
        }

        // Pattern starts with full fill and ends with the empty fill.
        // 1. Let's start with the empty phase after the corner.
        // 2. Check if we've got odd or even number of patterns and whether they fully cover the line.
        // 3. In case of even number of patterns and/or remainder, move the pattern start position
        // so that the pattern is balanced between the corners.
        float patternOffset = patternWidth;
        int numberOfSegments = std::floor(strokeWidth / patternWidth);
        bool oddNumberOfSegments = numberOfSegments % 2;
        float remainder = strokeWidth - (numberOfSegments * patternWidth);
        if (oddNumberOfSegments && remainder)
            patternOffset -= remainder / 2.f;
        else if (!oddNumberOfSegments) {
            if (remainder)
                patternOffset += patternOffset - (patternWidth + remainder) / 2.f;
            else
                patternOffset += patternWidth / 2.f;
        }

        Qt::PenCapStyle capStyle = Qt::FlatCap;
        QVector<qreal> dashes { patternWidth / thickness, patternWidth / thickness };

        QPen pen = p->pen();
        pen.setCapStyle(capStyle);
        pen.setDashPattern(dashes);
        pen.setDashOffset(patternOffset / thickness);
        p->setPen(pen);
    }

    FloatPoint p1 = point1;
    FloatPoint p2 = point2;
    // Center line and cut off corners for pattern patining.
    if (isVerticalLine) {
        float centerOffset = (p2.x() - p1.x()) / 2;
        p1.move(centerOffset, cornerWidth);
        p2.move(-centerOffset, -cornerWidth);
    } else {
        float centerOffset = (p2.y() - p1.y()) / 2;
        p1.move(cornerWidth, centerOffset);
        p2.move(-cornerWidth, -centerOffset);
    }

    p->drawLine(p1, p2);

    if (drawsDashedLine)
        p->restore();

    p->setRenderHint(QPainter::Antialiasing, savedAntiAlias);
}

// This method is only used to draw the little circles used in lists.
void GraphicsContextQt::drawEllipse(const FloatRect& rect)
{
    m_data->p()->drawEllipse(rect);
}

void GraphicsContextQt::drawPattern(NativeImage& nativeImage, const FloatRect &destRect, const FloatRect& tileRect, const AffineTransform& patternTransform,
    const FloatPoint& phase, const FloatSize& spacing, ImagePaintingOptions options)
{
    if (!patternTransform.isInvertible())
        return;

    QImage image = nativeImage.platformImage();

#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    FloatRect tileRectAdjusted = adjustSourceRectForDownSampling(tileRect, framePixmap->size());
#else
    FloatRect tileRectAdjusted = tileRect;
#endif

    // Qt interprets 0 width/height as full width/height so just short circuit.
    QRectF dr = QRectF(destRect).normalized();
    QRect tr = QRectF(tileRectAdjusted).toRect().normalized();
    if (!dr.width() || !dr.height() || !tr.width() || !tr.height())
        return;

    QImage qImage;
    if (tr.x() || tr.y() || tr.width() != image.width() || tr.height() != image.height())
        qImage = image.copy(tr);
    else
        qImage = image; // May do a deep copy if frameImage is being painted to

    QPoint trTopLeft = tr.topLeft();

    CompositeOperator previousOperator = compositeOperation();

    setCompositeOperation(!qImage.hasAlphaChannel() && options.compositeOperator() == CompositeOperator::SourceOver ? CompositeOperator::Copy : options.compositeOperator());

    QPainter* p = painter();
    QTransform transform(patternTransform);

    QTransform combinedTransform = p->combinedTransform();
    QTransform targetScaleTransform = QTransform::fromScale(combinedTransform.m11(), combinedTransform.m22());
    QTransform transformWithTargetScale = transform * targetScaleTransform;

    // If this would draw more than one scaled tile, we scale the image first and then use the result to draw.
    if (transformWithTargetScale.type() == QTransform::TxScale) {
        QRectF tileRectInTargetCoords = (transformWithTargetScale * QTransform().translate(phase.x(), phase.y())).mapRect(tr);

        bool tileWillBePaintedOnlyOnce = tileRectInTargetCoords.contains(dr);
        if (!tileWillBePaintedOnlyOnce) {
            QSizeF scaledSize(qreal(qImage.width()) * transformWithTargetScale.m11(), qreal(qImage.height()) * transformWithTargetScale.m22());
            QImage scaledImage;
            if (qImage.hasAlphaChannel()) {
                scaledImage = QImage(scaledSize.toSize(), NativeImageQt::defaultFormatForAlphaEnabledImages());
                scaledImage.fill(Qt::transparent);
            } else
                scaledImage = QImage(scaledSize.toSize(), NativeImageQt::defaultFormatForOpaqueImages());

            {
                QPainter painter(&scaledImage);
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.setRenderHints(p->renderHints());
                painter.drawImage(QRect(0, 0, scaledImage.width(), scaledImage.height()), qImage);
            }
            qImage = scaledImage;
            trTopLeft = transformWithTargetScale.map(trTopLeft);
            transform = targetScaleTransform.inverted().translate(transform.dx(), transform.dy());
        }
    }

    /* Translate the coordinates as phase is not in world matrix coordinate space but the tile rect origin is. */
    transform *= QTransform().translate(phase.x(), phase.y());
    transform.translate(trTopLeft.x(), trTopLeft.y());

    QBrush b(qImage);
    b.setTransform(transform);
    p->fillRect(dr, b);

    setCompositeOperation(previousOperator);
}

void GraphicsContextQt::drawGlyphs(const Font& font, const GlyphBufferGlyph* glyphs, const GlyphBufferAdvance* advances, unsigned numGlyphs, const FloatPoint& point, FontSmoothingMode fontSmoothingMode)
{
    FontCascade::drawGlyphs(*this, font, glyphs, advances, numGlyphs, point, fontSmoothingMode);
}

/*
 FIXME: Removed in https://bugs.webkit.org/show_bug.cgi?id=153174
 Find out if we need to adjust anything

void GraphicsContextQt::drawConvexPolygon(size_t npoints, const FloatPoint* points, bool shouldAntialias)
{
    if (paintingDisabled())
        return;

    if (npoints <= 1)
        return;

    QPolygonF polygon(npoints);

    for (size_t i = 0; i < npoints; i++)
        polygon[i] = points[i];

    QPainter* p = m_data->p();

    const bool antiAlias = p->testRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, shouldAntialias);

    p->drawConvexPolygon(polygon);

    p->setRenderHint(QPainter::Antialiasing, antiAlias);
}

void GraphicsContextQt::clipConvexPolygon(size_t numPoints, const FloatPoint* points, bool antialiased)
{
    if (paintingDisabled())
        return;

    if (numPoints <= 1)
        return;

    QPainterPath path(points[0]);
    for (size_t i = 1; i < numPoints; ++i)
        path.lineTo(points[i]);
    path.setFillRule(Qt::WindingFill);

    QPainter* p = m_data->p();

    bool painterWasAntialiased = p->testRenderHint(QPainter::Antialiasing);

    if (painterWasAntialiased != antialiased)
        p->setRenderHint(QPainter::Antialiasing, antialiased);

    p->setClipPath(path, Qt::IntersectClip);

    if (painterWasAntialiased != antialiased)
        p->setRenderHint(QPainter::Antialiasing, painterWasAntialiased);
}
*/

void GraphicsContextQt::fillPath(const Path& path)
{
    QPainter* p = m_data->p();
    QPainterPath platformPath = path.platformPath();
    platformPath.setFillRule(toQtFillRule(fillRule()));

    if (hasDropShadow()) {
        const auto shadow = dropShadow();
        ASSERT(shadow);
        if (mustUseShadowBlur() || m_state.fillBrush().pattern() || m_state.fillBrush().gradient()) {
            const auto& state = m_state;
            const auto& oldBrush = p->brush();
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            contextShadow.drawShadowLayer(getCTM(), clipBounds(), platformPath.controlPointRect(),
                [&state, &platformPath, &oldBrush](GraphicsContext& shadowContext) {
                    QPainter* shadowPainter = shadowContext.platformContext()->painter();
                    if (state.fillBrush().pattern()) {
                        shadowPainter->fillPath(platformPath, QBrush(state.fillBrush().pattern()->createPlatformPattern()));
                    } else if (state.fillBrush().gradient()) {
                        QBrush brush = state.fillBrush().gradient()->createBrush();
                        brush.setTransform(shadowContext.fillGradientSpaceTransform());
                        shadowPainter->fillPath(platformPath, brush);
                    } else {
                        shadowPainter->fillPath(platformPath, oldBrush);
                    }
                },
                [this](ImageBuffer& layerImage, const FloatPoint& layerOrigin, const FloatSize& layerSize) {
                    GraphicsContextStateSaver shadowStateSaver(*this);
                    clearDropShadow();
                    drawImageBuffer(layerImage, FloatRect(roundedIntPoint(layerOrigin), layerSize), FloatRect(FloatPoint(), layerSize), ImagePaintingOptions { compositeOperation() });
                });
        } else {
            QPointF offset(shadow->offset.width(), shadow->offset.height());
            p->translate(offset);
            QColor _shadowColor = shadow->color;
            _shadowColor.setAlphaF(_shadowColor.alphaF() * p->brush().color().alphaF());
            p->fillPath(platformPath, _shadowColor);
            p->translate(-offset);
        }
    }
    if (m_state.fillBrush().pattern()) {
        p->fillPath(platformPath, QBrush(m_state.fillBrush().pattern()->createPlatformPattern()));
    } else if (m_state.fillBrush().gradient()) {
        QBrush brush = m_state.fillBrush().gradient()->createBrush();
        brush.setTransform(fillGradientSpaceTransform());
        p->fillPath(platformPath, brush);
    } else
        p->fillPath(platformPath, p->brush());
}

inline static void fillPathStroke(QPainter* painter, const QPainterPath& platformPath, const QPen& pen)
{
    if (pen.color().alphaF() < 1.0) {
        QPainterPathStroker pathStroker;
        pathStroker.setJoinStyle(pen.joinStyle());
        pathStroker.setDashOffset(pen.dashOffset());
        pathStroker.setDashPattern(pen.dashPattern());
        pathStroker.setMiterLimit(pen.miterLimit());
        pathStroker.setCapStyle(pen.capStyle());
        pathStroker.setWidth(pen.widthF());

        QPainterPath stroke = pathStroker.createStroke(platformPath);
        painter->fillPath(stroke, pen.brush());
    } else {
        painter->strokePath(platformPath, pen);
    }
}

const QImage* prescaleImageIfRequired(QPainter* painter, const QImage* image, QImage* prescaledImage, const QRectF& destRect, QRectF* srcRect)
{
    // The quality of down scaling at 0.5x and below in QPainter is not very good
    // due to using bilinear sampling, so for high quality scaling we need to
    // perform scaling ourselves.
    ASSERT(image);
    ASSERT(painter);
    if (!(painter->renderHints() & QPainter::SmoothPixmapTransform))
        return image;

    if (painter->paintEngine()->type() != QPaintEngine::Raster)
        return image;

    QTransform transform = painter->combinedTransform();

    // Prescaling transforms that does more than scale or translate is not supported.
    if (transform.type() > QTransform::TxScale)
        return image;

    QRectF transformedDst = transform.mapRect(destRect);
    // Only prescale if downscaling to 0.5x or less
    if (transformedDst.width() * 2 > srcRect->width() && transformedDst.height() * 2 > srcRect->height())
        return image;

    // This may not work right with subpixel positions, but that can not currently happen.
    QRect pixelSrc = srcRect->toRect();
    QSize scaledSize = transformedDst.size().toSize();

    QString key = QStringLiteral("qtwebkit_prescaled_")
        % HexString<qint64>(image->cacheKey())
        % HexString<int>(pixelSrc.x()) % HexString<int>(pixelSrc.y())
        % HexString<int>(pixelSrc.width()) % HexString<int>(pixelSrc.height())
        % HexString<int>(scaledSize.width()) % HexString<int>(scaledSize.height());

    QPixmap buffer;

    if (!QPixmapCache::find(key, &buffer)) {
        if (pixelSrc != image->rect())
            buffer = QPixmap::fromImage(image->copy(pixelSrc).scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        else
            buffer = QPixmap::fromImage(image->scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        QPixmapCache::insert(key, buffer);
    }

    *srcRect = QRectF(QPointF(), buffer.size());
    *prescaledImage = buffer.toImage();

    return prescaledImage;
}

void GraphicsContextQt::drawNativeImageInternal(NativeImage& image, const FloatSize&, const FloatRect& destRect, const FloatRect& srcRect, ImagePaintingOptions options)
{
    painter()->setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF normalizedDst = destRect.normalized();
    QRectF normalizedSrc = srcRect.normalized();

    QImage prescaledImage;
    const QImage* maybePrescaledImage = prescaleImageIfRequired(painter(), &image.platformImage(), &prescaledImage, normalizedDst, &normalizedSrc);

    CompositeOperator previousOperator = compositeOperation();
    BlendMode previousBlendMode = blendMode();
    setCompositeOperation(!image.hasAlpha() && options.compositeOperator() == CompositeOperator::SourceOver && options.blendMode() == BlendMode::Normal ? CompositeOperator::Copy : options.compositeOperator(), options.blendMode());

    if (hasDropShadow() && mustUseShadowBlur()) {
        const auto shadow = dropShadow();
        ASSERT(shadow);
        ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
        contextShadow.drawShadowLayer(getCTM(), clipBounds(), normalizedDst,
            [normalizedSrc, normalizedDst, &image](GraphicsContext& shadowContext)
            {
                QPainter* shadowPainter = shadowContext.platformContext()->painter();
                shadowPainter->drawImage(normalizedDst, image.platformImage(), normalizedSrc);
            },
            [this](ImageBuffer& layerImage, const FloatPoint& layerOrigin, const FloatSize& layerSize)
            {
                drawImageBuffer(layerImage, FloatRect(roundedIntPoint(layerOrigin), layerSize), FloatRect(FloatPoint(), layerSize), ImagePaintingOptions { compositeOperation() });
            });
    }

    painter()->drawImage(normalizedDst, *maybePrescaledImage, normalizedSrc);

    setCompositeOperation(previousOperator, previousBlendMode);
}

void GraphicsContextQt::strokePath(const Path& path)
{
    QPainter* p = m_data->p();
    QPen pen(p->pen());
    QPainterPath platformPath = path.platformPath();
    platformPath.setFillRule(toQtFillRule(fillRule()));

    if (hasDropShadow()) {
        const auto shadow = dropShadow();
        ASSERT(shadow);
        if (mustUseShadowBlur() || strokePattern() || strokeGradient())
        {
            const auto& state = m_state;
            const auto& oldPen = p->pen();
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            FloatRect boundingRect = platformPath.controlPointRect();
            boundingRect.inflate(pen.miterLimit() + pen.widthF());
            contextShadow.drawShadowLayer(getCTM(), clipBounds(), boundingRect,
                [&state, &platformPath, &oldPen](GraphicsContext& shadowContext) {
                    QPainter* shadowPainter = shadowContext.platformContext()->painter();
                    if (shadowContext.strokeGradient()) {
                        QPen shadowPen(oldPen);
                        QBrush shadowBrush = shadowContext.strokeGradient()->createBrush();
                        shadowBrush.setTransform(shadowContext.fillGradientSpaceTransform());
                        shadowPen.setBrush(shadowBrush);
                        fillPathStroke(shadowPainter, platformPath, shadowPen);
                    } else {
                        fillPathStroke(shadowPainter, platformPath, oldPen);
                    }
                },
            [this](ImageBuffer& layerImage, const FloatPoint& layerOrigin, const FloatSize& layerSize) {
                GraphicsContextStateSaver shadowStateSaver(*this);
                clearDropShadow();
                drawImageBuffer(layerImage, FloatRect(roundedIntPoint(layerOrigin), layerSize), FloatRect(FloatPoint(), layerSize), ImagePaintingOptions { compositeOperation() });
            });
        } else {
            QPointF offset(shadow->offset.width(), shadow->offset.height());
            p->translate(offset);
            QColor _shadowColor = shadow->color;
            _shadowColor.setAlphaF(_shadowColor.alphaF() * pen.color().alphaF());
            QPen shadowPen(pen);
            shadowPen.setColor(_shadowColor);
            fillPathStroke(p, platformPath, shadowPen);
            p->translate(-offset);
        }
    }

    if (strokePattern()) {
        QBrush brush = strokePattern()->createPlatformPattern();
        pen.setBrush(brush);
        fillPathStroke(p, platformPath, pen);
    } else if (strokeGradient()) {
        QBrush brush = strokeGradient()->createBrush();
        brush.setTransform(fillGradientSpaceTransform());
        pen.setBrush(brush);
        fillPathStroke(p, platformPath, pen);
    } else
        fillPathStroke(p, platformPath, pen);
}

static inline void drawRepeatPattern(QPainter* p, const Pattern& pattern, const FloatRect& rect)
{
    const QBrush brush = pattern.createPlatformPattern();
    if (brush.style() != Qt::TexturePattern)
        return;

    const bool repeatX = pattern.repeatX();
    const bool repeatY = pattern.repeatY();
    // Patterns must be painted so that the top left of the first image is anchored at
    // the origin of the coordinate space

    QRectF targetRect(rect);
    const int w = brush.texture().width();
    const int h = brush.texture().height();

    ASSERT(p);
    QRegion oldClip;
    if (p->hasClipping())
        oldClip = p->clipRegion();

    QRectF clip = targetRect;
    QRectF patternRect = brush.transform().mapRect(QRectF(0, 0, w, h));
    if (!repeatX) {
        clip.setLeft(patternRect.left());
        clip.setWidth(patternRect.width());
    }
    if (!repeatY) {
        clip.setTop(patternRect.top());
        clip.setHeight(patternRect.height());
    }
    if (!repeatX || !repeatY)
        p->setClipRect(clip);

    p->fillRect(targetRect, brush);

    if (!oldClip.isEmpty())
        p->setClipRegion(oldClip);
    else if (!repeatX || !repeatY)
        p->setClipping(false);
}

void GraphicsContextQt::fillRect(const FloatRect& rect)
{
    QPainter* p = m_data->p();
    QRectF normalizedRect = rect.normalized();

    if (m_state.fillBrush().pattern()) {
        if (hasDropShadow()) {
            const auto shadow = dropShadow();
            ASSERT(shadow);
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            const auto& fillPattern = *m_state.fillBrush().pattern();
            contextShadow.drawShadowLayer(getCTM(), clipBounds(), normalizedRect,
            [&fillPattern, &normalizedRect](GraphicsContext& shadowContext) {
                QPainter* shadowPainter = shadowContext.platformContext()->painter();
                drawRepeatPattern(shadowPainter, fillPattern, normalizedRect);
            },
            [this](ImageBuffer& layerImage, const FloatPoint& layerOrigin, const FloatSize& layerSize) {
                GraphicsContextStateSaver shadowStateSaver(*this);
                clearDropShadow();
                drawImageBuffer(layerImage, FloatRect(roundedIntPoint(layerOrigin), layerSize), FloatRect(FloatPoint(), layerSize), ImagePaintingOptions { compositeOperation() });
            });
        }
        drawRepeatPattern(p, *m_state.fillBrush().pattern(), normalizedRect);
    } else if (m_state.fillBrush().gradient()) {
        QBrush brush(m_state.fillBrush().gradient()->createBrush());
        brush.setTransform(fillGradientSpaceTransform());

        if (hasDropShadow()) {
            const auto shadow = dropShadow();
            ASSERT(shadow);
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            contextShadow.drawShadowLayer(getCTM(), clipBounds(), normalizedRect,
            [&brush, &normalizedRect](GraphicsContext& shadowContext) {
                QPainter* shadowPainter = shadowContext.platformContext()->painter();
                shadowPainter->fillRect(normalizedRect, brush);
            },
            [this](ImageBuffer& layerImage, const FloatPoint& layerOrigin, const FloatSize& layerSize) {
                GraphicsContextStateSaver shadowStateSaver(*this);
                clearDropShadow();
                drawImageBuffer(layerImage, FloatRect(roundedIntPoint(layerOrigin), layerSize), FloatRect(FloatPoint(), layerSize), ImagePaintingOptions { compositeOperation() });
            });
        }
        p->fillRect(normalizedRect, brush);
    } else {
        if (hasDropShadow()) {
            const auto shadow = dropShadow();
            ASSERT(shadow);
            if (mustUseShadowBlur()) {
                ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
                // drawRectShadowWithTiling does not work with rotations, and the fallback of
                // drawing though clipToImageBuffer() produces scaling artifacts for us.
                if (!getCTM().preservesAxisAlignment()) {
                    const auto& brush = p->brush();
                    contextShadow.drawShadowLayer(getCTM(), clipBounds(), normalizedRect,
                    [&brush, &normalizedRect](GraphicsContext& shadowContext) {
                        QPainter* shadowPainter = shadowContext.platformContext()->painter();
                        shadowPainter->fillRect(normalizedRect, brush);
                    },
                    [this](ImageBuffer& layerImage, const FloatPoint& layerOrigin, const FloatSize& layerSize) {
                        GraphicsContextStateSaver shadowStateSaver(*this);
                        clearDropShadow();
                        drawImageBuffer(layerImage, FloatRect(roundedIntPoint(layerOrigin), layerSize), FloatRect(FloatPoint(), layerSize), ImagePaintingOptions { compositeOperation() });
                    });
                } else
                    contextShadow.drawRectShadow(*this, FloatRoundedRect(rect));
            } else {
                // Solid rectangle fill with no blur shadow or transformations applied can be done
                // faster without using the shadow layer at all.
                QColor _shadowColor = shadow->color;
                _shadowColor.setAlphaF(_shadowColor.alphaF() * p->brush().color().alphaF());
                p->fillRect(normalizedRect.translated(QPointF(shadow->offset.width(), shadow->offset.height())), _shadowColor);
            }
        }

        p->fillRect(normalizedRect, p->brush());
    }
}


void GraphicsContextQt::fillRect(const FloatRect& rect, const Color& color)
{
    if (!color.isValid())
        return;

    QRectF platformRect(rect);
    QPainter* p = m_data->p();
    if (hasDropShadow()) {
        const auto shadow = dropShadow();
        ASSERT(shadow);
        if (mustUseShadowBlur()) {
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            contextShadow.drawRectShadow(*this, FloatRoundedRect(platformRect));
        } else {
            QColor _shadowColor = shadow->color;
            _shadowColor.setAlphaF(_shadowColor.alphaF() * p->brush().color().alphaF());
            p->fillRect(platformRect.translated(QPointF(shadow->offset.width(), shadow->offset.height())), _shadowColor);
        }
    }
    p->fillRect(platformRect, QColor(color));
}

void GraphicsContextQt::fillRect(const FloatRect& rect, Gradient& gradient)
{
    QRectF platformRect(rect);
    QPainter* p = m_data->p();
    if (hasDropShadow()) {
        const auto shadow = dropShadow();
        ASSERT(shadow);
        if (mustUseShadowBlur()) {
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            contextShadow.drawRectShadow(*this, FloatRoundedRect(platformRect));
        } else {
            QColor _shadowColor = shadow->color;
            _shadowColor.setAlphaF(_shadowColor.alphaF() * p->brush().color().alphaF());
            p->fillRect(platformRect.translated(QPointF(shadow->offset.width(), shadow->offset.height())), _shadowColor);
        }
    }
    gradient.fill(*this, platformRect);
}

void GraphicsContextQt::fillRoundedRectImpl(const FloatRoundedRect& rect, const Color& color)
{
    if (!color.isValid())
        return;

    Path path;
    path.addRoundedRect(rect);
    QPainter* p = m_data->p();
    if (hasDropShadow()) {
        const auto shadow = dropShadow();
        ASSERT(shadow);
        if (mustUseShadowBlur()) {
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            contextShadow.drawRectShadow(*this, rect);
        } else {
            const QPointF offset(shadow->offset.width(), shadow->offset.height());
            p->translate(offset);
            const QPainterPath platformPath = path.platformPath();
            p->fillPath(platformPath, QColor(shadow->color));
            p->translate(-offset);
        }
    }
    const QPainterPath platformPath = path.platformPath();
    p->fillPath(platformPath, QColor(color));
}

void GraphicsContextQt::fillRectWithRoundedHole(const FloatRect& rect, const FloatRoundedRect& roundedHoleRect, const Color& color)
{
    if (!color.isValid())
        return;

    Path path;
    path.addRect(rect);
    if (!roundedHoleRect.radii().isZero())
        path.addRoundedRect(roundedHoleRect);
    else
        path.addRect(roundedHoleRect.rect());

    QPainterPath platformPath = path.platformPath();
    platformPath.setFillRule(Qt::OddEvenFill);

    QPainter* p = m_data->p();
    if (hasDropShadow()) {
        const auto shadow = dropShadow();
        ASSERT(shadow);
        if (mustUseShadowBlur()) {
            ShadowBlur contextShadow(*shadow, shadowsIgnoreTransforms());
            contextShadow.drawInsetShadow(*this, rect, roundedHoleRect);
        } else {
            const QPointF offset(shadow->offset.width(), shadow->offset.height());
            p->translate(offset);
            p->fillPath(platformPath, QColor(shadow->color));
            p->translate(-offset);
        }
    }

    p->fillPath(platformPath, QColor(color));
}

void GraphicsContextQt::clipToImageBuffer(ImageBuffer& buffer, const FloatRect& destRect)
{
    auto nativeImage = nativeImageForDrawing(buffer);
    if (!nativeImage)
        return;

    GraphicsContextQt* context = platformContext();
    context->pushTransparencyLayerInternal(QRectF(destRect).toRect(), 1.0, nativeImage->platformImage());
}

void GraphicsContextQt::clip(const FloatRect& rect)
{
    m_data->p()->setClipRect(rect, Qt::IntersectClip);
}

IntRect GraphicsContextQt::clipBounds() const
{
    QPainter* p = m_data->p();
    QRectF clipRect;

    clipRect = p->transform().inverted().mapRect(p->window());

    if (p->hasClipping())
        clipRect = clipRect.intersected(m_data->clipBoundingRect());

    return enclosingIntRect(clipRect);
}

void GraphicsContextQt::clipPath(const Path& path, WindRule clipRule)
{
    QPainter* p = m_data->p();
    QPainterPath platformPath = path.platformPath();
    platformPath.setFillRule(clipRule == WindRule::EvenOdd ? Qt::OddEvenFill : Qt::WindingFill);
    p->setClipPath(platformPath, Qt::IntersectClip);
}

void GraphicsContextQt::resetClip()
{
    m_data->p()->setClipping(false);
}

void drawFocusRingForPath(QPainter* p, const QPainterPath& path, const Color& color, bool antiAliasing)
{
    const bool antiAlias = p->testRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, antiAliasing);

    const QPen oldPen = p->pen();
    const QBrush oldBrush = p->brush();

    QPen nPen = p->pen();
    nPen.setColor(color);
    p->setBrush(Qt::NoBrush);
    nPen.setStyle(Qt::DotLine);

    p->strokePath(path, nPen);
    p->setBrush(oldBrush);
    p->setPen(oldPen);

    p->setRenderHint(QPainter::Antialiasing, antiAlias);
}

void GraphicsContextQt::drawFocusRing(const Path& path, float /* outlineWidth */, const Color& color)
{
    // FIXME: Use 'offset' for something? http://webkit.org/b/49909

    if (!color.isValid())
        return;

    drawFocusRingForPath(m_data->p(), path.platformPath(), color, m_data->antiAliasingForRectsAndLines);
}

/**
 * Focus ring handling for form controls is not handled here. Qt style in
 * RenderTheme handles drawing focus on widgets which 
 * need it. It is still handled here for links.
 */
void GraphicsContextQt::drawFocusRing(const Vector<FloatRect>& rects, float outlineOffset, float outlineWidth, const Color& color)
{
    if (paintingDisabled() || !color.isValid())
        return;

    unsigned rectCount = rects.size();

    if (!rects.size())
        return;

    float radius = (outlineWidth - 1) / 2;
    QPainterPath path;
    for (unsigned i = 0; i < rectCount; ++i) {
        QRectF rect = QRectF(rects[i]).adjusted(-outlineOffset - radius, -outlineOffset - radius, outlineOffset + radius, outlineOffset + radius);
        // This is not the most efficient way to add a rect to a path, but if we don't create the tmpPath,
        // we will end up with ugly lines in between rows of text on anchors with multiple lines.
        QPainterPath tmpPath;
        tmpPath.addRoundedRect(rect, radius, radius);
        path = path.united(tmpPath);
    }
    drawFocusRingForPath(m_data->p(), path, color, m_data->antiAliasingForRectsAndLines);
}

void GraphicsContextQt::drawLineForText(const FloatRect& rect, bool printing, bool doubleUnderlines, StrokeStyle strokeStyle)
{
    Color localStrokeColor(strokeColor());

    FloatRect bounds = computeLineBoundsAndAntialiasingModeForText(rect, printing, localStrokeColor);
    bool strokeColorChanged = strokeColor() != localStrokeColor;
    bool strokeThicknessChanged = strokeThickness() != bounds.height();
    bool needSavePen = strokeColorChanged || strokeThicknessChanged;

    QPainter* p = painter();
    const bool savedAntiAlias = p->testRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, m_data->antiAliasingForRectsAndLines);

    QPen oldPen(p->pen());
    if (needSavePen) {
        QPen newPen(oldPen);
        if (strokeThicknessChanged)
            newPen.setWidthF(bounds.height());
        if (strokeColorChanged)
            newPen.setColor(localStrokeColor);
        p->setPen(newPen);
    }

    QPointF startPoint = bounds.location();
    startPoint.setY(startPoint.y() + bounds.height() / 2);
    QPointF endPoint = startPoint;
    endPoint.setX(endPoint.x() + bounds.width());

    p->drawLine(startPoint, endPoint);

    if (doubleUnderlines) {
        // The space between double underlines is equal to the height of the underline
        // so distance between line centers is 2x height
        startPoint.setY(startPoint.y() + 2 * bounds.height());
        endPoint.setY(endPoint.y() + 2 * bounds.height());
        p->drawLine(startPoint, endPoint);
    }

    if (needSavePen)
        p->setPen(oldPen);

    p->setRenderHint(QPainter::Antialiasing, savedAntiAlias);
}

// NOTE: this code is based on GraphicsContextCG implementation
void GraphicsContextQt::drawLinesForText(const FloatPoint& origin, float thickness, const DashArray& widths, bool printing, bool doubleLines, StrokeStyle strokeStyle)
{
    if (widths.size() <= 0)
        return;

    Color localStrokeColor(strokeColor());

    FloatRect bounds = computeLineBoundsAndAntialiasingModeForText(FloatRect(origin, FloatSize(widths.last(), thickness)), printing, localStrokeColor);
    bool fillColorIsNotEqualToStrokeColor = fillColor() != localStrokeColor;

    // FIXME: drawRects() is significantly slower than drawLine() for thin lines (<= 1px)
    Vector<QRectF, 4> dashBounds;
    ASSERT(!(widths.size() % 2));
    dashBounds.reserveInitialCapacity(dashBounds.size() / 2);
    for (size_t i = 0; i < widths.size(); i += 2)
        dashBounds.append(QRectF(bounds.x() + widths[i], bounds.y(), widths[i+1] - widths[i], bounds.height()));

    if (doubleLines) {
        // The space between double underlines is equal to the height of the underline
        for (size_t i = 0; i < widths.size(); i += 2)
            dashBounds.append(QRectF(bounds.x() + widths[i], bounds.y() + 2 * bounds.height(), widths[i+1] - widths[i], bounds.height()));
    }

    QPainter* p = m_data->p();
    QPen oldPen = p->pen();
    p->setPen(Qt::NoPen);

    if (fillColorIsNotEqualToStrokeColor) {
        const QBrush oldBrush = p->brush();
        p->setBrush(QBrush(localStrokeColor));
        p->drawRects(dashBounds.data(), dashBounds.size());
        p->setBrush(oldBrush);
    } else {
        p->drawRects(dashBounds.data(), dashBounds.size());
    }

    p->setPen(oldPen);
}


/*
 *   NOTE: This code is completely based upon the one from
 *   Source/WebCore/platform/graphics/cairo/DrawErrorUnderline.{h|cpp}
 *
 *   Draws an error underline that looks like one of:
 *
 *               H       E                H
 *      /\      /\      /\        /\      /\               -
 *    A/  \    /  \    /  \     A/  \    /  \              |
 *     \   \  /    \  /   /D     \   \  /    \             |
 *      \   \/  C   \/   /        \   \/   C  \            | height = heightSquares * square
 *       \      /\  F   /          \  F   /\   \           |
 *        \    /  \    /            \    /  \   \G         |
 *         \  /    \  /              \  /    \  /          |
 *          \/      \/                \/      \/           -
 *          B                         B
 *          |---|
 *        unitWidth = (heightSquares - 1) * square
 *
 *  The x, y, width, height passed in give the desired bounding box;
 *  x/width are adjusted to make the underline a integer number of units wide.
*/
static void drawErrorUnderline(QPainter *painter, qreal x, qreal y, qreal width, qreal height)
{
    const qreal heightSquares = 2.5;

    qreal square = height / heightSquares;
    qreal halfSquare = 0.5 * square;

    qreal unitWidth = (heightSquares - 1.0) * square;
    int widthUnits = static_cast<int>((width + 0.5 * unitWidth) / unitWidth);

    x += 0.5 * (width - widthUnits * unitWidth);
    width = widthUnits * unitWidth;

    qreal bottom = y + height;
    qreal top = y;

    QPainterPath path;

    // Bottom of squiggle.
    path.moveTo(x - halfSquare, top + halfSquare); // A

    int i = 0;
    for (i = 0; i < widthUnits; i += 2) {
        qreal middle = x + (i + 1) * unitWidth;
        qreal right = x + (i + 2) * unitWidth;

        path.lineTo(middle, bottom); // B

        if (i + 2 == widthUnits)
            path.lineTo(right + halfSquare, top + halfSquare); // D
        else if (i + 1 != widthUnits)
            path.lineTo(right, top + square); // C
    }

    // Top of squiggle.
    for (i -= 2; i >= 0; i -= 2) {
        qreal left = x + i * unitWidth;
        qreal middle = x + (i + 1) * unitWidth;
        qreal right = x + (i + 2) * unitWidth;

        if (i + 1 == widthUnits)
            path.lineTo(middle + halfSquare, bottom - halfSquare); // G
        else {
            if (i + 2 == widthUnits)
                path.lineTo(right, top); // E

            path.lineTo(middle, bottom - halfSquare); // F
        }

        path.lineTo(left, top); // H
    }

    painter->drawPath(path);
}

void GraphicsContextQt::drawDotsForDocumentMarker(const FloatRect& rect, DocumentMarkerLineStyle style)
{
    if (paintingDisabled())
        return;

    QPainter* p = painter();
    const QPen originalPen = p->pen();

    // QTFIXME: Handle dark theme
    switch (style.mode) {
    case DocumentMarkerLineStyleMode::Spelling:
        p->setPen(Qt::red);
        break;
    case DocumentMarkerLineStyleMode::Grammar:
        p->setPen(Qt::green);
        break;
    default:
        return;
    }

    drawErrorUnderline(p, rect.x(), rect.y(), rect.width(), rect.height());
    p->setPen(originalPen);
}

void GraphicsContextQt::pushTransparencyLayerInternal(const QRect &rect, qreal opacity, const QImage& originalAlphaMask)
{
    QPainter* p = m_data->p();

    QTransform deviceTransform = p->transform();
    QRect deviceClip = deviceTransform.mapRect(rect);

    QImage alphaMask = originalAlphaMask.transformed(deviceTransform);
    if (alphaMask.width() != deviceClip.width() || alphaMask.height() != deviceClip.height())
        alphaMask = alphaMask.scaled(deviceClip.width(), deviceClip.height());

    m_data->layers.push(new TransparencyLayer(p, deviceClip, 1.0, alphaMask));
}

void GraphicsContextQt::beginTransparencyLayer(float opacity)
{
    GraphicsContext::beginTransparencyLayer(opacity);

    int x, y, w, h;
    x = y = 0;
    QPainter* p = m_data->p();
    const QPaintDevice* device = p->device();
    w = device->width();
    h = device->height();

    if (p->hasClipping()) {
        QRectF clip = m_data->clipBoundingRect();
        QRectF deviceClip = p->transform().mapRect(clip);
        x = int(qBound(qreal(0), deviceClip.x(), (qreal)w));
        y = int(qBound(qreal(0), deviceClip.y(), (qreal)h));
        w = int(qBound(qreal(0), deviceClip.width(), (qreal)w) + 2);
        h = int(qBound(qreal(0), deviceClip.height(), (qreal)h) + 2);
    }

    QImage emptyAlphaMask;
    m_data->layers.push(new TransparencyLayer(p, QRect(x, y, w, h), opacity, emptyAlphaMask));
    ++m_data->layerCount;
}

void GraphicsContextQt::popTransparencyLayerInternal()
{
    TransparencyLayer* layer = m_data->layers.pop();
    ASSERT(!layer->alphaMask.isNull());
    ASSERT(layer->saveCounter == 0);
    layer->painter.resetTransform();
    layer->painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    layer->painter.drawImage(QPoint(), layer->alphaMask);
    layer->painter.end();

    QPainter* p = m_data->p();
    p->save();
    p->resetTransform();
    p->setOpacity(layer->opacity);
    p->drawImage(layer->offset, layer->image);
    p->restore();

    delete layer;
}

void GraphicsContextQt::endTransparencyLayer()
{
    GraphicsContext::endTransparencyLayer();

    while ( ! m_data->layers.top()->alphaMask.isNull() ){
        --m_data->layers.top()->saveCounter;
        popTransparencyLayerInternal();
        if (m_data->layers.isEmpty())
            return;
    }
    TransparencyLayer* layer = m_data->layers.pop();
    --m_data->layerCount; // see the comment for layerCount
    layer->painter.end();

    QPainter* p = m_data->p();
    p->save();
    p->resetTransform();
    p->setOpacity(layer->opacity);
    p->drawImage(layer->offset, layer->image);
    p->restore();

    delete layer;
}

void GraphicsContextQt::clearRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    QPainter* p = m_data->p();
    QPainter::CompositionMode currentCompositionMode = p->compositionMode();
    p->setCompositionMode(QPainter::CompositionMode_Source);
    p->fillRect(rect, Qt::transparent);
    p->setCompositionMode(currentCompositionMode);
}

void GraphicsContextQt::strokeRect(const FloatRect& rect, float lineWidth)
{
    if (paintingDisabled())
        return;

    Path path;
    path.addRect(rect);

    float previousStrokeThickness = strokeThickness();

    if (lineWidth != previousStrokeThickness)
        setStrokeThickness(lineWidth);

    strokePath(path);

    if (lineWidth != previousStrokeThickness)
        setStrokeThickness(previousStrokeThickness);
}

void GraphicsContextQt::setLineCap(LineCap lc)
{
    if (paintingDisabled())
        return;

    QPainter* p = m_data->p();
    QPen nPen = p->pen();
    nPen.setCapStyle(toQtLineCap(lc));
    p->setPen(nPen);
}

void GraphicsContextQt::setLineDash(const DashArray& dashes, float dashOffset)
{
    QPainter* p = m_data->p();
    QPen pen = p->pen();
    unsigned dashLength = dashes.size();
    if (dashLength) {
        QVector<qreal> pattern;
        unsigned count = dashLength;
        if (dashLength % 2)
            count *= 2;

        float penWidth = narrowPrecisionToFloat(double(pen.widthF()));
        if (penWidth <= 0.f)
            penWidth = 1.f;

        for (unsigned i = 0; i < count; i++)
            pattern.append(dashes[i % dashLength] / penWidth);

        pen.setDashPattern(pattern);
        pen.setDashOffset(dashOffset / penWidth);
    } else
        pen.setStyle(Qt::SolidLine);
    p->setPen(pen);
}

void GraphicsContextQt::setLineJoin(LineJoin lj)
{
    if (paintingDisabled())
        return;

    QPainter* p = m_data->p();
    QPen nPen = p->pen();
    nPen.setJoinStyle(toQtLineJoin(lj));
    p->setPen(nPen);
}

void GraphicsContextQt::setMiterLimit(float limit)
{
    if (paintingDisabled())
        return;

    QPainter* p = m_data->p();
    QPen nPen = p->pen();
    nPen.setMiterLimit(limit);
    p->setPen(nPen);
}

void GraphicsContextQt::clipOut(const Path& path)
{
    if (paintingDisabled())
        return;

    QPainter* p = m_data->p();
    const QPainterPath clippedOut = path.platformPath();
    QPainterPath newClip;
    newClip.setFillRule(Qt::OddEvenFill);
    if (p->hasClipping()) {
        newClip.addRect(m_data->clipBoundingRect());
        newClip.addPath(clippedOut);
        p->setClipPath(newClip, Qt::IntersectClip);
    } else {
        QRect windowRect = p->transform().inverted().mapRect(p->window());
        newClip.addRect(windowRect);
        newClip.addPath(clippedOut.intersected(newClip));
        p->setClipPath(newClip);
    }
}

void GraphicsContextQt::translate(float x, float y)
{
    if (paintingDisabled())
        return;

    m_data->p()->translate(x, y);
}

void GraphicsContextQt::rotate(float radians)
{
    if (paintingDisabled())
        return;

    QTransform rotation = QTransform().rotateRadians(radians);
    m_data->p()->setTransform(rotation, true);
}

void GraphicsContextQt::scale(const FloatSize& s)
{
    if (paintingDisabled())
        return;

    m_data->p()->scale(s.width(), s.height());
}

void GraphicsContextQt::clipOut(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    QPainter* p = m_data->p();
    QPainterPath newClip;
    newClip.setFillRule(Qt::OddEvenFill);
    if (p->hasClipping()) {
        newClip.addRect(m_data->clipBoundingRect());
        newClip.addRect(QRectF(rect));
        p->setClipPath(newClip, Qt::IntersectClip);
    } else {
        QRectF clipOutRect(rect);
        QRect window = p->transform().inverted().mapRect(p->window());
        clipOutRect &= window;
        newClip.addRect(window);
        newClip.addRect(clipOutRect);
        p->setClipPath(newClip);
    }
}

void GraphicsContextQt::concatCTM(const AffineTransform& transform)
{
    if (paintingDisabled())
        return;

    m_data->p()->setWorldTransform(transform, true);
}

void GraphicsContextQt::setCTM(const AffineTransform& transform)
{
    if (paintingDisabled())
        return;

    m_data->p()->setWorldTransform(transform);
}

#if ENABLE(3D_TRANSFORMS)
TransformationMatrix GraphicsContextQt::get3DTransform() const
{
    if (paintingDisabled())
        return TransformationMatrix();

    return painter()->worldTransform();
}

void GraphicsContextQt::concat3DTransform(const TransformationMatrix& transform)
{
    if (paintingDisabled())
        return;

    m_data->p()->setWorldTransform(transform, true);
}

void GraphicsContextQt::set3DTransform(const TransformationMatrix& transform)
{
    if (paintingDisabled())
        return;

    m_data->p()->setWorldTransform(transform, false);
}
#endif

void GraphicsContextQt::setURLForRect(const URL& url, const FloatRect& rect)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && !defined(QT_NO_PDF)
    if (paintingDisabled())
        return;

    QPainter* p = m_data->p();
    if (p->paintEngine()->type() == QPaintEngine::Pdf)
        static_cast<QPdfEngine *>(p->paintEngine())->drawHyperlink(p->worldTransform().mapRect(rect), url);
#else
    UNUSED_PARAM(url);
    UNUSED_PARAM(rect);
#endif
}

void GraphicsContextQt::didUpdateState(GraphicsContextState& state)
{
    if (!state.changes())
        return;

    auto context = platformContext();

    QPainter* p = m_data->p();
    QPen newPen(p->pen());

    for (auto change : state.changes())
    {
        switch (change) {
        case GraphicsContextState::Change::FillBrush:
            m_data->solidColor.setColor(state.fillBrush().color());
            m_data->p()->setBrush(m_data->solidColor);
            break;

        case GraphicsContextState::Change::StrokeThickness:
            newPen.setWidthF(std::max(state.strokeThickness(), 0.f));
            break;

        case GraphicsContextState::Change::StrokeBrush:
            m_data->solidColor.setColor(state.strokeBrush().color());
            newPen.setBrush(m_data->solidColor);
            break;

        case GraphicsContextState::Change::StrokeStyle:
            newPen.setStyle(toQPenStyle(state.strokeStyle()));
            break;

        case GraphicsContextState::Change::CompositeMode:
            if (state.compositeMode().operation == WebCore::CompositeOperator::SourceOver)
                p->setCompositionMode(toQtCompositionMode(state.compositeMode().blendMode));
            else
                p->setCompositionMode(toQtCompositionMode(state.compositeMode().operation));
            break;

        case GraphicsContextState::Change::DropShadow:
            // not handled
            break;

        case GraphicsContextState::Change::Alpha:
            p->setOpacity(state.alpha());
            break;

        case GraphicsContextState::Change::ImageInterpolationQuality:
            switch (state.imageInterpolationQuality()) {
                case InterpolationQuality::DoNotInterpolate:
                case InterpolationQuality::Low:
                    // use nearest-neigbor
                    p->setRenderHint(QPainter::SmoothPixmapTransform, false);
                    break;

                case InterpolationQuality::Medium:
                case InterpolationQuality::High:
                    // use the filter
                    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
                    break;

                case InterpolationQuality::Default:
                default:
                    p->setRenderHint(QPainter::SmoothPixmapTransform, m_data->initialSmoothPixmapTransformHint);
                    break;
            };
            break;

        case GraphicsContextState::Change::TextDrawingMode:
            // not handled
            break;

        case GraphicsContextState::Change::ShouldAntialias:
            p->setRenderHint(QPainter::Antialiasing, state.shouldAntialias());
            break;

        case GraphicsContextState::Change::ShouldSmoothFonts:
            // not handled
            break;
        }
    }

    p->setPen(newPen);
}

#if OS(WINDOWS)

HDC GraphicsContextQt::getWindowsContext(const IntRect& dstRect, bool supportAlphaBlend)
{
    if (dstRect.isEmpty())
        return 0;

    // Create a bitmap DC in which to draw.
    BITMAPINFO bitmapInfo;
    bitmapInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth         = dstRect.width();
    bitmapInfo.bmiHeader.biHeight        = dstRect.height();
    bitmapInfo.bmiHeader.biPlanes        = 1;
    bitmapInfo.bmiHeader.biBitCount      = 32;
    bitmapInfo.bmiHeader.biCompression   = BI_RGB;
    bitmapInfo.bmiHeader.biSizeImage     = 0;
    bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    bitmapInfo.bmiHeader.biClrUsed       = 0;
    bitmapInfo.bmiHeader.biClrImportant  = 0;

    void* pixels = 0;
    HBITMAP bitmap = ::CreateDIBSection(0, &bitmapInfo, DIB_RGB_COLORS, &pixels, 0, 0);
    if (!bitmap)
        return 0;

    HDC displayDC = ::GetDC(0);
    HDC bitmapDC = ::CreateCompatibleDC(displayDC);
    ::ReleaseDC(0, displayDC);

    ::SelectObject(bitmapDC, bitmap);

    // Fill our buffer with clear if we're going to alpha blend.
    if (supportAlphaBlend) {
        BITMAP bmpInfo;
        GetObject(bitmap, sizeof(bmpInfo), &bmpInfo);
        int bufferSize = bmpInfo.bmWidthBytes * bmpInfo.bmHeight;
        memset(bmpInfo.bmBits, 0, bufferSize);
    }

#if !OS(WINCE)
    // Make sure we can do world transforms.
    SetGraphicsMode(bitmapDC, GM_ADVANCED);

    // Apply a translation to our context so that the drawing done will be at (0,0) of the bitmap.
    XFORM xform;
    xform.eM11 = 1.0f;
    xform.eM12 = 0.0f;
    xform.eM21 = 0.0f;
    xform.eM22 = 1.0f;
    xform.eDx = -dstRect.x();
    xform.eDy = -dstRect.y();
    ::SetWorldTransform(bitmapDC, &xform);
#endif

    return bitmapDC;
}

void GraphicsContextQt::releaseWindowsContext(HDC hdc, const IntRect& dstRect, bool supportAlphaBlend)
{
    if (hdc) {

        if (!dstRect.isEmpty()) {

            HBITMAP bitmap = static_cast<HBITMAP>(GetCurrentObject(hdc, OBJ_BITMAP));
            BITMAP info;
            GetObject(bitmap, sizeof(info), &info);
            ASSERT(info.bmBitsPixel == 32);

            QPixmap pixmap = qt_pixmapFromWinHBITMAP(bitmap, supportAlphaBlend ? HBitmapPremultipliedAlpha : HBitmapNoAlpha);
            m_data->p()->drawPixmap(dstRect, pixmap);

            ::DeleteObject(bitmap);
        }

        ::DeleteDC(hdc);
    }
}
#endif

void GraphicsContextQt::takeOwnershipOfPlatformContext()
{
    m_data->takeOwnershipOfPlatformContext();
}

RenderingMode GraphicsContextQt::renderingMode() const
{
    return (painter()->paintEngine()->type() == QPaintEngine::OpenGL2) ? RenderingMode::Accelerated : RenderingMode::Unaccelerated;
}

}
// vim: ts=4 sw=4 et
