set(QtTestBrowser_INCLUDE_DIRECTORIES
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/include"
    "${CMAKE_BINARY_DIR}/include/QtWebKit"
    "${CMAKE_BINARY_DIR}/include/QtWebKitWidgets"
    "${WTF_FRAMEWORK_HEADERS_DIR}"
    "${WebKitWidgets_FRAMEWORK_HEADERS_DIR}"
)

set(QtTestBrowser_SOURCES
    cookiejar.cpp
    fpstimer.cpp
    launcherwindow.cpp
    locationedit.cpp
    mainwindow.cpp
    qttestbrowser.cpp
    urlloader.cpp
    utils.cpp
    webinspector.h
    webpage.cpp
    webview.cpp
)

set(QtTestBrowser_FRAMEWORKS
    JavaScriptCore
    PAL
    WTF
    WebCore
    WebCoreTestSupport
    WebKitLegacy
)

if (NOT USE_SYSTEM_MALLOC)
    list(APPEND QtTestBrowser_FRAMEWORKS bmalloc)
endif ()

set(QtTestBrowser_SYSTEM_INCLUDE_DIRECTORIES
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt6StateMachine_INCLUDE_DIRS}
)

set(QtTestBrowser_LIBRARIES
    ${Qt6Gui_LIBRARIES}
    ${Qt6Network_LIBRARIES}
    ${Qt6Widgets_LIBRARIES}
    ${Qt6PrintSupport_LIBRARIES}
    ${Qt6OpenGLWidgets_LIBRARIES}
    ${Qt6StateMachine_LIBRARIES}
    WebKitWidgets
    ${STATIC_LIB_DEPENDENCIES}
)

qt_add_resources(QtTestBrowser_SOURCES
    QtTestBrowser.qrc
)

if (Qt6OpenGL_FOUND)
    list(APPEND QtTestBrowser_SYSTEM_INCLUDE_DIRECTORIES
        ${Qt6OpenGL_INCLUDE_DIRS}
    )
    list(APPEND QtTestBrowser_LIBRARIES
        ${Qt6OpenGL_LIBRARIES}
    )
endif ()

if (ENABLE_TEST_SUPPORT)
    add_definitions(-DHAVE_QTTESTSUPPORT)
endif ()

if (NOT TARGET Qt6::StateMachine)
    add_definitions(-DQT_NO_ANIMATION)
endif ()

include_directories(${QtTestBrowser_INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${QtTestBrowser_SYSTEM_INCLUDE_DIRECTORIES})
add_executable(QtTestBrowser ${QtTestBrowser_SOURCES})
target_link_libraries(QtTestBrowser ${QtTestBrowser_LIBRARIES})
set_target_properties(QtTestBrowser PROPERTIES FOLDER "Tools" AUTOMOC ON)

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    set_target_properties(QtTestBrowser PROPERTIES WIN32_EXECUTABLE ON)
endif ()
