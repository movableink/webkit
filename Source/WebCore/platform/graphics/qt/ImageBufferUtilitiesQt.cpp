/*
 Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)

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
#include "ImageBufferUtilitiesQt.h"
#include "MIMETypeRegistry.h"
#include <wtf/text/WTFString.h>

#include <QBuffer>

namespace WebCore {

static bool encodeImage(const QImage& image, const String& mimeType, std::optional<double> quality, QByteArray& data)
{
    //ASSERT(MIMETypeRegistry::isSupportedImageMIMETypeForEncoding(mimeType));

    if (!MIMETypeRegistry::isSupportedImageMIMETypeForEncoding(mimeType))
    {
        qWarning("Unsupported mime type '%s'", mimeType.utf8().data());
        return false;
    }

    // QImageWriter does not support mimetypes. It does support Qt image formats (png,
    // gif, jpeg..., xpm) so skip the image/ to get the Qt image format used to encode
    // the m_pixmap image.
    String format = mimeType.substring(sizeof "image");

    int compressionQuality = -1;
    if (format == "jpeg"_s || format == "webp"_s) {
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

Vector<uint8_t> encodeData(const QImage& image, const String& mimeType, std::optional<double> quality)
{
    QByteArray encodedImage;
    if (image.isNull() || !encodeImage(image, mimeType, quality, encodedImage))
        return { };
    size_t imageSize(encodedImage.size());
    return { reinterpret_cast<const uint8_t*>(encodedImage.constData()), imageSize };
}

} // namespace WebCore
