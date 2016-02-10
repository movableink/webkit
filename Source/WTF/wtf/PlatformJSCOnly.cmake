list(APPEND WTF_SOURCES
    none/MainThreadNone.cpp
    none/RunLoopNone.cpp
    none/WorkQueueNone.cpp
)

list(APPEND WTF_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
)
