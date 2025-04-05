/*
    Copyright (C) 2007 Eric Seidel <eric@webkit.org>
    Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
    Copyright (C) 2019 Apple Inc. All rights reserved.

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
#include "CSSComputedStyleDeclaration.h"

#include "CSSPrimitiveValueMappings.h"
#include "CSSPropertyNames.h"
#include "CSSURLValue.h"
#include "CSSValueList.h"
#include "CSSValuePool.h"
#include "Document.h"
#include "Element.h"
#include "RenderStyle.h"
#include "SVGRenderStyle.h"
#include "StyleURL.h"
#include <wtf/URL.h>

namespace WebCore {

static RefPtr<CSSPrimitiveValue> createCSSValue(GlyphOrientation orientation)
{
    switch (orientation) {
    case GlyphOrientation::Degrees0:
        return CSSPrimitiveValue::create(0.0f, CSSUnitType::CSS_DEG);
    case GlyphOrientation::Degrees90:
        return CSSPrimitiveValue::create(90.0f, CSSUnitType::CSS_DEG);
    case GlyphOrientation::Degrees180:
        return CSSPrimitiveValue::create(180.0f, CSSUnitType::CSS_DEG);
    case GlyphOrientation::Degrees270:
        return CSSPrimitiveValue::create(270.0f, CSSUnitType::CSS_DEG);
    case GlyphOrientation::Auto:
        return nullptr;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

static Ref<CSSValue> createCSSValue(const Vector<SVGLengthValue>& dashes)
{
    if (dashes.isEmpty())
        return CSSPrimitiveValue::create(CSSValueNone);

    CSSValueListBuilder list;
    for (auto& length : dashes) {
        auto primitiveValue = length.toCSSPrimitiveValue();
        // Computed lengths should always be in 'px' unit.
        if (primitiveValue->isLength() && primitiveValue->primitiveType() != CSSUnitType::CSS_PX)
            list.append(CSSPrimitiveValue::create(primitiveValue->resolveAsLengthDeprecated(), CSSUnitType::CSS_PX));
        else
            list.append(WTFMove(primitiveValue));
    }
    return CSSValueList::createCommaSeparated(WTFMove(list));
}

Ref<CSSValue> ComputedStyleExtractor::adjustSVGPaint(const RenderStyle& style, SVGPaintType paintType, const Style::URL& url, const Style::Color& color) const
{
    if (paintType >= SVGPaintType::URINone) {
        CSSValueListBuilder values;
        values.append(CSSURLValue::create(Style::toCSS(url, style)));
        if (paintType == SVGPaintType::URINone)
            values.append(CSSPrimitiveValue::create(CSSValueNone));
        else if (paintType == SVGPaintType::URICurrentColor || paintType == SVGPaintType::URIRGBColor)
            values.append(currentColorOrValidColor(style, color));
        return CSSValueList::createSpaceSeparated(WTFMove(values));
    }
    if (paintType == SVGPaintType::None)
        return CSSPrimitiveValue::create(CSSValueNone);
    
    return currentColorOrValidColor(style, color);
}

static RefPtr<CSSValue> svgMarkerValue(const RenderStyle& style, const Style::URL& marker)
{
    if (marker.isNone())
        return CSSPrimitiveValue::create(CSSValueNone);
    return CSSURLValue::create(Style::toCSS(marker, style));
}

RefPtr<CSSValue> ComputedStyleExtractor::svgPropertyValue(CSSPropertyID propertyID) const
{
    if (!m_element)
        return nullptr;

    auto* style = m_element->computedStyle();
    if (!style)
        return nullptr;

    auto& svgStyle = style->svgStyle();

    switch (propertyID) {
    case CSSPropertyClipRule:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.clipRule()));
    case CSSPropertyFloodOpacity:
        return CSSPrimitiveValue::create(svgStyle.floodOpacity());
    case CSSPropertyStopOpacity:
        return CSSPrimitiveValue::create(svgStyle.stopOpacity());
    case CSSPropertyColorInterpolation:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.colorInterpolation()));
    case CSSPropertyColorInterpolationFilters:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.colorInterpolationFilters()));
    case CSSPropertyFillOpacity:
        return CSSPrimitiveValue::create(svgStyle.fillOpacity());
    case CSSPropertyFillRule:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.fillRule()));
    case CSSPropertyShapeRendering:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.shapeRendering()));
    case CSSPropertyStrokeOpacity:
        return CSSPrimitiveValue::create(svgStyle.strokeOpacity());
    case CSSPropertyAlignmentBaseline:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.alignmentBaseline()));
    case CSSPropertyDominantBaseline:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.dominantBaseline()));
    case CSSPropertyTextAnchor:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.textAnchor()));
    case CSSPropertyFloodColor:
        return currentColorOrValidColor(*style, svgStyle.floodColor());
    case CSSPropertyLightingColor:
        return currentColorOrValidColor(*style, svgStyle.lightingColor());
    case CSSPropertyStopColor:
        return currentColorOrValidColor(*style, svgStyle.stopColor());
    case CSSPropertyFill:
        return adjustSVGPaint(*style, svgStyle.fillPaintType(), svgStyle.fillPaintUri(), svgStyle.fillPaintColor());
    case CSSPropertyMarkerEnd:
        return svgMarkerValue(*style, svgStyle.markerEndResource());
    case CSSPropertyMarkerMid:
        return svgMarkerValue(*style, svgStyle.markerMidResource());
    case CSSPropertyMarkerStart:
        return svgMarkerValue(*style, svgStyle.markerStartResource());
    case CSSPropertyStroke:
        return adjustSVGPaint(*style, svgStyle.strokePaintType(), svgStyle.strokePaintUri(), svgStyle.strokePaintColor());
    case CSSPropertyStrokeDasharray:
        return createCSSValue(svgStyle.strokeDashArray());
    case CSSPropertyBaselineShift: {
        switch (svgStyle.baselineShift()) {
        case BaselineShift::Baseline:
            return CSSPrimitiveValue::create(CSSValueBaseline);
        case BaselineShift::Super:
            return CSSPrimitiveValue::create(CSSValueSuper);
        case BaselineShift::Sub:
            return CSSPrimitiveValue::create(CSSValueSub);
        case BaselineShift::Length: {
            auto computedValue = svgStyle.baselineShiftValue().toCSSPrimitiveValue(m_element.get());
            if (computedValue->isLength() && computedValue->primitiveType() != CSSUnitType::CSS_PX)
                return CSSPrimitiveValue::create(computedValue->resolveAsLengthDeprecated(), CSSUnitType::CSS_PX);
            return computedValue;
        }
        }
        ASSERT_NOT_REACHED();
        return nullptr;
    }
    case CSSPropertyBufferedRendering:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.bufferedRendering()));
    case CSSPropertyGlyphOrientationHorizontal:
        return createCSSValue(svgStyle.glyphOrientationHorizontal());
    case CSSPropertyGlyphOrientationVertical: {
        if (RefPtr<CSSPrimitiveValue> value = createCSSValue(svgStyle.glyphOrientationVertical()))
            return value;

        if (svgStyle.glyphOrientationVertical() == GlyphOrientation::Auto)
            return CSSPrimitiveValue::create(CSSValueAuto);

        return nullptr;
    }
    case CSSPropertyVectorEffect:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.vectorEffect()));
    case CSSPropertyMaskType:
        return CSSPrimitiveValue::create(toCSSValueID(svgStyle.maskType()));
    case CSSPropertyMarker:
        // this property is not yet implemented in the engine
        break;
    default:
        // If you crash here, it's because you added a css property and are not handling it
        // in either this switch statement or the one in CSSComputedStyleDeclaration::getPropertyCSSValue
        ASSERT_WITH_MESSAGE(0, "unimplemented propertyID: %d", propertyID);
    }
    return nullptr;
}

}
