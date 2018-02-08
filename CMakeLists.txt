project(gestion)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set (CMAKE_CXX_FLAGS "-g -std=c++14 -Wall -pthread -lboost_system")

set(SRC_FILES src/main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(server src/server.cpp)
set_target_properties(server PROPERTIES COMPILE_FLAGS "-lwiringPi")

add_executable(client src/client.cpp)

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})
target_link_libraries(client ${OPENAL_LIBRARY})
