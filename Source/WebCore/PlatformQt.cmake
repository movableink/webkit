include(platform/ImageDecoders.cmake)
include(platform/graphics/qt/texmap/TextureMapper.cmake)

set(WebCore_OUTPUT_NAME WebCore)

if (NOT USE_LIBJPEG)
    list(REMOVE_ITEM WebCore_SOURCES
        platform/image-decoders/jpeg/JPEGImageDecoder.cpp
    )
endif ()

if (JPEG_DEFINITIONS)
    add_definitions(${JPEG_DEFINITIONS})
endif ()

list(APPEND WebCore_PRIVATE_INCLUDE_DIRECTORIES
    "${WEBCORE_DIR}/bridge/qt"
    "${WEBCORE_DIR}/dom/qt"
    "${WEBCORE_DIR}/editing/qt"
    "${WEBCORE_DIR}/history/qt"
    "${WEBCORE_DIR}/page/qt"
    "${WEBCORE_DIR}/platform/qt"
    "${WEBCORE_DIR}/platform/audio/qt"
    "${WEBCORE_DIR}/platform/glib"
    "${WEBCORE_DIR}/platform/graphics/egl"
    "${WEBCORE_DIR}/platform/graphics/glx"
    "${WEBCORE_DIR}/platform/graphics/gpu/qt"
    "${WEBCORE_DIR}/platform/graphics/opengl"
    "${WEBCORE_DIR}/platform/graphics/surfaces"
    "${WEBCORE_DIR}/platform/graphics/surfaces/qt"
    "${WEBCORE_DIR}/platform/graphics/qt"
    "${WEBCORE_DIR}/platform/graphics/win"
    "${WEBCORE_DIR}/platform/network/qt"
    "${WEBCORE_DIR}/platform/network/glib"
    "${WEBCORE_DIR}/platform/text/qt"
    "${WEBCORE_DIR}/platform/video-codecs"
    "${WEBCORE_DIR}/platform/win"
    "${WEBCORE_DIR}/platform/graphics/x11"
)

list(APPEND WebCore_PRIVATE_FRAMEWORK_HEADERS
    bridge/qt/qt_instance.h
    bridge/qt/qt_runtime.h

    bindings/js/ScriptSourceCode.h
    bindings/js/CachedScriptSourceProvider.h
    bindings/js/ScriptBufferSourceProvider.h

    platform/qt/QtBytecodeCacheDelegate.h
    platform/qt/QtDiskCacheDelegate.h

    dom/StaticNodeList.h

    inspector/LegacyWebSocketInspectorInstrumentation.h

    loader/NavigationScheduler.h

    loader/cache/CachedScript.h

    page/SpatialNavigation.h

    platform/glib/ApplicationGLib.h

    platform/graphics/MediaPlayerPrivate.h

    platform/graphics/qt/MediaPlayerPrivateQt.h
    platform/graphics/qt/ImageBufferQtBackend.h
    platform/graphics/qt/ImageBufferUtilitiesQt.h
    platform/graphics/qt/GraphicsContextQt.h
    platform/graphics/qt/TransparencyLayer.h
    platform/graphics/qt/NativeImageQt.h

    platform/mock/GeolocationClientMock.h

    platform/network/MIMESniffing.h
    platform/network/ResourceHandleInternal.h

    platform/network/qt/AuthenticationChallenge.h
    platform/network/qt/CertificateInfo.h
    platform/network/qt/QNetworkReplyHandler.h
    platform/network/qt/QtMIMETypeSniffer.h
    platform/network/qt/ResourceError.h
    platform/network/qt/ResourceRequest.h
    platform/network/qt/ResourceResponse.h
    platform/network/qt/SharedCookieJarQt.h

    platform/qt/KeyedDecoderQt.h
    platform/qt/KeyedEncoderQt.h
    platform/qt/PlatformGestureEvent.h
    platform/qt/QStyleFacade.h
    platform/qt/QStyleHelpers.h
    platform/qt/QWebPageClient.h
    platform/qt/RenderThemeQt.h
    platform/qt/RenderThemeQStyle.h
    platform/qt/ScrollbarThemeQStyle.h
    platform/qt/ThirdPartyCookiesQt.h
    platform/qt/UserAgentQt.h

    platform/sql/SQLiteDatabase.h

    testing/js/WebCoreTestSupport.h
)

