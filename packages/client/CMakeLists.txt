set(IMGUI_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imconfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imgui.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imgui_internal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imstb_rectpack.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imstb_textedit.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/imstb_truetype.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/backends/imgui_impl_sdl2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/backends/imgui_impl_sdl2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/backends/imgui_impl_sdlrenderer2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

set(CLIENT_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/asio/asio/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/nlohmann/single_include/nlohmann
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/CLI11/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/spdlog/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/zlib/
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/sdl/include
        ${PROJECT_BINARY_DIR}/packages/common/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/hex_dumps/images
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/hex_dumps/fonts
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/
)

set(CLIENT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/global.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/messages.h
        ${IMGUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/chat_imgui_components.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_client.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/compression_utils.h
        src/data_manager.cpp
        src/data_manager.h
)

include_directories(
        ${CLIENT_TARGET_NAME}
        ${CLIENT_INCLUDE_DIRS}
)

add_executable(${CLIENT_TARGET_NAME} ${CLIENT_SOURCES})

target_compile_definitions(${CLIENT_TARGET_NAME} PRIVATE ASIO_STANDALONE SDL_MAIN_HANDLED)

target_link_libraries(${CLIENT_TARGET_NAME}
        PRIVATE
        SDL2::SDL2-static
        zlibstatic
)

# Windows build
if (WIN32)
    target_link_libraries(${CLIENT_TARGET_NAME} PRIVATE ws2_32 mswsock)
endif()