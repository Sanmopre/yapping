cmake_minimum_required(VERSION 3.11)
project(cli_chat
        VERSION 1.0.0
)

# Client properties
set(CLIENT_TARGET_NAME cli_chat)
set(CLIENT_DESCRIPTION "This is a cli chatting application that connects to a server.")

# Server properties
set(SERVER_TARGET_NAME server)
set(SERVER_DESCRIPTION "This is the server for the cli_chat.")

# Configuration file with constant cmake variables
configure_file(
    "${PROJECT_SOURCE_DIR}/packages/common/src/cmake_constants.h.in"
    "${PROJECT_BINARY_DIR}/packages/common/src/cmake_constants.h"
)

# Add sdl2 dependency
add_subdirectory(submodules/sdl)

add_subdirectory(packages/client)
add_subdirectory(packages/server)