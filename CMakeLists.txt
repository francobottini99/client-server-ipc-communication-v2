cmake_minimum_required (VERSION 3.22)

project(SOCKECTS)

find_package(ZLIB REQUIRED)

file(MAKE_DIRECTORY bin)
file(MAKE_DIRECTORY data)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/client)
include_directories(${CMAKE_SOURCE_DIR}/include/server)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/client)
include_directories(${CMAKE_SOURCE_DIR}/src/server)

set(SOURCE_C src/client/client.c src/client/client_utils.c src/communication_api.c)
set(SOURCE_S src/server/server.c src/server/server_threads_handle.c src/server/server_utils.c src/communication_api.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -Wextra -Wconversion -std=gnu11 -g")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(client ${SOURCE_C})
add_executable(server ${SOURCE_S})

target_link_libraries(server PRIVATE ZLIB::ZLIB)
target_link_libraries(client PRIVATE ZLIB::ZLIB)