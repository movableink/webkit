/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2008 Holger Hans Peter Freyther
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2010 Torch Mobile (Beijing) Co. Ltd. All rights reserved.
 * Copyright (C) 2014 Digia Plc. and/or its subsidiary(-ies)
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
#include "ImageBufferQtBackend.h"
#include "StillImageQt.h"
#include "NativeImageQt.h"
#include "ImageData.h"
#include "GraphicsContextQt.h"
#include "ColorUtilities.h"
#include "ColorTransferFunctions.h"
#include "IntRect.h"
#include "MIMETypeRegistry.h"
#include <wtf/IsoMallocInlines.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/TextStream.h>

#include <QImage>
#include <QPainter>
#include <QBuffer>

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(ImageBufferQtBackend);

ImageBufferQtBackend::ImageBufferQtBackend(const Parameters& parameters, std::unique_ptr<GraphicsContext>&& context, std::unique_ptr<QImage>&& nativeImage, Ref<Image> image)
    : ImageBufferBackend(parameters)
    , m_context(WTFMove(context))
    , m_nativeImage(WTFMove(nativeImage))
    , m_image(WTFMove(image))
{}

std::unique_ptr<ImageBufferQtBackend> ImageBufferQtBackend::create(const Parameters& parameters, const ImageBufferCreationContext&)
{
    IntSize backendSize = parameters.backendSize;
    if (backendSize.isEmpty())
        return nullptr;

    QPainter* painter = new QPainter;

    auto nativeImage = makeUniqueWithoutFastMallocCheck<QImage>(IntSize(parameters.backendSize * parameters.resolutionScale), NativeImageQt::defaultFormatForAlphaEnabledImages());
    nativeImage->fill(QColor(Qt::transparent));
    nativeImage->setDevicePixelRatio(parameters.resolutionScale);

    if (!painter->begin(nativeImage.get()))
        return nullptr;

    ImageBufferQtBackend::initPainter(painter);

    auto image = StillImage::createForRendering(nativeImage.get());
    auto context = makeUnique<GraphicsContextQt>(painter);

    return std::unique_ptr<ImageBufferQtBackend>(new ImageBufferQtBackend(parameters, WTFMove(context), WTFMove(nativeImage), WTFMove(image)));
}

std::unique_ptr<ImageBufferQtBackend> ImageBufferQtBackend::create(const Parameters& parameters, const GraphicsContext& context)
{
    return ImageBufferQtBackend::create(parameters, ImageBufferCreationContext { });
}

size_t ImageBufferQtBackend::calculateMemoryCost(const Parameters& parameters)
{
    IntSize backendSize = parameters.backendSize;
    return ImageBufferBackend::calculateMemoryCost(backendSize, calculateBytesPerRow(backendSize));
}

size_t ImageBufferQtBackend::calculateExternalMemoryCost(const Parameters& parameters)
{
    return calculateMemoryCost(parameters);
}

unsigned ImageBufferQtBackend::calculateBytesPerRow(const IntSize& backendSize)
{
    ASSERT(!backendSize.isEmpty());
    return CheckedUint32(backendSize.width()) * 4;
}

void ImageBufferQtBackend::initPainter(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // Since ImageBuffer is used mainly for Canvas, explicitly initialize
    // its painter's pen and brush with the corresponding canvas defaults
    // NOTE: keep in sync with CanvasRenderingContext2D::State
    QPen pen = painter->pen();
    pen.setColor(Qt::black);
    pen.setWidth(1);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::SvgMiterJoin);
    pen.setMiterLimit(10);
    painter->setPen(pen);

    QBrush brush = painter->brush();
    brush.setColor(Qt::black);
    painter->setBrush(brush);

    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

RefPtr<NativeImage> ImageBufferQtBackend::copyNativeImage()
{
    return NativeImage::create(m_nativeImage->copy());
}

RefPtr<NativeImage> ImageBufferQtBackend::createNativeImageReference()
{
    return NativeImage::create(WTFMove(QImage(*m_nativeImage.get())));
}

GraphicsContext &ImageBufferQtBackend::context() { return *m_context; }

