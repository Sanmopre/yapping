cmake_minimum_required(VERSION 3.11)

project(cli_chat VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/utf-8 /permissive- /Zc:__cplusplus)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

option(BUILD_CLIENT "Build the client component" ON)
option(BUILD_SERVER "Build the server component" ON)

# Client properties
set(CLIENT_TARGET_NAME cli_chat CACHE STRING "Client target name")
set(CLIENT_DESCRIPTION "This is a cli chatting application that connects to a server.")

# Server properties
set(SERVER_TARGET_NAME server CACHE STRING "Server target name")
set(SERVER_DESCRIPTION "This is the server for the cli_chat.")

# Configuration file with constant cmake variables
configure_file(
    "${PROJECT_SOURCE_DIR}/packages/common/src/cmake_constants.h.in"
    "${PROJECT_BINARY_DIR}/packages/common/src/cmake_constants.h"
)

if(BUILD_CLIENT)
    # Add sdl2 dependency
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON  CACHE BOOL "" FORCE)
    add_subdirectory(submodules/sdl)

    add_subdirectory(packages/client)
endif ()

if(BUILD_SERVER)
    add_subdirectory(packages/server)
endif ()