get_target_property(RCC_EXECUTABLE ${Qt5Core_RCC_EXECUTABLE} IMPORTED_LOCATION)

add_custom_command(
    OUTPUT ${WebInspectorUI_DERIVED_SOURCES_DIR}/qrc_WebInspector.cpp
    DEPENDS ${InspectorFilesDependencies}
            ${WebInspectorUI_DERIVED_SOURCES_DIR}/UserInterface/Protocol/InspectorBackendCommands.js
            ${TOOLS_DIR}/qt/generate-inspector-qrc.pl
    COMMAND ${PERL_EXECUTABLE} ${TOOLS_DIR}/qt/generate-inspector-qrc.pl
            --baseDir ${CMAKE_SOURCE_DIR}/Source/WebInspectorUI
            --outDir ${WebInspectorUI_DERIVED_SOURCES_DIR}
            --prefix /webkit/inspector
            --rccExecutable ${RCC_EXECUTABLE}
            --resourceName WebInspector
            --add UserInterface/Protocol/InspectorBackendCommands.js
            ${InspectorFiles}
    VERBATIM
)
