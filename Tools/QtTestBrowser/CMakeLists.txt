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
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
    WebKitWidgets
    ${STATIC_LIB_DEPENDENCIES}
)

if (TARGET Qt6::PrintSupport)
    list(APPEND QtTestBrowser_LIBRARIES Qt6::PrintSupport)
endif ()

if (Qt6Core_VERSION VERSION_GREATER_EQUAL 6.10)
    list(APPEND QtTestBrowser_PRIVATE_LIBRARIES Qt6::CorePrivate)
endif ()
if (Qt6Gui_VERSION VERSION_GREATER_EQUAL 6.10)
    list(APPEND QtTestBrowser_PRIVATE_LIBRARIES Qt6::GuiPrivate)
endif ()

qt_add_resources(QtTestBrowser_SOURCES
    QtTestBrowser.qrc
)

if (TARGET Qt6::OpenGL)
    list(APPEND QtTestBrowser_LIBRARIES Qt6::OpenGL)
endif ()

if (TARGET Qt6::OpenGLWidgets)
    list(APPEND QtTestBrowser_LIBRARIES Qt6::OpenGLWidgets)
endif ()

if (ENABLE_TEST_SUPPORT)
    add_definitions(-DHAVE_QTTESTSUPPORT)
endif ()

if (TARGET Qt6::StateMachine)
    list(APPEND QtTestBrowser_LIBRARIES Qt6::StateMachine)
else ()
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
