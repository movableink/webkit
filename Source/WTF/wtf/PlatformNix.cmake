list(APPEND WTF_SOURCES
    nix/MainThreadNix.cpp
    nix/RunLoopNix.cpp
    nix/WorkQueueNix.cpp
)

list(APPEND WTF_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
)
