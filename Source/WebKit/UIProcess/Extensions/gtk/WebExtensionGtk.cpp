/*
 * Copyright (C) 2024 Igalia S.L. All rights reserved.
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

#include "config.h"
#include "WebExtension.h"

#if ENABLE(WK_WEB_EXTENSIONS)

#include "Logging.h"
#include "WebExtensionLocalization.h"
#include "WebExtensionUtilities.h"
#include <WebCore/LocalizedStrings.h>
#include <gtk/gtk.h>
#include <wtf/FileSystem.h>
#include <wtf/Language.h>
#include <wtf/glib/GRefPtr.h>
#include <wtf/text/Base64.h>

namespace WebKit {

static constexpr auto generatedBackgroundPageFilename = "_generated_background_page.html"_s;
static constexpr auto generatedBackgroundServiceWorkerFilename = "_generated_service_worker.js"_s;

WebExtension::WebExtension(const JSON::Value& manifest, Resources&& resources)
    : m_manifestJSON(manifest)
    , m_resources(WTFMove(resources))
{
    auto manifestString = manifest.toJSONString();
    RELEASE_ASSERT(manifestString);

    m_resources.set("manifest.json"_s, manifestString);
}

RefPtr<API::Data> WebExtension::resourceDataForPath(const String& originalPath, RefPtr<API::Error>& outError, CacheResult cacheResult, SuppressNotFoundErrors suppressErrors)
{
    ASSERT(originalPath);

    outError = nullptr;

    String path = originalPath;

    // Remove leading slash to normalize the path for lookup/storage in the cache dictionary.
    if (path.startsWith('/'))
        path = path.substring(1);

    if (path.startsWith("data:"_s)) {
        if (auto base64Position = path.find(";base64,"_s); base64Position != notFound) {
            auto base64String = base64DecodeToString(path.substring(base64Position));
            return API::Data::create(base64String.utf8().span());
        }

        if (auto commaPosition = path.find(','); commaPosition != notFound) {
            auto urlEncodedString = path.substring(commaPosition);
            auto decodedString = URL(urlEncodedString).string();
            return API::Data::create(decodedString.utf8().span());
        }

        ASSERT(path == "data:"_s);
        return API::Data::create(std::span<const uint8_t> { });
    }

    if (path == generatedBackgroundPageFilename || path  == generatedBackgroundServiceWorkerFilename)
        return API::Data::create(generatedBackgroundContent().utf8().span());

    if (auto entry = m_resources.find(path); entry != m_resources.end()) {
        return WTF::switchOn(entry->value,
            [](const Ref<API::Data>& data) {
                return data;
            },
            [](const String& string) {
                return API::Data::create(string.utf8().span());
            });
    }

    auto resourceURL = resourceFileURLForPath(path);
    if (!resourceURL.isEmpty()) {
        if (suppressErrors == SuppressNotFoundErrors::No)
            outError = createError(Error::ResourceNotFound, WEB_UI_FORMAT_STRING("Unable to find \"%s\" in the extension’s resources. It is an invalid path.", "WKWebExtensionErrorResourceNotFound description with invalid file path", path.utf8().data()));
        return nullptr;
    }

    auto rawData = FileSystem::readEntireFile((resourceURL).fileSystemPath());
    if (!rawData.has_value()) {
        if (suppressErrors == SuppressNotFoundErrors::No)
            outError = createError(Error::ResourceNotFound, WEB_UI_FORMAT_STRING("Unable to find \"%s\" in the extension’s resources.", "WKWebExtensionErrorResourceNotFound description with file name", path.utf8().data()));
        return nullptr;
    }

    auto data = API::Data::create(*rawData);

    if (cacheResult == CacheResult::Yes)
        m_resources.set(path, data);

    return data;
}

void WebExtension::recordError(Ref<API::Error> error)
{
    RELEASE_LOG_ERROR(Extensions, "Error recorded: %s", error->platformError());

    // Only the first occurrence of each error is recorded in the array. This prevents duplicate errors,
    // such as repeated "resource not found" errors, from being included multiple times.
    if (m_errors.contains(error))
        return;

    m_errors.append(error);
}

RefPtr<WebCore::Icon> WebExtension::iconForPath(const String& path, RefPtr<API::Error>& outError, WebCore::FloatSize sizeForResizing, std::optional<double> idealDisplayScale)
{
    ASSERT(path);

    auto imageData = resourceDataForPath(path, outError);
    if (imageData->span().empty())
        return nullptr;

    auto gimageBytes = adoptGRef(g_bytes_new(imageData->span().data(), imageData->size()));

    if (!sizeForResizing.isZero()) {
        GUniqueOutPtr<GError> error;

        auto loader = adoptGRef(gdk_pixbuf_loader_new());
        gdk_pixbuf_loader_write_bytes(loader.get(), gimageBytes.get(), &error.outPtr());
        if (error) {
            RELEASE_LOG_ERROR(Extensions, "Unknown error when loading an icon: %s", error.get()->message);
            outError = createError(Error::Unknown);
        }
        if (!gdk_pixbuf_loader_close(loader.get(), &error.outPtr()) && error) {
            RELEASE_LOG_ERROR(Extensions, "Unknown error when loading an icon: %s", error.get()->message);
            outError = createError(Error::Unknown);
        }
        auto pixbuf = adoptGRef(gdk_pixbuf_copy(gdk_pixbuf_loader_get_pixbuf(loader.get())));
        if (!pixbuf)
            return nullptr;

        // Proportionally scale the size
        auto originalWidth = gdk_pixbuf_get_width(pixbuf.get());
        auto originalHeight = gdk_pixbuf_get_height(pixbuf.get());
        auto aspectWidth = originalWidth ? (sizeForResizing.width() / originalWidth) : 0;
        auto aspectHeight = originalHeight ? (sizeForResizing.height() / originalHeight) : 0;
        auto aspectRatio = std::min(aspectWidth, aspectHeight);

        gdk_pixbuf_scale_simple(pixbuf.get(), originalWidth * aspectRatio, originalHeight * aspectRatio, GDK_INTERP_BILINEAR);

        gchar* buffer;
        gsize bufferSize;
        if (!gdk_pixbuf_save_to_buffer(pixbuf.get(), &buffer, &bufferSize, "png", &error.outPtr(), nullptr) && error) {
            RELEASE_LOG_ERROR(Extensions, "Unknown error when loading an icon: %s", error.get()->message);
            outError = createError(Error::Unknown);
        }

        gimageBytes = adoptGRef(g_bytes_new_take(buffer, bufferSize));
    }

    GRefPtr<GIcon> image = adoptGRef(g_bytes_icon_new(gimageBytes.get()));

    return WebCore::Icon::create(WTFMove(image));
}

RefPtr<WebCore::Icon> WebExtension::bestIcon(RefPtr<JSON::Object> icons, WebCore::FloatSize idealSize, const Function<void(Ref<API::Error>)>& reportError)
{
    if (!icons)
        return nullptr;

    auto idealPointSize = idealSize.width() > idealSize.height() ? idealSize.width() : idealSize.height();
    auto bestScale = largestDisplayScale();

    auto pixelSize = idealPointSize * bestScale;
    auto iconPath = pathForBestImage(*icons, pixelSize);
    if (iconPath.isEmpty())
        return nullptr;

    RefPtr<API::Error> resourceError;
    if (RefPtr image = iconForPath(iconPath, resourceError, idealSize))
        return image;

    if (reportError && resourceError)
        reportError(*resourceError);

    return nullptr;
}

} // namespace WebKit

#endif // ENABLE(WK_WEB_EXTENSIONS)