list(APPEND WebCore_SOURCES
    accessibility/qt/AccessibilityObjectQt.cpp
    accessibility/qt/AXObjectCacheQt.cpp

    bindings/js/CachedScriptSourceProviderQt.cpp
    bindings/js/ScriptControllerQt.cpp

    bridge/qt/qt_class.cpp
    bridge/qt/qt_instance.cpp
    bridge/qt/qt_pixmapruntime.cpp
    bridge/qt/qt_runtime.cpp

    dom/qt/GestureEvent.cpp

    editing/qt/EditorQt.cpp

    inspector/LegacyWebSocketInspectorInstrumentation.cpp

    page/qt/DragControllerQt.cpp
    page/qt/EventHandlerQt.cpp

    platform/ScrollAnimationKinetic.cpp
    platform/ScrollAnimationSmooth.cpp

    platform/audio/qt/AudioBusQt.cpp

    platform/graphics/ImageSource.cpp
#    platform/graphics/PlatformDisplay.cpp
    platform/graphics/WOFFFileFormat.cpp

    platform/graphics/cairo/FontCairoHarfbuzzNG.cpp

    platform/graphics/harfbuzz/ComplexTextControllerHarfBuzz.cpp
    platform/graphics/harfbuzz/FontDescriptionHarfBuzz.cpp

#    platform/graphics/texmap/BitmapTextureImageBuffer.cpp
#    platform/graphics/texmap/TextureMapperImageBuffer.cpp

    platform/graphics/qt/ColorQt.cpp
    platform/graphics/qt/FloatPointQt.cpp
    platform/graphics/qt/FloatRectQt.cpp
    platform/graphics/qt/FloatSizeQt.cpp
    platform/graphics/qt/FontCacheQt.cpp
    platform/graphics/qt/FontCascadeQt.cpp
    platform/graphics/qt/FontCustomPlatformDataQt.cpp
    platform/graphics/qt/FontPlatformDataQt.cpp
    platform/graphics/qt/FontQt.cpp
    platform/graphics/qt/GlyphPageTreeNodeQt.cpp
    platform/graphics/qt/GradientQt.cpp
    platform/graphics/qt/GraphicsContextQt.cpp
    platform/graphics/qt/IconQt.cpp
    platform/graphics/qt/ImageAdapterQt.cpp
    platform/graphics/qt/ImageBufferQtBackend.cpp
    platform/graphics/qt/ImageBufferUtilitiesQt.cpp
    platform/graphics/qt/IntPointQt.cpp
    platform/graphics/qt/IntRectQt.cpp
    platform/graphics/qt/IntSizeQt.cpp
    platform/graphics/qt/NativeImageQt.cpp
    platform/graphics/qt/PathQt.cpp
    platform/graphics/qt/PatternQt.cpp
    platform/graphics/qt/ShareableBitmapQt.cpp
    platform/graphics/qt/StillImageQt.cpp
    platform/graphics/qt/SystemFontDatabaseQt.cpp
    platform/graphics/qt/TileQt.cpp
    platform/graphics/qt/TransformationMatrixQt.cpp

    platform/image-decoders/qt/ImageBackingStoreQt.cpp

    platform/network/MIMESniffing.cpp

    platform/network/qt/BlobUrlConversion.cpp
    platform/network/qt/CookieStorageQt.cpp
    platform/network/qt/DNSResolveQueueQt.cpp
    platform/network/qt/NetworkStateNotifierQt.cpp
    platform/network/qt/NetworkStorageSessionQt.cpp
    platform/network/qt/PublicSuffixStoreQt.cpp
    platform/network/qt/QNetworkReplyHandler.cpp
    platform/network/qt/QtMIMETypeSniffer.cpp
    platform/network/qt/ResourceHandleQt.cpp
    platform/network/qt/ResourceRequestQt.cpp
    platform/network/qt/ResourceResponseQt.cpp
    platform/network/qt/SharedCookieJarQt.cpp
    platform/network/qt/SynchronousLoaderClientQt.cpp

    platform/qt/ApplicationQt.cpp
    platform/qt/CursorQt.cpp
    platform/qt/DataTransferItemListQt.cpp
    platform/qt/DataTransferItemQt.cpp
    platform/qt/DragDataQt.cpp
    platform/qt/DragImageQt.cpp
    platform/qt/KeyedDecoderQt.cpp
    platform/qt/KeyedEncoderQt.cpp
    platform/qt/LocalizedStringsQt.cpp
    platform/qt/LoggingQt.cpp
    platform/qt/MainThreadSharedTimerQt.cpp
    platform/qt/MIMETypeRegistryQt.cpp
    platform/qt/PasteboardQt.cpp
    platform/qt/PlatformKeyboardEventQt.cpp
    platform/qt/PlatformScreenQt.cpp
    platform/qt/QStyleHelpers.cpp
    platform/qt/QtBytecodeCacheDelegate.cpp
    platform/qt/QtDiskCacheDelegate.cpp
    platform/qt/RenderThemeQStyle.cpp
    platform/qt/RenderThemeQt.cpp
    platform/qt/RenderThemeQtMobile.cpp
    platform/qt/ScrollViewQt.cpp
    platform/qt/ScrollbarThemeQStyle.cpp
    platform/qt/ScrollbarThemeQt.cpp
    platform/qt/SharedBufferQt.cpp
    platform/qt/TemporaryLinkStubsQt.cpp
    platform/qt/ThemeQt.cpp
    platform/qt/ThirdPartyCookiesQt.cpp
    platform/qt/UserAgentQt.cpp
    platform/qt/WidgetQt.cpp

    platform/text/Hyphenation.cpp
    platform/text/LocaleICU.cpp

    platform/text/hyphen/HyphenationLibHyphen.cpp
)

