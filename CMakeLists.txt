project(gestion)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set (CMAKE_CXX_FLAGS "-g -std=c++14 -Wall -pthread -lwiringPi")

set(SRC_FILES src/main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(MidiSynth ${SRC_FILES})

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})
target_link_libraries(MidiSynth ${OPENAL_LIBRARY})
