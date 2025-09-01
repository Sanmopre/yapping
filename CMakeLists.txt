cmake_minimum_required(VERSION 3.11)
project(cli_chat)

set(TARGET_NAME cli_chat)

add_subdirectory(FTXUI)

set(INCLUDE_DIRS
    FTXUI/include
)

set(CLI_CHAT_SOURCES
        src/main.cpp
)

include_directories(
    ${TARGET_NAME}
    ${INCLUDE_DIRS}
)

add_executable(${TARGET_NAME}
        ${CLI_CHAT_SOURCES}
)

target_link_libraries(${TARGET_NAME}
        PRIVATE
        ftxui::dom
        ftxui::component
        ftxui::screen
)