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
#include "GraphicsContext.h"
#include "ColorUtilities.h"
#include "IntRect.h"
#include "MIMETypeRegistry.h"
#include <wtf/IsoMallocInlines.h>

#include <QImage>
#include <QPainter>
#include <QBuffer>

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(ImageBufferQtBackend);

ImageBufferQtBackend::ImageBufferQtBackend(const FloatSize &logicalSize, const IntSize &backendSize, float resolutionScale, ColorSpace colorSpace, std::unique_ptr<GraphicsContext>&& context, QImage nativeImage, Ref<Image> image)
    : ImageBufferBackend(logicalSize, backendSize, resolutionScale, colorSpace)
    , m_nativeImage(nativeImage)
    , m_image(WTFMove(image))
    , m_context(WTFMove(context))
{}

std::unique_ptr<ImageBufferQtBackend> ImageBufferQtBackend::create(const FloatSize &size, float resolutionScale, ColorSpace colorSpace, const HostWindow *hostWindow)
{
    IntSize backendSize = calculateBackendSize(size, resolutionScale);
    if (backendSize.isEmpty())
        return nullptr;

    auto painter = new QPainter;

    auto nativeImage = QImage(IntSize(size * resolutionScale), NativeImageQt::defaultFormatForAlphaEnabledImages());
    nativeImage.fill(QColor(Qt::transparent));
    nativeImage.setDevicePixelRatio(resolutionScale);

    if (!painter->begin(&nativeImage))
        return nullptr;

    ImageBufferQtBackend::initPainter(painter);

    auto image = StillImage::create(nativeImage);

    auto context = std::make_unique<GraphicsContext>(painter);

    return std::make_unique<ImageBufferQtBackend>(size, backendSize, resolutionScale, colorSpace, WTFMove(context), nativeImage, WTFMove(image));
}

