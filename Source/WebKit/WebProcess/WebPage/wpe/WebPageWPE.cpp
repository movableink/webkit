/*
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

#include "config.h"
#include "WebPage.h"

#include "DrawingArea.h"
#include "WebPageProxy.h"
#include "WebPageProxyMessages.h"
#include <WebCore/NotImplemented.h>
#include <WebCore/PlatformScreen.h>
#include <WebCore/PointerCharacteristics.h>

namespace WebKit {
using namespace WebCore;

void WebPage::platformReinitialize()
{
}

bool WebPage::platformCanHandleRequest(const ResourceRequest&)
{
    notImplemented();
    return false;
}

bool WebPage::hoverSupportedByPrimaryPointingDevice() const
{
    return WebProcess::singleton().primaryPointingDevice() == AvailableInputDevices::Mouse;
}

bool WebPage::hoverSupportedByAnyAvailablePointingDevice() const
{
    return WebProcess::singleton().availableInputDevices().contains(AvailableInputDevices::Mouse);
}

std::optional<PointerCharacteristics> WebPage::pointerCharacteristicsOfPrimaryPointingDevice() const
{
    const auto& primaryPointingDevice = WebProcess::singleton().primaryPointingDevice();
    if (primaryPointingDevice == AvailableInputDevices::Mouse)
        return PointerCharacteristics::Fine;
    if (primaryPointingDevice == AvailableInputDevices::Touchscreen)
        return PointerCharacteristics::Coarse;
    return std::nullopt;
}

OptionSet<PointerCharacteristics> WebPage::pointerCharacteristicsOfAllAvailablePointingDevices() const
{
    OptionSet<PointerCharacteristics> pointerCharacteristics;
    const auto& availableInputs = WebProcess::singleton().availableInputDevices();
    if (availableInputs.contains(AvailableInputDevices::Mouse))
        pointerCharacteristics.add(PointerCharacteristics::Fine);
    if (availableInputs.contains(AvailableInputDevices::Touchscreen))
        pointerCharacteristics.add(PointerCharacteristics::Coarse);
    return pointerCharacteristics;
}

#if USE(GBM) && ENABLE(WPE_PLATFORM)
void WebPage::preferredBufferFormatsDidChange(Vector<DMABufRendererBufferFormat>&& preferredBufferFormats)
{
    m_preferredBufferFormats = WTFMove(preferredBufferFormats);
    if (m_drawingArea)
        m_drawingArea->preferredBufferFormatsDidChange();
}
#endif

} // namespace WebKit
