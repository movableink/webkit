if (${JavaScriptCore_LIBRARY_TYPE} MATCHES STATIC)
    add_definitions(-DSTATICALLY_LINKED_WITH_WTF)
endif ()

target_compile_definitions(JavaScriptCore PRIVATE QT_NO_KEYWORDS)

list(APPEND JavaScriptCore_SOURCES
    API/JSStringRefQt.cpp
)

list(APPEND JavaScriptCore_PRIVATE_FRAMEWORK_HEADERS
    API/JSStringRefQt.h

    bytecode/UnlinkedFunctionExecutable.h
    runtime/JSDateMath.h
)

list(APPEND JavaScriptCore_SYSTEM_INCLUDE_DIRECTORIES
    ${Qt6Core_INCLUDE_DIRS}
)

list(APPEND JavaScriptCore_LIBRARIES
    ${Qt6Core_LIBRARIES}
)

if (QT_STATIC_BUILD)
    list(APPEND JavaScriptCore_LIBRARIES
        ${STATIC_LIB_DEPENDENCIES}
    )
endif ()

# From PlatformWin.cmake
if (WIN32)
    list(REMOVE_ITEM JavaScriptCore_SOURCES
        inspector/JSGlobalObjectInspectorController.cpp
    )
endif ()


if (APPLE)
    list(APPEND JavaScriptCore_PUBLIC_FRAMEWORK_HEADERS
        runtime/JSExportMacros.h
    )
endif ()