std::unique_ptr<ImageBufferQtBackend> ImageBufferQtBackend::create(const FloatSize &size, const GraphicsContext &)
{
    return ImageBufferQtBackend::create(size, 1, ColorSpace::SRGB, nullptr);
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

// QTFIXME: Use PreserveResolution?
RefPtr<Image> ImageBufferQtBackend::copyImage(BackingStoreCopy copyBehavior, PreserveResolution) const
{
    if (copyBehavior == CopyBackingStore)
        return StillImage::create(m_nativeImage);

    return StillImage::createForRendering(&m_nativeImage);
}

PlatformImagePtr ImageBufferQtBackend::copyNativeImage(BackingStoreCopy copyBehavior) const
{
    if (copyBehavior == CopyBackingStore)
        return m_nativeImage.copy();

    return QImage(m_nativeImage);
}

void ImageBufferQtBackend::draw(GraphicsContext &destContext, const FloatRect &destRect, const FloatRect &srcRect, const ImagePaintingOptions &options) {
    if (&destContext == &context()) {
        RefPtr<Image> copy = copyImage();
        destContext.drawImage(*copy, destRect, srcRect, options);
    } else {
        destContext.drawImage(*m_image, destRect, srcRect, options);
    }
}

void ImageBufferQtBackend::drawPattern(GraphicsContext &destContext, const FloatRect &destRect, const FloatRect &srcRect, const AffineTransform &patternTransform, const FloatPoint &phase, const FloatSize &spacing, const ImagePaintingOptions &options)
{
    if (&destContext == &context()) {
        RefPtr<Image> copy = copyImage();
        copy->drawPattern(destContext, destRect, srcRect, patternTransform, phase, spacing, options);
    } else {
        m_image->drawPattern(destContext, destRect, srcRect, patternTransform, phase, spacing, options);
    }
}

GraphicsContext &ImageBufferQtBackend::context() const { return *m_context; }

static bool encodeImage(const QImage& image, const String& mimeType, std::optional<double> quality, QByteArray& data)
{
    ASSERT(MIMETypeRegistry::isSupportedImageMIMETypeForEncoding(mimeType));

    // QImageWriter does not support mimetypes. It does support Qt image formats (png,
    // gif, jpeg..., xpm) so skip the image/ to get the Qt image format used to encode
    // the m_pixmap image.
    String format = mimeType.substring(sizeof "image");

    int compressionQuality = -1;
    if (format == "jpeg" || format == "webp") {
        compressionQuality = 100;
        if (quality && *quality >= 0.0 && *quality <= 1.0)
            compressionQuality = static_cast<int>(*quality * 100 + 0.5);
    }

    QBuffer buffer(&data);
    buffer.open(QBuffer::WriteOnly);
    bool success = image.save(&buffer, format.utf8().data(), compressionQuality);
    buffer.close();

    return success;
}

// QTFIXME: Use PreserveResolution?
String ImageBufferQtBackend::toDataURL(const String& mimeType, std::optional<double> quality, PreserveResolution) const
{
    RefPtr<Image> image = copyImage(DontCopyBackingStore);
    QByteArray data;
    if (!encodeImage(image->nativeImageForCurrentFrame()->platformImage(), mimeType, quality, data))
        return "data:,";

    return "data:" + mimeType + ";base64," + data.toBase64().data();
}

Vector<uint8_t> ImageBufferQtBackend::toData(const String& mimeType, std::optional<double> quality) const
{
    RefPtr<Image> image = copyImage(DontCopyBackingStore);
    QByteArray data;
    if (!encodeImage(image->nativeImageForCurrentFrame()->platformImage(), mimeType, quality, data))
        return { };

    Vector<uint8_t> result(data.size());
    memcpy(result.data(), data.constData(), data.size());
    return result;
}

void ImageBufferQtBackend::transformColorSpace(ColorSpace srcColorSpace, ColorSpace destColorSpace)
{
    if (srcColorSpace == destColorSpace)
        return;

    // only sRGB <-> linearRGB are supported at the moment
    if ((srcColorSpace != ColorSpace::LinearRGB && srcColorSpace != ColorSpace::SRGB)
        || (destColorSpace != ColorSpace::LinearRGB && destColorSpace != ColorSpace::SRGB))
        return;

    if (destColorSpace == ColorSpace::LinearRGB) {
        static const std::array<uint8_t, 256> linearRgbLUT = [] {
            std::array<uint8_t, 256> array;
            for (unsigned i = 0; i < 256; i++) {
                float color = i / 255.0f;
                color = sRGBToLinearColorComponent(color);
                array[i] = static_cast<uint8_t>(round(color * 255));
            }
            return array;
        }();
        platformTransformColorSpace(linearRgbLUT);
    } else if (destColorSpace == ColorSpace::SRGB) {
        static const std::array<uint8_t, 256> deviceRgbLUT= [] {
            std::array<uint8_t, 256> array;
            for (unsigned i = 0; i < 256; i++) {
                float color = i / 255.0f;
                color = linearToSRGBColorComponent(color);
                array[i] = static_cast<uint8_t>(round(color * 255));
            }
            return array;
        }();
        platformTransformColorSpace(deviceRgbLUT);
    }
}

void ImageBufferQtBackend::platformTransformColorSpace(const std::array<uint8_t, 256>& lookUpTable)
{
    QPainter* painter = context().platformContext();

    QImage image = m_nativeImage.convertToFormat(QImage::Format_ARGB32);
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

Vector<uint8_t> ImageBufferQtBackend::toBGRAData() const
{
    return ImageBufferBackend::toBGRAData(const_cast<void*>(reinterpret_cast<const void*>(m_nativeImage.bits())));
}

RefPtr<ImageData> ImageBufferQtBackend::getImageData(AlphaPremultiplication outputFormat, const IntRect& srcRect) const
{
    return ImageBufferBackend::getImageData(outputFormat, srcRect, const_cast<void*>(reinterpret_cast<const void*>(m_nativeImage.bits())));
}

void ImageBufferQtBackend::putImageData(AlphaPremultiplication inputFormat, const ImageData& imageData, const IntRect& srcRect, const IntPoint& destPoint)
{
    ImageBufferBackend::putImageData(inputFormat, imageData, srcRect, destPoint, const_cast<void*>(reinterpret_cast<const void*>(m_nativeImage.bits())));
}

} // namespace WebCore
