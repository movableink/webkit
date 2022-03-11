cmake_minimum_required(VERSION 3.12)

include(WebKitCommon)
include(target/TargetWTF)
include(target/TargetJavaScriptCore)
include(target/PAL)
include(target/WebCore)

set_property(DIRECTORY . PROPERTY FOLDER "WebKitLegacy")

set(WebKitLegacy_SOURCES
    Storage/InProcessIDBServer.cpp
    Storage/StorageAreaImpl.cpp
    Storage/StorageAreaSync.cpp
    Storage/StorageNamespaceImpl.cpp
    Storage/StorageSyncManager.cpp
    Storage/StorageThread.cpp
    Storage/StorageTracker.cpp
    Storage/WebDatabaseProvider.cpp
    Storage/WebStorageNamespaceProvider.cpp

    WebCoreSupport/NetworkStorageSessionMap.cpp
    WebCoreSupport/WebBroadcastChannelRegistry.cpp
    WebCoreSupport/WebResourceLoadScheduler.cpp
    WebCoreSupport/WebViewGroup.cpp
)

set(WebKitLegacy_PRIVATE_INCLUDE_DIRECTORIES
    "${CMAKE_BINARY_DIR}"
    "${WEBKITLEGACY_DIR}/Storage"
    "${WEBKITLEGACY_DIR}/WebCoreSupport"
    "${WebKitLegacy_DERIVED_SOURCES_DIR}"
)

set(WebKitLegacy_PRIVATE_LIBRARIES
    WebKit::JavaScriptCore
    WebKit::PAL
    WebKit::WebCore
)

WEBKIT_FRAMEWORK_DECLARE(WebKitLegacy)
WEBKIT_INCLUDE_CONFIG_FILES_IF_EXISTS()

if (MSVC)
    if (NOT ${PORT} STREQUAL "Qt")
        WEBKIT_ADD_PRECOMPILED_HEADER("WebKitPrefix.h" "win/WebKitPrefix.cpp" WebKitLegacy_SOURCES)
        list(APPEND WebKitLegacy_SOURCES win/WebKit.resources/WebKit.rc)
        add_definitions(/DWEBKIT_EXPORTS /DFRAMEWORK_NAME=WebKitLegacy)
    else ()
        WEBKIT_ADD_PRECOMPILED_HEADER("WebKitPrefix.h" "qt/WebKitPrefix.cpp" WebKitLegacy_SOURCES)
    endif ()
endif ()

set(WebKit_PRIVATE_HEADERS_LOCATION Headers/${PROJECT_VERSION}/QtWebKit/private)

WEBKIT_FRAMEWORK(WebKitLegacy)

if (${PORT} STREQUAL "Qt")
    set_target_properties(WebKitLegacy PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
    install(TARGETS WebKitLegacy EXPORT WebKitTargets
            DESTINATION "${LIB_INSTALL_DIR}"
            RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
    )
    if (MSVC AND NOT QT_STATIC_BUILD)
        install(FILES $<TARGET_PDB_FILE:WebKitLegacy> DESTINATION "${BIN_INSTALL_DIR}" OPTIONAL)
    endif ()

    if (SEPARATE_DEBUG_INFO)
        QTWEBKIT_SEPARATE_DEBUG_INFO(WebKitLegacy WebKitLegacy_DEBUG_INFO)
        install(FILES ${WebKitLegacy_DEBUG_INFO} DESTINATION "${LIB_INSTALL_DIR}" OPTIONAL)
    endif ()

    if (NOT MSVC)
        ADD_PREFIX_HEADER(WebKitLegacy "${WEBKITLEGACY_DIR}/qt/WebKitPrefix.h")
    endif ()

    if (MACOS_BUILD_FRAMEWORKS)
        set_target_properties(WebKitLegacy PROPERTIES
            FRAMEWORK_VERSION ${PROJECT_VERSION_MAJOR}
            SOVERSION ${MACOS_COMPATIBILITY_VERSION}
            MACOSX_FRAMEWORK_IDENTIFIER org.qt-project.QtWebKit
        )
    endif ()

    if (USE_LINKER_VERSION_SCRIPT)
        set(VERSION_SCRIPT "${CMAKE_BINARY_DIR}/QtWebKit.version")
        add_custom_command(TARGET WebKitLegacy PRE_LINK
            COMMAND ${PERL_EXECUTABLE} ${TOOLS_DIR}/qt/generate-version-script.pl ${Qt5_VERSION} > ${VERSION_SCRIPT}
            VERBATIM
        )
    set_target_properties(WebKitLegacy PROPERTIES LINK_FLAGS -Wl,--version-script,${VERSION_SCRIPT})
    endif ()

    if (ENABLE_WEBKIT)
        add_dependencies(WebKitLegacy WebKit)
    endif ()
endif ()

if (TARGET WebKitLegacyGUID)
    add_dependencies(WebKitLegacy WebKitLegacyGUID)
endif ()

if (APPLE AND NOT ${PORT} STREQUAL "Qt")
    set_target_properties(WebKitLegacy PROPERTIES LINK_FLAGS "-umbrella WebKit")
endif ()

if (MSVC AND NOT ${PORT} STREQUAL "Qt")
    add_custom_command(
        TARGET WebKitLegacy
        POST_BUILD
        COMMAND ${PERL_EXECUTABLE} ${WEBKIT_LIBRARIES_DIR}/tools/scripts/version-stamp.pl ${WebKitLegacy_DERIVED_SOURCES_DIR} $<TARGET_FILE:WebKitLegacy>
        VERBATIM)

    if (USE_CA AND NOT ${PORT} STREQUAL "Qt")
        add_subdirectory(win/WebKitQuartzCoreAdditions)
    endif ()
endif ()
