/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2009, 2010 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2010, 2011 Andreas Kling <kling@webkit.org>
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
#include "PathQt.h"

#include "AffineTransform.h"
#include "FloatRect.h"
#include "GraphicsContextQt.h"
#include "NativeImageQt.h"
#include <QPainterPath>
#include <QPainter>
#include <QPen>
#include <QString>
#include <QTransform>
#include <wtf/MathExtras.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

Ref<PathQt> PathQt::create()
{
    return adoptRef(*new PathQt);
}

Ref<PathQt> PathQt::create(const PathStream& stream)
{
    auto pathQt = PathQt::create();

    stream.applySegments([&](const PathSegment& segment) {
        pathQt->appendSegment(segment);
    });

    return pathQt;
}

Ref<PathQt> PathQt::create(const PathSegment& segment)
{
    auto pathQt = PathQt::create();
    pathQt->appendSegment(segment);

    return pathQt;
}

Ref<PathQt> PathQt::create(QPainterPath platformPath)
{
    return adoptRef(*new PathQt(WTFMove(platformPath)));
}

PathQt::PathQt()
{
}

PathQt::PathQt(QPainterPath&& platformPath)
    : m_path(WTFMove(platformPath))
{
}

PathQt::PathQt(const PathQt& other)
    : m_path(other.m_path)
{
}

PathQt::PathQt(PathQt&& other)
{
    m_path.swap(other.m_path);
}

PathQt& PathQt::operator=(const PathQt& other)
{
    m_path = other.m_path;
    return *this;
}

PathQt& PathQt::operator=(PathQt&& other)
{
    m_path.swap(other.m_path);
    return *this;
}

Ref<PathImpl> PathQt::copy() const
{
    return create(m_path);
}

QPainterPath PathQt::platformPath() const
{
    return m_path;
}

static inline bool areCollinear(const QPointF& a, const QPointF& b, const QPointF& c)
{
    // Solved from comparing the slopes of a to b and b to c: (ay-by)/(ax-bx) == (cy-by)/(cx-bx)
    return qFuzzyCompare((c.y() - b.y()) * (a.x() - b.x()), (a.y() - b.y()) * (c.x() - b.x()));
}

static inline bool withinRange(qreal p, qreal a, qreal b)
{
    return (p >= a && p <= b) || (p >= b && p <= a);
}

// Check whether a point is on the border
static bool isPointOnPathBorder(const QPolygonF& border, const QPointF& p)
{
    // null border doesn't contain points
    if (border.isEmpty())
        return false;

    QPointF p1 = border.at(0);
    QPointF p2;

    for (int i = 1; i < border.size(); ++i) {
        p2 = border.at(i);
        if (areCollinear(p, p1, p2)
                // Once we know that the points are collinear we
                // only need to check one of the coordinates
                && (qAbs(p2.x() - p1.x()) > qAbs(p2.y() - p1.y()) ?
                        withinRange(p.x(), p1.x(), p2.x()) :
                        withinRange(p.y(), p1.y(), p2.y()))) {
            return true;
        }
        p1 = p2;
    }
    return false;
}

bool PathQt::contains(const FloatPoint& point, WindRule rule) const
{
    Qt::FillRule savedRule = m_path.fillRule();
    const_cast<QPainterPath*>(&m_path)->setFillRule(rule == WindRule::EvenOdd ? Qt::OddEvenFill : Qt::WindingFill);

    bool contains = m_path.contains(point);

    if (!contains) {
        // check whether the point is on the border
        contains = isPointOnPathBorder(m_path.toFillPolygon(), point);
    }

    const_cast<QPainterPath*>(&m_path)->setFillRule(savedRule);
    return contains;
}

static QPainter* scratchContext()
{
    static QImage image(1, 1, NativeImageQt::defaultFormatForAlphaEnabledImages());
    static QPainter painter(&image);
    return &painter;
}

