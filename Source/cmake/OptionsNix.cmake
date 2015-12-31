find_package(ICU REQUIRED)
find_package(Threads REQUIRED)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_MICRO 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_MICRO})

WEBKIT_OPTION_BEGIN()
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_FTL_JIT PUBLIC ON)
WEBKIT_OPTION_END()

set(ENABLE_WEBKIT OFF)
set(ENABLE_WEBKIT2 OFF)
set(ENABLE_WEBCORE OFF)
set(ENABLE_API_TESTS OFF)
set(WTF_USE_UDIS86 1)

if (ENABLE_FTL_JIT)
    if (WTF_CPU_X86_64)
        find_package(LLVM 3.6)
        if (NOT LLVM_FOUND)
            message(FATAL_ERROR "LLVM 3.6 is required for ENABLE_FTL_JIT")
        endif ()
        SET_AND_EXPOSE_TO_BUILD(HAVE_LLVM TRUE)
    else ()
        message(FATAL_ERROR "FTL is only supported for X86_64")
    endif ()
endif ()


# From OptionsGTK.cmake
if (CMAKE_MAJOR_VERSION LESS 3)
    # Before CMake 3 it was necessary to use a build script instead of using cmake --build directly
    # to preserve colors and pretty-printing.

    build_command(COMMAND_LINE_TO_BUILD)
    # build_command unconditionally adds -i (ignore errors) for make, and there's
    # no reasonable way to turn that off, so we just replace it with -k, which has
    # the same effect, except that the return code will indicate that an error occurred.
    # See: http://www.cmake.org/cmake/help/v3.0/command/build_command.html
    string(REPLACE " -i" " -k" COMMAND_LINE_TO_BUILD ${COMMAND_LINE_TO_BUILD})
    file(WRITE
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build.sh
        "#!/bin/sh\n"
        "${COMMAND_LINE_TO_BUILD} $@"
    )
    file(COPY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build.sh
        DESTINATION ${CMAKE_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
    )
endif ()

# You can build JavaScriptCore as a static library if you specify it as STATIC
# set(JavaScriptCore_LIBRARY_TYPE STATIC)
