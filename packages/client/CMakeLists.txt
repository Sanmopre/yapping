set(CLIENT_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/FTXUI/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/asio/asio/include
)

set(CLIENT_SOURCES
        src/main.cpp
        src/tcp_client.h
)

include_directories(
        ${CLIENT_TARGET_NAME}
        ${CLIENT_INCLUDE_DIRS}
)

add_executable(${CLIENT_TARGET_NAME}
        ${CLIENT_SOURCES}
)

target_compile_definitions(${CLIENT_TARGET_NAME} PRIVATE ASIO_STANDALONE)
target_link_libraries(${CLIENT_TARGET_NAME}
        PRIVATE
        ftxui::dom
        ftxui::component
        ftxui::screen
)