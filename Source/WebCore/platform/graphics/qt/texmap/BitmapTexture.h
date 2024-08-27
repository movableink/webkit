/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2014 Igalia S.L.
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

#include "IntPoint.h"
#include "IntRect.h"
#include "IntSize.h"
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/OptionSet.h>
#include <wtf/RefPtr.h>

namespace WebCore {

class FilterOperations;
class GraphicsLayer;
class NativeImage;
class TextureMapper;
enum class TextureMapperFlags : uint16_t;

// A 2D texture that can be the target of software or GL rendering.
class BitmapTexture : public ThreadSafeRefCounted<BitmapTexture> {
public:
    enum class Flags : uint8_t {
        SupportsAlpha = 1 << 0,
        DepthBuffer = 1 << 1,
    };

    static Ref<BitmapTexture> create(const IntSize& size, OptionSet<Flags> flags = { })
    {
        return adoptRef(*new BitmapTexture(size, flags));
    }

    WEBCORE_EXPORT ~BitmapTexture() = default;

    const IntSize& size() const { return m_size; };
    OptionSet<Flags> flags() const { return m_flags; }
    bool isOpaque() const { return !m_flags.contains(Flags::SupportsAlpha); }

    void updateContents(RefPtr<NativeImage>, const IntRect&, const IntPoint& offset) { }
    void updateContents(GraphicsLayer*, const IntRect& target, const IntPoint& offset, float scale = 1);
    void updateContents(const void*, const IntRect& target, const IntPoint& offset, int bytesPerLine) { }

    void bindAsSurface();
    void initializeStencil();
    void initializeDepthBuffer();

    void reset(const IntSize& size, OptionSet<Flags> flags = { })
    {
        m_flags = flags;
        m_size = size;
    }

private:
    BitmapTexture(const IntSize& size, OptionSet<Flags> flags) : m_size(size), m_flags(flags) {}

    IntSize m_size;
    OptionSet<Flags> m_flags;
};

}
