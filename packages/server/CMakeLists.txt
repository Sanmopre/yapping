set(SQLITE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3/sqlite3.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3/sqlite3.h
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3/sqlite3ext.h
)

set(SERVER_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/asio/asio/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/nlohmann/
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/CLI11/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/spdlog/include
        ${PROJECT_BINARY_DIR}/packages/common/src
        ${CMAKE_CURRENT_SOURCE_DIR}/packages/server/sqlite3
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/
)

set(SERVER_SOURCES
        ${SQLITE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/global.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/messages.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/db_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/db_manager.h
)

include_directories(
        ${SERVER_TARGET_NAME}
        ${SERVER_INCLUDE_DIRS}
)

add_executable(${SERVER_TARGET_NAME}
        ${SERVER_SOURCES}
)

target_compile_definitions(${SERVER_TARGET_NAME} PRIVATE ASIO_STANDALONE)

if (WIN32)
    target_link_libraries(${SERVER_TARGET_NAME} PRIVATE ws2_32 mswsock)
endif()