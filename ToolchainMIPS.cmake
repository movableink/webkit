INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_SYSTEM_PROCESSOR mips)

#
# stbgcc toolchain
#
SET(STBGCC_PATH /opt/toolchains/stbgcc-4.8-1.2)

#
# specify the cross compiler
#
SET(CMAKE_C_COMPILER   ${STBGCC_PATH}/bin-ccache/mipsel-linux-gcc)
SET(CMAKE_CXX_COMPILER ${STBGCC_PATH}/bin-ccache/mipsel-linux-g++)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH ${STBGCC_PATH}/target)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