bool PathQt::strokeContains(const FloatPoint& point, const Function<void(GraphicsContext&)>& strokeStyleApplier) const
{
    ASSERT(strokeStyleApplier);

    QPainterPathStroker stroke;
    GraphicsContextQt context(scratchContext());
    strokeStyleApplier(context);

    QPen pen = context.painter()->pen();
    stroke.setWidth(pen.widthF());
    stroke.setCapStyle(pen.capStyle());
    stroke.setJoinStyle(pen.joinStyle());
    stroke.setMiterLimit(pen.miterLimit());
    stroke.setDashPattern(pen.dashPattern());
    stroke.setDashOffset(pen.dashOffset());

    return stroke.createStroke(m_path).contains(point);
}

FloatRect PathQt::fastBoundingRect() const
{
    return m_path.controlPointRect();
}

FloatRect PathQt::boundingRect() const
{
    return m_path.boundingRect();
}

FloatRect PathQt::strokeBoundingRect(const Function<void(GraphicsContext&)>& strokeStyleApplier) const
{
    GraphicsContextQt context(scratchContext());
    QPainterPathStroker stroke;
    if (strokeStyleApplier) {
        strokeStyleApplier(context);

        QPen pen = context.painter()->pen();
        stroke.setWidth(pen.widthF());
        stroke.setCapStyle(pen.capStyle());
        stroke.setJoinStyle(pen.joinStyle());
        stroke.setMiterLimit(pen.miterLimit());
        stroke.setDashPattern(pen.dashPattern());
        stroke.setDashOffset(pen.dashOffset());
    }
    return stroke.createStroke(m_path).boundingRect();
}

void PathQt::moveTo(const FloatPoint& point)
{
    m_path.moveTo(point);
}

void PathQt::addLineTo(const FloatPoint& p)
{
    m_path.lineTo(p);
}

void PathQt::addQuadCurveTo(const FloatPoint& cp, const FloatPoint& p)
{
    m_path.quadTo(cp, p);
}

void PathQt::addBezierCurveTo(const FloatPoint& cp1, const FloatPoint& cp2, const FloatPoint& p)
{
    m_path.cubicTo(cp1, cp2, p);
}

void PathQt::addArcTo(const FloatPoint& p1, const FloatPoint& p2, float radius)
{
    FloatPoint p0(m_path.currentPosition());

    FloatPoint p1p0((p0.x() - p1.x()), (p0.y() - p1.y()));
    FloatPoint p1p2((p2.x() - p1.x()), (p2.y() - p1.y()));
    float p1p0_length = sqrtf(p1p0.x() * p1p0.x() + p1p0.y() * p1p0.y());
    float p1p2_length = sqrtf(p1p2.x() * p1p2.x() + p1p2.y() * p1p2.y());

    double cos_phi = (p1p0.x() * p1p2.x() + p1p0.y() * p1p2.y()) / (p1p0_length * p1p2_length);

    // The points p0, p1, and p2 are on the same straight line (HTML5, 4.8.11.1.8)
    // We could have used areCollinear() here, but since we're reusing
    // the variables computed above later on we keep this logic.
    if (qFuzzyCompare(qAbs(cos_phi), 1.0)) {
        m_path.lineTo(p1);
        return;
    }

    float tangent = radius / tan(acos(cos_phi) / 2);
    float factor_p1p0 = tangent / p1p0_length;
    FloatPoint t_p1p0((p1.x() + factor_p1p0 * p1p0.x()), (p1.y() + factor_p1p0 * p1p0.y()));

    FloatPoint orth_p1p0(p1p0.y(), -p1p0.x());
    float orth_p1p0_length = sqrt(orth_p1p0.x() * orth_p1p0.x() + orth_p1p0.y() * orth_p1p0.y());
    float factor_ra = radius / orth_p1p0_length;

    // angle between orth_p1p0 and p1p2 to get the right vector orthographic to p1p0
    double cos_alpha = (orth_p1p0.x() * p1p2.x() + orth_p1p0.y() * p1p2.y()) / (orth_p1p0_length * p1p2_length);
    if (cos_alpha < 0.f)
        orth_p1p0 = FloatPoint(-orth_p1p0.x(), -orth_p1p0.y());

    FloatPoint p((t_p1p0.x() + factor_ra * orth_p1p0.x()), (t_p1p0.y() + factor_ra * orth_p1p0.y()));

    // calculate angles for addArc
    orth_p1p0 = FloatPoint(-orth_p1p0.x(), -orth_p1p0.y());
    float sa = acos(orth_p1p0.x() / orth_p1p0_length);
    if (orth_p1p0.y() < 0.f)
        sa = 2 * piDouble - sa;

    // anticlockwise logic
    RotationDirection rotationDirection = RotationDirection::Clockwise;

    float factor_p1p2 = tangent / p1p2_length;
    FloatPoint t_p1p2((p1.x() + factor_p1p2 * p1p2.x()), (p1.y() + factor_p1p2 * p1p2.y()));
    FloatPoint orth_p1p2((t_p1p2.x() - p.x()), (t_p1p2.y() - p.y()));
    float orth_p1p2_length = sqrtf(orth_p1p2.x() * orth_p1p2.x() + orth_p1p2.y() * orth_p1p2.y());
    float ea = acos(orth_p1p2.x() / orth_p1p2_length);
    if (orth_p1p2.y() < 0)
        ea = 2 * piDouble - ea;
    if ((sa > ea) && ((sa - ea) < piDouble))
        rotationDirection = RotationDirection::Counterclockwise;
    if ((sa < ea) && ((ea - sa) > piDouble))
        rotationDirection = RotationDirection::Counterclockwise;

    m_path.lineTo(t_p1p0);

    addArc(p, radius, sa, ea, rotationDirection);
}

