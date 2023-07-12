add_custom_target(WebKitTestRunner-forwarding-headers
    COMMAND ${PERL_EXECUTABLE} ${WEBKIT_DIR}/Scripts/generate-forwarding-headers.pl --include-path ${WEBKIT_TESTRUNNER_DIR} --include-path ${WEBKIT_TESTRUNNER_SHARED_DIR} --output ${FORWARDING_HEADERS_DIR} --platform qt
)

set(ForwardingHeadersForWebKitTestRunner_NAME WebKitTestRunner-forwarding-headers)

list(APPEND WebKitTestRunner_SOURCES
    qt/main.cpp
    qt/PlatformWebViewQt.cpp
    qt/TestControllerQt.cpp
    qt/EventSenderProxyQt.cpp
    qt/TestInvocationQt.cpp
)

QTWEBKIT_GENERATE_MOC_FILES_CPP(WebKitTestRunner
    qt/main.cpp
    qt/PlatformWebViewQt.cpp
    qt/TestControllerQt.cpp
)

list(APPEND WebKitTestRunner_INCLUDE_DIRECTORIES
    ${FORWARDING_HEADERS_DIR}
    "${WEBKITLEGACY_DIR}/qt/WebCoreSupport"
)

list(APPEND WebKitTestRunner_SYSTEM_INCLUDE_DIRECTORIES
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt6Quick_INCLUDE_DIRS}
    ${Qt6Quick_PRIVATE_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
)

list(APPEND WebKitTestRunner_LIBRARIES
    WebCore
    ${Qt6Core_LIBRARIES}
    ${Qt6Widgets_LIBRARIES}
    ${Qt6Quick_LIBRARIES}
    ${Qt6Test_LIBRARIES}
)

set(WebKitTestRunnerInjectedBundle_LIBRARIES
    WebCoreTestSupport
    WebKit
    ${Qt6Quick_LIBRARIES}
)

list(APPEND WebKitTestRunnerInjectedBundle_SOURCES
    ${WEBKIT_TESTRUNNER_INJECTEDBUNDLE_DIR}/qt/TestRunnerQt.cpp
)

add_definitions(
    -DTOP_LEVEL_DIR="${CMAKE_SOURCE_DIR}"
)
