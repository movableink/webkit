function(MSVC_ADD_COMPILE_OPTIONS)
    foreach (_option ${ARGV})
        add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:${_option}>)
    endforeach ()
endfunction()

if (NOT COMPILER_IS_CLANG_CL)
    # List of disabled warnings
    # When adding to the list add a short description and link to the warning's text if available
    #
    # https://bugs.webkit.org/show_bug.cgi?id=221508 is for tracking removal of warnings
    MSVC_ADD_COMPILE_OPTIONS(
        /wd4018 # 'token' : signed/unsigned mismatch
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4018

        /wd4060 # switch statement contains no 'case' or 'default' labels

        /wd4068 # unknown pragma
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4068

        /wd4100 # 'identifier' : unreferenced formal parameter
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4100

        /wd4127 # conditional expression is constant
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4127

        /wd4146 # unary minus operator applied to unsigned type, result still unsigned
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4146

        /wd4189 # 'identifier' : local variable is initialized but not referenced
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4189

        /wd4201 # nonstandard extension used : nameless struct/union
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4201

        /wd4244 # 'argument' : conversion from 'type1' to 'type2', possible loss of data
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4244

        /wd4245 # 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4245

        /wd4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4251

        /wd4275 # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4275

        /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4267

        /wd4305 # 'context' : truncation from 'type1' to 'type2'
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4305

        /wd4309 # 'conversion' : truncation of constant value
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4309

        /wd4312 # 'operation' : conversion from 'type1' to 'type2' of greater size
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4312

        /wd4324 # 'struct_name' : structure was padded due to __declspec(align())
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4324

        /wd4389 # 'operator' : signed/unsigned mismatch
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4389

        /wd4456 # declaration of 'identifier' hides previous local declaration
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4456

        /wd4457 # declaration of 'identifier' hides function parameter
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4457

        /wd4458 # declaration of 'identifier' hides class member
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4458

        /wd4459 # declaration of 'identifier' hides global declaration
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4459

        /wd4505 # 'function' : unreferenced local function has been removed
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4505


        /wd4611 # interaction between 'function' and C++ object destruction is non-portable
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4611

        /wd4646 # function declared with __declspec(noreturn) has non-void return type
                # https://docs.microsoft.com/mt-mt/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4646

        /wd4701 # Potentially uninitialized local variable 'name' used
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4701

        /wd4702 # unreachable code
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4702

        /wd4706 # assignment within conditional expression
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4706
                # NOTE: Can't fix without changes to style guide

        /wd4715 # 'function' : not all control paths return a value
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4715

        /wd4722 # 'function' : destructor never returns, potential memory leak
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4722

        /wd4723 # The second operand in a divide operation evaluated to zero at compile time, giving undefined results.
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4723

        /wd4805 # 'operation' : unsafe mix of type 'type' and type 'type' in operation
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4805

        /wd4838 # conversion from 'type_1' to 'type_2' requires a narrowing conversion
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4838

        /wd4840 # non-portable use of class 'type' as an argument to a variadic function
                # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4840

        /wd4996 # Your code uses a function, class member, variable, or typedef that's marked deprecated

        /wd5205 # delete of an abstract class 'type-name' that has a non-virtual destructor results in undefined behavior

        /wd5054 # operator 'operator-name': deprecated between enumerations of different types

        /wd5055 # operator 'operator-name': deprecated between enumerations and floating-point types
    )
endif ()

# Create pdb files for debugging purposes, also for Release builds
MSVC_ADD_COMPILE_OPTIONS(/Zi /GS)

# Disable ICF (identical code folding) optimization,
# as it makes it unsafe to pointer-compare functions with identical definitions.
string(APPEND CMAKE_SHARED_LINKER_FLAGS " /DEBUG /OPT:NOICF /OPT:REF")
string(APPEND CMAKE_EXE_LINKER_FLAGS " /DEBUG /OPT:NOICF /OPT:REF")

# We do not use exceptions
add_definitions(-D_HAS_EXCEPTIONS=0)
MSVC_ADD_COMPILE_OPTIONS(/EHa- /EHc- /EHs- /fp:except-)

# We have some very large object files that have to be linked
MSVC_ADD_COMPILE_OPTIONS(/analyze- /bigobj)

# Use CRT security features
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
if (NOT COMPILER_IS_CLANG_CL)
    add_definitions(-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1)
endif ()

add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)

# Specify the source code encoding
MSVC_ADD_COMPILE_OPTIONS(/utf-8 /validate-charset)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " /OPT:NOREF")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " /OPT:NOREF")

    # To debug linking time issues, uncomment the following three lines:
    #add_compile_options(/Bv)
    #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /VERBOSE /VERBOSE:INCR /TIME")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /VERBOSE /VERBOSE:INCR /TIME")
elseif (${CMAKE_BUILD_TYPE} MATCHES "Release")
    MSVC_ADD_COMPILE_OPTIONS(/Oy-)
endif ()

if (NOT ${CMAKE_GENERATOR} MATCHES "Ninja")
    link_directories("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}")
    add_definitions(/MP)
endif ()
if (NOT ${CMAKE_CXX_FLAGS} STREQUAL "")
    string(REGEX REPLACE "(/EH[a-z]+) " "\\1- " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable C++ exceptions
    string(REGEX REPLACE "/EHsc$" "/EHs- /EHc- " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable C++ exceptions
    string(REGEX REPLACE "/GR " "/GR- " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable RTTI
    # More warnings. /W4 should be specified before -Wno-* options for clang-cl.
    string(REGEX REPLACE "/W3" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
    string(REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    WEBKIT_PREPEND_GLOBAL_COMPILER_FLAGS(/W4)
endif ()

# Make sure incremental linking is turned off, as it creates unacceptably long link times.
string(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replace_CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS})
set(CMAKE_SHARED_LINKER_FLAGS "${replace_CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")
string(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replace_CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS "${replace_CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")

string(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replace_CMAKE_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${replace_CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
string(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replace_CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${replace_CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")

string(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replace_CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO})
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${replace_CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /INCREMENTAL:NO")
string(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replace_CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${replace_CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /INCREMENTAL:NO")

if (COMPILER_IS_CLANG_CL)
    string(REGEX MATCH "^[0-9]+" CLANG_CL_MAJOR_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    cmake_path(REMOVE_FILENAME CMAKE_CXX_COMPILER OUTPUT_VARIABLE CLANG_CL_DIR)
    cmake_path(APPEND CLANG_CL_DIR "../lib/clang" ${CLANG_CL_MAJOR_VERSION} "lib/windows")
    find_library(CLANG_BUILTINS_LIBRARY clang_rt.builtins-x86_64 PATHS ${CLANG_CL_DIR} REQUIRED NO_DEFAULT_PATH)
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " \"${CLANG_BUILTINS_LIBRARY}\"")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " \"${CLANG_BUILTINS_LIBRARY}\"")
endif ()

# Enable the new lambda processor for better C++ conformance
if (NOT COMPILER_IS_CLANG_CL AND MSVC_VERSION GREATER_EQUAL 1928)
    MSVC_ADD_COMPILE_OPTIONS(/Zc:lambda)
endif ()