void PathQt::closeSubpath()
{
    m_path.closeSubpath();
}

static void addEllipticArc(QPainterPath &path, qreal xc, qreal yc, qreal radiusX, qreal radiusY,  float sar, float ear, RotationDirection rotationDirection)
{
    //### HACK
    // In Qt we don't switch the coordinate system for degrees
    // and still use the 0,0 as bottom left for degrees so we need
    // to switch
    sar = -sar;
    ear = -ear;
    bool anticlockwise = rotationDirection != RotationDirection::Counterclockwise;
    //end hack

    float sa = rad2deg(sar);
    float ea = rad2deg(ear);

    double span = 0;

    double xs = xc - radiusX;
    double ys = yc - radiusY;
    double width  = radiusX * 2;
    double height = radiusY * 2;

    if ((!anticlockwise && (ea - sa >= 360)) || (anticlockwise && (sa - ea >= 360))) {
        // If the anticlockwise argument is false and endAngle-startAngle is equal to or greater than 2*PI, or, if the
        // anticlockwise argument is true and startAngle-endAngle is equal to or greater than 2*PI, then the arc is the whole
        // circumference of this circle.
        span = 360;

        if (anticlockwise)
            span = -span;
    } else {
        if (!anticlockwise && (ea < sa))
            span += 360;
        else if (anticlockwise && (sa < ea))
            span -= 360;

        // this is also due to switched coordinate system
        // we would end up with a 0 span instead of 360
        if (!(qFuzzyCompare(span + (ea - sa) + 1, 1.0)
            && qFuzzyCompare(qAbs(span), 360.0))) {
            // mod 360
            span += (ea - sa) - (static_cast<int>((ea - sa) / 360)) * 360;
        }
    }

    // If the path is empty, move to where the arc will start to avoid painting a line from (0,0)
    // NOTE: QPainterPath::isEmpty() won't work here since it ignores a lone MoveToElement
    if (!path.elementCount())
        path.arcMoveTo(xs, ys, width, height, sa);
    else if (qFuzzyIsNull(radiusX) || qFuzzyIsNull(radiusY)) {
        path.lineTo(xc, yc);
        return;
    }

    path.arcTo(xs, ys, width, height, sa, span);
}

void PathQt::addArc(const FloatPoint& p, float r, float sar, float ear, RotationDirection rotationDirection)
{
    addEllipticArc(m_path, p.x(), p.y(), r, r, sar, ear, rotationDirection);
}

void PathQt::addRect(const FloatRect& r)
{
    m_path.addRect(r.x(), r.y(), r.width(), r.height());
}

void PathQt::addRoundedRect(const FloatRoundedRect& roundedRect, PathRoundedRect::Strategy)
{
    addBeziersForRoundedRect(roundedRect);
}

