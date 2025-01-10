/*
 * Copyright (C) 2025 Igalia S.L
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * aint with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "GStreamerVideoFrameConverter.h"

#if USE(GSTREAMER)

#include "GStreamerCommon.h"
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gl/gl.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/Scope.h>

namespace WebCore {

GST_DEBUG_CATEGORY_STATIC(webkit_gst_video_frame_converter_debug);
#define GST_CAT_DEFAULT webkit_gst_video_frame_converter_debug

GStreamerVideoFrameConverter& GStreamerVideoFrameConverter::singleton()
{
    static NeverDestroyed<GStreamerVideoFrameConverter> sharedInstance;
    return sharedInstance;
}

GStreamerVideoFrameConverter::GStreamerVideoFrameConverter()
{
    ensureGStreamerInitialized();
    GST_DEBUG_CATEGORY_INIT(webkit_gst_video_frame_converter_debug, "webkitvideoframeconverter", 0, "WebKit GStreamer Video Frame Converter");
    m_pipeline = gst_element_factory_make("pipeline", "video-frame-converter");
    m_src = makeGStreamerElement("appsrc", nullptr);
    auto gldownload = makeGStreamerElement("gldownload", nullptr);
    auto videoconvert = makeGStreamerElement("videoconvert", nullptr);
    auto videoscale = makeGStreamerElement("videoscale", nullptr);
    m_sink = makeGStreamerElement("appsink", nullptr);

    if (webkitGstCheckVersion(1, 24, 0)) {
        g_object_set(m_sink.get(), "emit-signals", TRUE, nullptr);
        g_signal_connect(m_sink.get(), "propose-allocation", G_CALLBACK(+[](GstElement*, GstQuery* query) -> gboolean {
            gst_query_add_allocation_meta(query, GST_VIDEO_META_API_TYPE, nullptr);
            return TRUE;
        }), nullptr);
    }

    gst_bin_add_many(GST_BIN_CAST(m_pipeline.get()), m_src.get(), gldownload, videoconvert, videoscale, m_sink.get(), nullptr);
    gst_element_link_many(m_src.get(), gldownload, videoconvert, videoscale, m_sink.get(), nullptr);
}

GRefPtr<GstSample> GStreamerVideoFrameConverter::convert(const GRefPtr<GstSample>& sample, const GRefPtr<GstCaps>& destinationCaps)
{
    auto inputCaps = gst_sample_get_caps(sample.get());
    if (gst_caps_is_equal(inputCaps, destinationCaps.get()))
        return GRefPtr(sample);

    if (!setGstElementGLContext(m_sink.get(), GST_GL_DISPLAY_CONTEXT_TYPE))
        return nullptr;
    if (!setGstElementGLContext(m_sink.get(), "gst.gl.app_context"))
        return nullptr;

    unsigned capsSize = gst_caps_get_size(destinationCaps.get());
    auto newCaps = adoptGRef(gst_caps_new_empty());
    for (unsigned i = 0; i < capsSize; i++) {
        auto structure = gst_caps_get_structure(destinationCaps.get(), i);
        auto modifiedStructure = gst_structure_copy(structure);
        gst_structure_remove_field(modifiedStructure, "framerate");
        gst_caps_append_structure(newCaps.get(), modifiedStructure);
    }

    GST_TRACE_OBJECT(m_pipeline.get(), "Converting sample with caps %" GST_PTR_FORMAT " to %" GST_PTR_FORMAT, inputCaps, newCaps.get());
    g_object_set(m_sink.get(), "caps", newCaps.get(), nullptr);

    auto scopeExit = WTF::makeScopeExit([&] {
        gst_element_set_state(m_pipeline.get(), GST_STATE_NULL);
    });

    gst_element_set_state(m_pipeline.get(), GST_STATE_PAUSED);
    gst_app_src_push_sample(GST_APP_SRC_CAST(m_src.get()), sample.get());

    auto bus = adoptGRef(gst_element_get_bus(m_pipeline.get()));
    auto message = adoptGRef(gst_bus_timed_pop_filtered(bus.get(), GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_ASYNC_DONE)));
    RELEASE_ASSERT(message);
    if (GST_MESSAGE_TYPE(message.get()) == GST_MESSAGE_ERROR) {
        GST_ERROR_OBJECT(m_pipeline.get(), "Unable to convert video frame. Error: %" GST_PTR_FORMAT, message.get());
        return nullptr;
    }

    auto convertedSample = adoptGRef(gst_app_sink_pull_preroll(GST_APP_SINK_CAST(m_sink.get())));
    gst_sample_set_caps(convertedSample.get(), destinationCaps.get());
    return convertedSample;
}

#undef GST_CAT_DEFAULT

} // namespace WebCore

#endif
