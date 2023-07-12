set(ImageDiff_SOURCES
    ${IMAGE_DIFF_DIR}/qt/ImageDiff.cpp
)

list(APPEND ImageDiff_SYSTEM_INCLUDE_DIRECTORIES
    ${Qt6Gui_INCLUDE_DIRS}
)

set(ImageDiff_LIBRARIES
    ${Qt6Gui_LIBRARIES}
)