void PathQt::addEllipse(const FloatPoint& center, float radiusX, float radiusY, float rotation, float startAngle, float endAngle, RotationDirection rotationDirection)
{
    if (!qFuzzyIsNull(rotation)) {
        QPainterPath subPath;
        QTransform t;
        t.translate(center.x(), center.y());
        t.rotateRadians(rotation);

        bool isEmpty = m_path.elementCount() == 0;
        if (!isEmpty) {
            QTransform invTransform = t.inverted(nullptr);
            subPath.moveTo(invTransform.map(m_path.currentPosition()));
        }

        addEllipticArc(subPath, 0, 0, radiusX, radiusY, startAngle, endAngle, rotationDirection);
        subPath = t.map(subPath);

        if (isEmpty)
            m_path = subPath;
        else
            m_path.connectPath(subPath);
        return;
    }

    addEllipticArc(m_path, center.x(), center.y(), radiusX, radiusY, startAngle, endAngle, rotationDirection);
}

void PathQt::addEllipseInRect(const FloatRect& r)
{
    m_path.addEllipse(r.x(), r.y(), r.width(), r.height());
}

void PathQt::addPath(const PathQt& path, const AffineTransform& transform)
{
    if (!transform.isInvertible())
        return;

    QTransform qTransform(transform);
    m_path.addPath(qTransform.map(path.platformPath()));
}

bool PathQt::isEmpty() const
{
    // Don't use QPainterPath::isEmpty(), as that also returns true if there's only
    // one initial MoveTo element in the path.
    return !m_path.elementCount();
}

FloatPoint PathQt::currentPoint() const
{
    return m_path.currentPosition();
}

void PathQt::applySegments(const PathSegmentApplier& applier) const
{
    applyElements([&] (const PathElement& pathElement) {
        switch (pathElement.type) {
        case PathElement::Type::MoveToPoint:
            applier({ PathMoveTo { pathElement.points[0] } });
            break;

        case PathElement::Type::AddLineToPoint:
            applier({ PathLineTo { pathElement.points[0] } });
            break;

        case PathElement::Type::AddQuadCurveToPoint:
            applier({ PathQuadCurveTo { pathElement.points[0], pathElement.points[1] } });
            break;

        case PathElement::Type::AddCurveToPoint:
            applier({ PathBezierCurveTo { pathElement.points[0], pathElement.points[1], pathElement.points[2] } });
            break;

        case PathElement::Type::CloseSubpath:
            applier({ PathCloseSubpath { } });
            break;
        }
    });
}

bool PathQt::applyElements(const PathElementApplier& applier) const
{
    PathElement pelement;
    for (int i = 0; i < m_path.elementCount(); ++i) {
        const QPainterPath::Element& cur = m_path.elementAt(i);

        switch (cur.type) {
            case QPainterPath::MoveToElement:
                pelement.type = PathElement::Type::MoveToPoint;
                pelement.points[0] = QPointF(cur);
                applier(pelement);
                break;
            case QPainterPath::LineToElement:
                pelement.type = PathElement::Type::AddLineToPoint;
                pelement.points[0] = QPointF(cur);
                applier(pelement);
                break;
            case QPainterPath::CurveToElement:
            {
                const QPainterPath::Element& c1 = m_path.elementAt(i + 1);
                const QPainterPath::Element& c2 = m_path.elementAt(i + 2);

                Q_ASSERT(c1.type == QPainterPath::CurveToDataElement);
                Q_ASSERT(c2.type == QPainterPath::CurveToDataElement);

                pelement.type = PathElement::Type::AddCurveToPoint;
                pelement.points[0] = QPointF(cur);
                pelement.points[1] = QPointF(c1);
                pelement.points[2] = QPointF(c2);
                applier(pelement);

                i += 2;
                break;
            }
            case QPainterPath::CurveToDataElement:
                Q_ASSERT(false);
        }
    }
    return true;
}

bool PathQt::transform(const AffineTransform& transform)
{
    QTransform qTransform(transform);
    m_path = qTransform.map(m_path);

    return true;
}

}
