set(SERVER_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/asio/asio/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/nlohmann/
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/CLI11/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/spdlog/include
        "${PROJECT_BINARY_DIR}/packages/common/src"
        ../common/src/
)

set(SERVER_SOURCES
        src/main.cpp
        src/tcp_server.h
        ../common/src/global.h
        ../common/src/messages.h
)

include_directories(
        ${SERVER_TARGET_NAME}
        ${SERVER_INCLUDE_DIRS}
)

add_executable(${SERVER_TARGET_NAME}
        ${SERVER_SOURCES}
)

target_compile_definitions(${SERVER_TARGET_NAME} PRIVATE ASIO_STANDALONE)
