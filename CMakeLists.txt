cmake_minimum_required(VERSION 3.11)
project(cli_chat)

set(CLIENT_TARGET_NAME cli_chat)
set(SERVER_TARGET_NAME server)

add_subdirectory(submodules/FTXUI)

add_subdirectory(packages/client)
add_subdirectory(packages/server)