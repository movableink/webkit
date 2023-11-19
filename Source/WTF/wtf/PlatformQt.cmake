target_compile_definitions(WTF PRIVATE QT_NO_KEYWORDS)

list(APPEND WTF_SOURCES
    qt/FileSystemQt.cpp
    qt/LanguageQt.cpp
    qt/MainThreadQt.cpp
    qt/RunLoopQt.cpp
    qt/URLQt.cpp

    text/qt/AtomStringQt.cpp
    text/qt/StringQt.cpp
    text/qt/TextBreakIteratorInternalICUQt.cpp
)
QTWEBKIT_GENERATE_MOC_FILES_CPP(WTF qt/MainThreadQt.cpp qt/RunLoopQt.cpp)

if (WIN32)
    list(APPEND WTF_PUBLIC_HEADERS
        text/win/WCharStringEXtras.h
    )
    list(APPEND WTF_SOURCES
        text/win/StringWin.cpp

        win/DbgHelperWin.cpp
        win/FileSystemWin.cpp
        win/OSAllocatorWin.cpp
        win/PathWalker.cpp
        win/ThreadSpecificWin.cpp
        win/ThreadingWin.cpp
    )
    list(APPEND WTF_LIBRARIES
        dbghelp
        shlwapi
    )
else ()
    list(APPEND WTF_SOURCES
        posix/FileSystemPOSIX.cpp
        posix/OSAllocatorPOSIX.cpp
        posix/ThreadingPOSIX.cpp
        posix/CPUTimePOSIX.cpp
    )
endif ()

if (USE_MACH_PORTS)
    list(APPEND WTF_SOURCES
        cocoa/MachSendRight.cpp
        cocoa/WorkQueueCocoa.cpp
        darwin/OSLogPrintStream.mm
    )
    list(APPEND WTF_PUBLIC_HEADERS
        spi/cocoa/MachVMSPI.h
        spi/darwin/XPCSPI.h
        spi/darwin/AbortWithReasonSPI.h
        darwin/OSLogPrintStream.h
    )
endif()

list(APPEND WTF_SYSTEM_INCLUDE_DIRECTORIES
    ${Qt6Core_INCLUDE_DIRS}
)

list(APPEND WTF_LIBRARIES
    ${Qt6Core_LIBRARIES}
    Threads::Threads
)

if (SHARED_CORE)
    set(WTF_LIBRARY_TYPE SHARED)
else ()
    set(WTF_LIBRARY_TYPE STATIC)
endif ()

if (QT_STATIC_BUILD)
    list(APPEND WTF_LIBRARIES
        ${STATIC_LIB_DEPENDENCIES}
    )
endif ()

if (USE_UNIX_DOMAIN_SOCKETS)
    list(APPEND WTF_SOURCES
        qt/WorkQueueQt.cpp

        unix/UniStdExtrasUnix.cpp
    )
    QTWEBKIT_GENERATE_MOC_FILES_CPP(WTF qt/WorkQueueQt.cpp)
endif ()

if (USE_GLIB)
    list(APPEND WTF_SOURCES
        glib/GRefPtr.cpp
        glib/Sandbox.cpp
    )
    list(APPEND WTF_SYSTEM_INCLUDE_DIRECTORIES
        ${GLIB_INCLUDE_DIRS}
    )
    list(APPEND WTF_LIBRARIES
        ${GLIB_GIO_LIBRARIES}
        ${GLIB_GOBJECT_LIBRARIES}
        ${GLIB_LIBRARIES}
    )
    list(APPEND WTF_PUBLIC_HEADERS
        glib/ChassisType.h
        glib/GRefPtr.h
        glib/GTypedefs.h
        glib/GUniquePtr.h
        glib/RunLoopSourcePriority.h
        glib/Sandbox.h
        glib/WTFGType.h
    )
endif ()

if (WIN32)
    list(APPEND WTF_SOURCES
        win/CPUTimeWin.cpp
        win/WorkQueueWin.cpp
    )
    list(APPEND WTF_LIBRARIES
        winmm
    )
endif ()

if (APPLE)
    list(APPEND WTF_SOURCES
        cocoa/WorkQueueCocoa.cpp

        text/cf/AtomStringImplCF.cpp
        text/cf/StringCF.cpp
        text/cf/StringImplCF.cpp
        text/cf/StringViewCF.cpp
    )
    list(APPEND WTF_LIBRARIES
        ${COREFOUNDATION_LIBRARY}
    )
    list(APPEND WTF_PUBLIC_HEADERS
        cf/TypeCastsCF.h
    )
endif ()

if (UNIX AND NOT APPLE)
    list(APPEND WTF_SOURCES
        unix/MemoryPressureHandlerUnix.cpp
        unix/LoggingUnix.cpp
    )

    check_function_exists(clock_gettime CLOCK_GETTIME_EXISTS)
    if (NOT CLOCK_GETTIME_EXISTS)
        set(CMAKE_REQUIRED_LIBRARIES rt)
        check_function_exists(clock_gettime CLOCK_GETTIME_REQUIRES_LIBRT)
        if (CLOCK_GETTIME_REQUIRES_LIBRT)
            list(APPEND WTF_LIBRARIES rt)
        endif ()
    endif ()
endif ()

# From PlatformJSCOnly.cmake
if (WIN32)
    list(APPEND WTF_SOURCES
        win/MemoryFootprintWin.cpp
        win/MemoryPressureHandlerWin.cpp
    )
    list(APPEND WTF_PUBLIC_HEADERS
        win/Win32Handle.h
    )
elseif (APPLE)
    file(COPY mac/MachExceptions.defs DESTINATION ${WTF_DERIVED_SOURCES_DIR})
    add_custom_command(
        OUTPUT
            ${WTF_DERIVED_SOURCES_DIR}/MachExceptionsServer.h
            ${WTF_DERIVED_SOURCES_DIR}/mach_exc.h
            ${WTF_DERIVED_SOURCES_DIR}/mach_excServer.c
            ${WTF_DERIVED_SOURCES_DIR}/mach_excUser.c
        MAIN_DEPENDENCY mac/MachExceptions.defs
        WORKING_DIRECTORY ${WTF_DERIVED_SOURCES_DIR}
        COMMAND mig -sheader MachExceptionsServer.h MachExceptions.defs
        VERBATIM)
    list(APPEND WTF_SOURCES
        cocoa/MemoryFootprintCocoa.cpp
        cocoa/MemoryPressureHandlerCocoa.mm
        ${WTF_DERIVED_SOURCES_DIR}/mach_excServer.c
        ${WTF_DERIVED_SOURCES_DIR}/mach_excUser.c
    )
    list(APPEND WTF_PUBLIC_HEADERS
        spi/darwin/ProcessMemoryFootprint.h
    )
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND WTF_SOURCES
        linux/CurrentProcessMemoryStatus.cpp
        linux/MemoryFootprintLinux.cpp
    )
    list(APPEND WTF_PUBLIC_HEADERS
        linux/CurrentProcessMemoryStatus.h
        linux/ProcessMemoryFootprint.h
    )
else ()
    list(APPEND WTF_SOURCES
        generic/MemoryFootprintGeneric.cpp
        generic/MemoryPressureHandlerGeneric.cpp
    )
endif ()
