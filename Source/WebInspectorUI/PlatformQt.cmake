get_target_property(RCC_EXECUTABLE Qt6::rcc IMPORTED_LOCATION)

if (NOT DEFINED InspectorFiles)
    # QTFIXME: Must be kept in sync with CMakeLists.txt; probably a better way?
    set(InspectorFiles
        ${WEBINSPECTORUI_DIR}/UserInterface/*.html
        ${WEBINSPECTORUI_DIR}/UserInterface/Base/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Controllers/*.css
        ${WEBINSPECTORUI_DIR}/UserInterface/Controllers/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Debug/*.css
        ${WEBINSPECTORUI_DIR}/UserInterface/Debug/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/External/CodeMirror/*.css
        ${WEBINSPECTORUI_DIR}/UserInterface/External/CodeMirror/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/External/Esprima/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/External/three.js/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Models/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Protocol/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Proxies/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Test/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Views/*.css
        ${WEBINSPECTORUI_DIR}/UserInterface/Views/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Workers/Formatter/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Workers/HeapSnapshot/*.js
        ${WEBINSPECTORUI_DIR}/UserInterface/Images/*.png
        ${WEBINSPECTORUI_DIR}/UserInterface/Images/*.svg
        ${WEBINSPECTORUI_DIR}/Localizations/en.lproj/localizedStrings.js
    )
endif()

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