if (APPLE)
    list(APPEND WebCore_SOURCES
        editing/SmartReplaceCF.cpp
    )
endif ()

QTWEBKIT_GENERATE_MOC_FILES_CPP(WebCore
    platform/network/qt/DNSResolveQueueQt.cpp
    platform/qt/MainThreadSharedTimerQt.cpp
)

QTWEBKIT_GENERATE_MOC_FILES_H(WebCore
    platform/network/qt/QNetworkReplyHandler.h
    platform/network/qt/QtMIMETypeSniffer.h
    platform/network/qt/SharedCookieJarQt.h
)

QTWEBKIT_GENERATE_MOC_FILE_H(WebCore platform/network/qt/NetworkStateNotifierPrivate.h platform/network/qt/NetworkStateNotifierQt.cpp)

if (COMPILER_IS_GCC_OR_CLANG)
    set_source_files_properties(
        platform/network/qt/BlobUrlConversion.cpp
    PROPERTIES
        COMPILE_FLAGS "-fexceptions -UQT_NO_EXCEPTIONS"
    )
endif ()

if (ENABLE_DEVICE_ORIENTATION)
    list(APPEND WebCore_SOURCES
        platform/qt/DeviceMotionClientQt.cpp
        platform/qt/DeviceMotionProviderQt.cpp
        platform/qt/DeviceOrientationClientQt.cpp
        platform/qt/DeviceOrientationProviderQt.cpp
    )
endif ()

if (ENABLE_GRAPHICS_CONTEXT_3D)
    list(APPEND WebCore_SOURCES
        platform/graphics/qt/GraphicsContext3DQt.cpp
    )
endif ()

if (ENABLE_TOUCH_ADJUSTMENT)
    list(APPEND WebCore_SOURCES
        page/qt/TouchAdjustment.cpp
    )
endif ()

# Do it in the WebCore to support SHARED_CORE since WebKitWidgets won't load WebKitLegacy in that case.
# This should match the opposite statement in WebKitLegacy/PlatformQt.cmake
if (SHARED_CORE)
    qt6_add_resources(WebCore_SOURCES
        WebCore.qrc
    )

    if (ENABLE_INSPECTOR_UI)
        include("${CMAKE_SOURCE_DIR}/Source/WebInspectorUI/PlatformQt.cmake")
        list(APPEND WebCore_SOURCES
            "${DERIVED_SOURCES_WEBINSPECTORUI_DIR}/qrc_WebInspector.cpp"
        )
    endif ()
endif ()

