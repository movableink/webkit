remove_definitions(-DQT_ASCII_CAST_WARNINGS)

add_subdirectory(QtTestBrowser)

if (ENABLE_TEST_SUPPORT)
    add_subdirectory(DumpRenderTree)
    add_subdirectory(ImageDiff)
endif ()

add_subdirectory(MiniBrowser/qt)

# FIXME: Remove when WK2 Tools patches are merged
set(ENABLE_WEBKIT2 0)
