list(APPEND DumpRenderTree_INCLUDE_DIRECTORIES
    "${QtWebKit_FRAMEWORK_HEADERS_DIR}"
    "${QtWebKitWidgets_FRAMEWORK_HEADERS_DIR}"
    "${WebKitWidgets_FRAMEWORK_HEADERS_DIR}"
    "${WEBKITLEGACY_DIR}/qt/WidgetSupport"
    "${DumpRenderTree_DIR}/qt"
)

list(REMOVE_ITEM DumpRenderTree_SOURCES
    JavaScriptThreading.cpp
    PixelDumpSupport.cpp
    WorkQueueItem.cpp
)

list(APPEND DumpRenderTree_SOURCES
    qt/DumpRenderTreeMain.cpp
    qt/DumpRenderTreeQt.cpp
    qt/EventSenderQt.cpp
    qt/GCControllerQt.cpp
    qt/TestRunnerQt.cpp
    qt/TextInputControllerQt.cpp
    qt/WorkQueueItemQt.cpp
    qt/UIScriptControllerQt.cpp
)

Qt6_add_resources(DumpRenderTree_SOURCES
    qt/DumpRenderTree.qrc
)

list(APPEND DumpRenderTree_SYSTEM_INCLUDE_DIRECTORIES
    ${ICU_INCLUDE_DIRS}
    ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
)

list(APPEND DumpRenderTree_LIBRARIES
    ${Qt6PrintSupport_LIBRARIES}
    ${Qt6Test_LIBRARIES}
    ${Qt6Widgets_LIBRARIES}
    WebKitWidgets
)

list(APPEND DumpRenderTree_FRAMEWORKS
    WebKitLegacy
    WebKitWidgets
)

if (USE_QT_MULTIMEDIA)
    list(APPEND DumpRenderTree_SYSTEM_INCLUDE_DIRECTORIES
        ${Qt6Multimedia_INCLUDE_DIRS}
    )
    list(APPEND DumpRenderTree_LIBRARIES
        ${Qt6Multimedia_LIBRARIES}
    )
endif ()

if (WIN32)
    add_definitions(-DWEBCORE_EXPORT=)
    add_definitions(-DSTATICALLY_LINKED_WITH_WTF -DSTATICALLY_LINKED_WITH_JavaScriptCore)
endif ()
