/*
 * Copyright (C) 2025 Igalia S.L.
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
#include "WebPasteboardProxy.h"

#if PLATFORM(WPE)
#include "Connection.h"
#include <WebCore/Pasteboard.h>
#include <WebCore/PasteboardCustomData.h>
#include <WebCore/PasteboardItemInfo.h>
#include <WebCore/PlatformPasteboard.h>
#include <WebCore/SelectionData.h>

#if ENABLE(WPE_PLATFORM)
#include <wpe/wpe-platform.h>
#endif

namespace WebKit {
using namespace WebCore;

#if ENABLE(WPE_PLATFORM)
static inline bool usingWPEPlatformAPI()
{
    return !!g_type_class_peek(WPE_TYPE_DISPLAY);
}
#endif

void WebPasteboardProxy::getTypes(const String& pasteboardName, CompletionHandler<void(Vector<String>&&)>&& completionHandler)
{
#if ENABLE(WPE_PLATFORM)
    if (usingWPEPlatformAPI()) {
        completionHandler({ });
        return;
    }
#endif

    Vector<String> pasteboardTypes;
    PlatformPasteboard().getTypes(pasteboardTypes);
    completionHandler(WTFMove(pasteboardTypes));
}

void WebPasteboardProxy::readText(IPC::Connection& connection, const String& pasteboardName, const String& pasteboardType, CompletionHandler<void(String&&)>&& completionHandler)
{
#if ENABLE(WPE_PLATFORM)
    if (usingWPEPlatformAPI()) {
        completionHandler(String());
        return;
    }
#endif

    completionHandler(PlatformPasteboard().readString(0, pasteboardType.startsWith("text/plain"_s) ? "text/plain;charset=utf-8"_s : pasteboardType));
}

void WebPasteboardProxy::readFilePaths(IPC::Connection& connection, const String& pasteboardName, CompletionHandler<void(Vector<String>&&)>&& completionHandler)
{
    completionHandler({ });
}

void WebPasteboardProxy::readBuffer(IPC::Connection& connection, const String& pasteboardName, const String& pasteboardType, CompletionHandler<void(RefPtr<SharedBuffer>&&)>&& completionHandler)
{
    completionHandler({ });
}

void WebPasteboardProxy::writeToClipboard(const String& pasteboardName, SelectionData&& selectionData)
{
#if ENABLE(WPE_PLATFORM)
    if (usingWPEPlatformAPI())
        return;
#endif

    PasteboardWebContent contents;
    if (selectionData.hasText())
        contents.text = selectionData.text();
    if (selectionData.hasMarkup())
        contents.markup = selectionData.markup();
    PlatformPasteboard().write(contents);
}

void WebPasteboardProxy::clearClipboard(const String& pasteboardName)
{
}

void WebPasteboardProxy::typesSafeForDOMToReadAndWrite(IPC::Connection& connection, const String& pasteboardName, const String& origin, std::optional<WebPageProxyIdentifier>, CompletionHandler<void(Vector<String>&&)>&& completionHandler)
{
    completionHandler({ });
}

void WebPasteboardProxy::writeCustomData(IPC::Connection&, const Vector<PasteboardCustomData>& data, const String& pasteboardName, std::optional<WebPageProxyIdentifier>, CompletionHandler<void(int64_t)>&& completionHandler)
{
#if ENABLE(WPE_PLATFORM)
    if (usingWPEPlatformAPI()) {
        completionHandler(0);
        return;
    }
#endif

    completionHandler(PlatformPasteboard().write(data));
}

void WebPasteboardProxy::allPasteboardItemInfo(IPC::Connection&, const String& pasteboardName, int64_t changeCount, std::optional<WebPageProxyIdentifier>, CompletionHandler<void(std::optional<Vector<WebCore::PasteboardItemInfo>>&&)>&& completionHandler)
{
    completionHandler(std::nullopt);
}

void WebPasteboardProxy::informationForItemAtIndex(IPC::Connection&, size_t index, const String& pasteboardName, int64_t changeCount, std::optional<WebPageProxyIdentifier>, CompletionHandler<void(std::optional<WebCore::PasteboardItemInfo>&&)>&& completionHandler)
{
    completionHandler(std::nullopt);
}

void WebPasteboardProxy::getPasteboardItemsCount(IPC::Connection&, const String& pasteboardName, std::optional<WebPageProxyIdentifier>, CompletionHandler<void(uint64_t)>&& completionHandler)
{
    completionHandler(0);
}

void WebPasteboardProxy::readURLFromPasteboard(IPC::Connection& connection, size_t index, const String& pasteboardName, std::optional<WebPageProxyIdentifier>, CompletionHandler<void(String&& url, String&& title)>&& completionHandler)
{
    completionHandler({ }, { });
}

void WebPasteboardProxy::readBufferFromPasteboard(IPC::Connection& connection, std::optional<size_t> index, const String& pasteboardType, const String& pasteboardName, std::optional<WebPageProxyIdentifier>, CompletionHandler<void(RefPtr<WebCore::SharedBuffer>&&)>&& completionHandler)
{
    completionHandler({ });
}

void WebPasteboardProxy::getPasteboardChangeCount(IPC::Connection&, const String& pasteboardName, CompletionHandler<void(int64_t)>&& completionHandler)
{
#if ENABLE(WPE_PLATFORM)
    if (usingWPEPlatformAPI()) {
        completionHandler(0);
        return;
    }
#endif

    completionHandler(PlatformPasteboard().changeCount());
}

} // namespace WebKit

#endif // PLATFORM(WPE)