void ImageBufferQtBackend::transformToColorSpace(const DestinationColorSpace& newColorSpace)
{
    if (m_parameters.colorSpace == newColorSpace)
        return;

#if ENABLE(DESTINATION_COLOR_SPACE_LINEAR_SRGB)
    // only sRGB <-> linearRGB are supported at the moment
    if ((m_parameters.colorSpace != DestinationColorSpace::LinearSRGB() && m_parameters.colorSpace != DestinationColorSpace::SRGB())
        || (newColorSpace != DestinationColorSpace::LinearSRGB() && newColorSpace != DestinationColorSpace::SRGB()))
        return;

    if (newColorSpace == DestinationColorSpace::LinearSRGB()) {
        static const std::array<uint8_t, 256> linearRgbLUT = [] {
            std::array<uint8_t, 256> array;
            for (unsigned i = 0; i < 256; i++) {
                float color = i / 255.0f;
                color = SRGBTransferFunction<float, TransferFunctionMode::Clamped>::toLinear(color);
                array[i] = static_cast<uint8_t>(round(color * 255));
            }
            return array;
        }();
        platformTransformColorSpace(linearRgbLUT);
    } else if (newColorSpace == DestinationColorSpace::SRGB()) {
        static const std::array<uint8_t, 256> deviceRgbLUT= [] {
            std::array<uint8_t, 256> array;
            for (unsigned i = 0; i < 256; i++) {
                float color = i / 255.0f;
                color = SRGBTransferFunction<float, TransferFunctionMode::Clamped>::toGammaEncoded(color);
                array[i] = static_cast<uint8_t>(round(color * 255));
            }
            return array;
        }();
        platformTransformColorSpace(deviceRgbLUT);
    }
#else
    ASSERT(newColorSpace == DestinationColorSpace::SRGB());
    ASSERT(m_parameters.colorSpace == DestinationColorSpace::SRGB());
    UNUSED_PARAM(newColorSpace);
#endif
}

void ImageBufferQtBackend::platformTransformColorSpace(const std::array<uint8_t, 256>& lookUpTable)
{
    QPainter* painter = context().platformContext()->painter();

    QImage image = m_nativeImage->convertToFormat(QImage::Format_ARGB32);
    ASSERT(!image.isNull());

    uchar* bits = image.bits();
    const int bytesPerLine = image.bytesPerLine();

    for (int y = 0; y < image.height(); ++y) {
        quint32* scanLine = reinterpret_cast_ptr<quint32*>(bits + y * bytesPerLine);
        for (int x = 0; x < image.width(); ++x) {
            QRgb& pixel = scanLine[x];
            pixel = qRgba(lookUpTable[qRed(pixel)],
                          lookUpTable[qGreen(pixel)],
                          lookUpTable[qBlue(pixel)],
                          qAlpha(pixel));
        }
    }

    painter->save();
    painter->resetTransform();
    painter->setOpacity(1.0);
    painter->setClipping(false);
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->drawImage(QPoint(0, 0), image);
    painter->restore();
}

void ImageBufferQtBackend::getPixelBuffer(const IntRect& srcRect, PixelBuffer& destination)
{
    ImageBufferBackend::getPixelBuffer(srcRect, m_nativeImage->bits(), destination);
}

void ImageBufferQtBackend::putPixelBuffer(const PixelBuffer& pixelBuffer, const IntRect& srcRect, const IntPoint& destPoint, AlphaPremultiplication destFormat)
{
    ImageBufferBackend::putPixelBuffer(pixelBuffer, srcRect, destPoint, destFormat, const_cast<void*>(reinterpret_cast<const void*>(m_nativeImage->bits())));
}

unsigned ImageBufferQtBackend::bytesPerRow() const
{
    return m_nativeImage->bytesPerLine();
}

String ImageBufferQtBackend::debugDescription() const
{
    TextStream stream;
    stream << "ImageBufferQtBackend " << m_nativeImage->width() << "x" << m_nativeImage->height() << " " << m_nativeImage->format();
    return stream.release();
}

} // namespace WebCore
