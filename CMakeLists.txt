cmake_minimum_required(VERSION 3.11)
project(cli_chat VERSION 1.0.0)

set(CLIENT_TARGET_NAME cli_chat)
set(SERVER_TARGET_NAME server)

# Configuration file with constant cmake variables
configure_file(
    "${PROJECT_SOURCE_DIR}/packages/common/src/cmake_constants.h.in"
    "${PROJECT_BINARY_DIR}/packages/common/src/cmake_constants.h"
)

# Add FTXUI dependency
add_subdirectory(submodules/FTXUI)

add_subdirectory(packages/client)
add_subdirectory(packages/server)