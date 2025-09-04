cmake_minimum_required(VERSION 3.11)
project(cli_chat)

set(CLIENT_TARGET_NAME cli_chat)
set(SERVER_TARGET_NAME server)

add_subdirectory(FTXUI)

find_package(Threads REQUIRED)

set(CLIENT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/include
    ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include
)

set(SERVER_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/include
        ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include
)

set(CLIENT_SOURCES
        packages/client/src/main.cpp
        packages/client/src/tcp_client.h
)

set(SERVER_SOURCES
        packages/server/src/main.cpp
)

include_directories(
    ${CLIENT_TARGET_NAME}
    ${CLIENT_INCLUDE_DIRS}
)
include_directories(
        ${SERVER_TARGET_NAME}
        ${SERVER_INCLUDE_DIRS}
)

add_executable(${CLIENT_TARGET_NAME}
        ${CLIENT_SOURCES}
)

add_executable(${SERVER_TARGET_NAME}
        ${SERVER_SOURCES}
)

target_compile_definitions(${CLIENT_TARGET_NAME} PRIVATE ASIO_STANDALONE)
target_compile_definitions(${SERVER_TARGET_NAME} PRIVATE ASIO_STANDALONE)

target_link_libraries(${CLIENT_TARGET_NAME}
        PRIVATE
        ftxui::dom
        ftxui::component
        ftxui::screen
        Threads::Threads
)

target_link_libraries(${SERVER_TARGET_NAME}
        PRIVATE
        Threads::Threads
)