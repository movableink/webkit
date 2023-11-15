/*
 * Copyright (C) 2008 Google Inc. All rights reserved.
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

#pragma once

#include "GraphicsContextQt.h"
#include "ImageBufferBackend.h"
#include "PlatformImage.h"
#include "PixelBuffer.h"
#include <wtf/IsoMalloc.h>
#include <QPainter>

namespace WebCore {

class WEBCORE_EXPORT ImageBufferQtBackend : public ImageBufferBackend {
    WTF_MAKE_ISO_ALLOCATED(ImageBufferQtBackend);
    WTF_MAKE_NONCOPYABLE(ImageBufferQtBackend);
public:
    ImageBufferQtBackend(const Parameters& parameters, std::unique_ptr<GraphicsContext>&& context, std::unique_ptr<QImage>&& nativeImage, Ref<Image> image);
    ~ImageBufferQtBackend() { context().platformContext()->painter()->end(); }

    static std::unique_ptr<ImageBufferQtBackend> create(const Parameters&, const ImageBufferCreationContext&);
    static std::unique_ptr<ImageBufferQtBackend> create(const Parameters&, const GraphicsContext &);

    static size_t calculateMemoryCost(const Parameters&);
    static unsigned calculateBytesPerRow(const IntSize& backendSize);
    static size_t calculateExternalMemoryCost(const Parameters&);

    RefPtr<NativeImage> copyNativeImage() override;
    RefPtr<NativeImage> createNativeImageReference() override;

    void transformToColorSpace(const DestinationColorSpace&) override;

    GraphicsContext& context() override;
    String debugDescription() const override;

    void getPixelBuffer(const IntRect& srcRect, PixelBuffer& destination) override;
    void putPixelBuffer(const PixelBuffer&, const IntRect& srcRect, const IntPoint& destPoint, AlphaPremultiplication destFormat) override;

protected:
    using ImageBufferBackend::ImageBufferBackend;

    static void initPainter(QPainter *painter);
    void platformTransformColorSpace(const std::array<uint8_t, 256>& lookUpTable);
    unsigned bytesPerRow() const override;

    RefPtr<Image> m_image;
    std::unique_ptr<QImage> m_nativeImage;
    std::unique_ptr<GraphicsContext> m_context;
};

} // namespace WebCore