# Note: Qt6Network_INCLUDE_DIRS includes Qt6Core_INCLUDE_DIRS
list(APPEND WebCore_SYSTEM_INCLUDE_DIRECTORIES
    ${HARFBUZZ_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${HYPHEN_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${LIBXSLT_INCLUDE_DIR}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
    ${Qt6Network_PRIVATE_INCLUDE_DIRS}
    ${Qt6Sensors_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

list(APPEND WebCore_LIBRARIES
    HarfBuzz::HarfBuzz
    HarfBuzz::ICU
    ${FREETYPE_LIBRARIES}
    ${HYPHEN_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${LIBXSLT_LIBRARIES}
    ${Qt6Core_LIBRARIES}
    ${Qt6Gui_LIBRARIES}
    ${Qt6Network_LIBRARIES}
    ${Qt6Sensors_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${X11_X11_LIB}
    ${ZLIB_LIBRARIES}
)

if (QT_STATIC_BUILD)
    list(APPEND WebCore_LIBRARIES
        ${STATIC_LIB_DEPENDENCIES}
    )
endif ()

list(APPEND WebCore_USER_AGENT_STYLE_SHEETS
#    ${WEBCORE_DIR}/css/mediaControlsGtk.css
#    ${WEBCORE_DIR}/css/mediaControlsQt.css
#    ${WEBCORE_DIR}/css/mediaControlsQtFullscreen.css
    ${WEBCORE_DIR}/css/mobileThemeQt.css
    ${WEBCORE_DIR}/css/themeQtNoListboxes.css
)

if (ENABLE_WEBKIT)
    list(APPEND WebCore_SOURCES
        page/qt/GestureTapHighlighter.cpp
    )
endif ()

if (ENABLE_OPENGL)
    list(APPEND WebCore_SOURCES
        platform/graphics/opengl/Extensions3DOpenGLCommon.cpp
        platform/graphics/opengl/GraphicsContext3DOpenGLCommon.cpp
        platform/graphics/opengl/TemporaryOpenGLSetting.cpp

        platform/graphics/qt/QFramebufferPaintDevice.cpp
    )

    if (${Qt6Gui_OPENGL_IMPLEMENTATION} STREQUAL GLESv2)
        list(APPEND WebCore_SOURCES
            platform/graphics/opengl/Extensions3DOpenGLES.cpp
            platform/graphics/opengl/GraphicsContext3DOpenGLES.cpp
        )
        list(APPEND WebCore_LIBRARIES
            ${Qt6Gui_EGL_LIBRARIES}
            ${Qt6Gui_OPENGL_LIBRARIES}
        )
    else ()
        list(APPEND WebCore_SOURCES
            platform/graphics/opengl/Extensions3DOpenGL.cpp
            platform/graphics/opengl/GraphicsContext3DOpenGL.cpp
        )
    endif ()
else ()
    # remove OpenGL::GLES from WebCore
    list(REMOVE_ITEM WebCore_LIBRARIES OpenGL::GLES)
endif ()

if (USE_GLIB)
    list(APPEND WebCore_SYSTEM_INCLUDE_DIRECTORIES
        ${GIO_UNIX_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
    )
    list(APPEND WebCore_LIBRARIES
        ${GLIB_GIO_LIBRARIES}
        ${GLIB_GOBJECT_LIBRARIES}
        ${GLIB_LIBRARIES}
    )
    list(APPEND WebCore_SOURCES
        platform/network/glib/DNSResolveQueueGLib.cpp
        platform/glib/UserAgentQuirks.cpp
        platform/glib/SharedBufferGlib.cpp
    )
endif ()

if (USE_GSTREAMER)
    include(platform/GStreamer.cmake)
    list(APPEND WebCore_SOURCES
        platform/graphics/gstreamer/ImageGStreamerQt.cpp
    )
endif ()

if (USE_MEDIA_FOUNDATION)
    list(APPEND WebCore_SOURCES
        platform/graphics/win/MediaPlayerPrivateMediaFoundation.cpp
    )
    list(APPEND WebCore_LIBRARIES
        mfuuid
        strmbase
    )
endif ()

if (USE_QT_MULTIMEDIA)
    list(APPEND WebCore_SOURCES
        platform/graphics/qt/MediaPlayerPrivateQt.cpp
    )
    list(APPEND WebCore_LIBRARIES
        ${Qt6Multimedia_LIBRARIES}
    )
    QTWEBKIT_GENERATE_MOC_FILES_H(WebCore platform/graphics/qt/MediaPlayerPrivateQt.h)
endif ()

if (ENABLE_VIDEO)
    list(APPEND WebCore_USER_AGENT_STYLE_SHEETS
        ${WebCore_DERIVED_SOURCES_DIR}/ModernMediaControls.css
    )

    list(APPEND WebCore_USER_AGENT_SCRIPTS
        ${WebCore_DERIVED_SOURCES_DIR}/ModernMediaControls.js
    )

    set(WebCore_USER_AGENT_SCRIPTS_DEPENDENCIES ${WEBCORE_DIR}/platform/qt/RenderThemeQt.cpp)
endif ()

if (USE_GCRYPT)
    include(platform/GCrypt.cmake)
    list(APPEND WebCore_SYSTEM_INCLUDE_DIRECTORIES
        ${LIBTASN1_INCLUDE_DIRS}
    )
    list(APPEND WebCore_LIBRARIES
        ${LIBTASN1_LIBRARIES}
    )
endif ()

if (USE_COMMONCRYPTO)
    list(APPEND WebCore_PRIVATE_FRAMEWORK_HEADERS
        crypto/CommonCryptoUtilities.h
    )

    list(APPEND WebCore_SOURCES
        crypto/CommonCryptoUtilities.cpp

        crypto/cocoa/CommonCryptoDERUtilities.cpp
        crypto/cocoa/CryptoAlgorithmAESCBCMac.cpp
        crypto/cocoa/CryptoAlgorithmAESCFBMac.cpp
        crypto/cocoa/CryptoAlgorithmAESCTRMac.cpp
        crypto/cocoa/CryptoAlgorithmAESGCMMac.cpp
        crypto/cocoa/CryptoAlgorithmAESKWMac.cpp
        crypto/cocoa/CryptoAlgorithmECDHMac.cpp
        crypto/cocoa/CryptoAlgorithmECDSAMac.cpp
        crypto/cocoa/CryptoAlgorithmHKDFMac.cpp
        crypto/cocoa/CryptoAlgorithmHMACMac.cpp
        crypto/cocoa/CryptoAlgorithmPBKDF2Mac.cpp
        crypto/cocoa/CryptoAlgorithmRSASSA_PKCS1_v1_5Mac.cpp
        crypto/cocoa/CryptoAlgorithmRSA_OAEPMac.cpp
        crypto/cocoa/CryptoAlgorithmRSA_PSSMac.cpp
        crypto/cocoa/CryptoAlgorithmRegistryMac.cpp
        crypto/cocoa/CryptoKeyECMac.cpp
        crypto/cocoa/CryptoKeyMac.cpp
        crypto/cocoa/CryptoKeyRSAMac.cpp
        crypto/cocoa/CryptoUtilitiesCocoa.cpp

        crypto/qt/SerializedCryptoKeyWrapNone.cpp
    )
endif ()

# Build the include path with duplicates removed
list(REMOVE_DUPLICATES WebCore_SYSTEM_INCLUDE_DIRECTORIES)

list(APPEND WebCoreTestSupport_LIBRARIES PRIVATE
    WebCore
)

if (HAVE_FONTCONFIG)
    list(APPEND WebCoreTestSupport_LIBRARIES
        Fontconfig::Fontconfig
    )
endif ()

# From PlatformWin.cmake



if (USE_MACH_PORTS)
    list(APPEND WebCore_SOURCES
        platform/cf/SharedBufferCF.cpp
        platform/qt/SharedMemoryMac.cpp
    )

    if (HAVE_FONTCONFIG)
        list(APPEND WebCoreTestSupport_INCLUDE_DIRECTORIES
            ${FONTCONFIG_INCLUDE_DIR}
        )
    endif ()
elseif (WIN32)
    # Eliminate C2139 errors
    if (MSVC)
        add_compile_options(/D_ENABLE_EXTENDED_ALIGNED_STORAGE)
    endif ()

    if (${JavaScriptCore_LIBRARY_TYPE} MATCHES STATIC)
        add_definitions(-DSTATICALLY_LINKED_WITH_WTF -DSTATICALLY_LINKED_WITH_JavaScriptCore)
    endif ()

    list(APPEND WebCore_SOURCES
        platform/win/SharedMemoryWin.cpp
        platform/win/SystemInfo.cpp
    )
else ()
    list(APPEND WebCore_SOURCES
        platform/unix/SharedMemoryUnix.cpp
    )
endif ()
