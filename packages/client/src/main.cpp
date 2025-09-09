#include "tcp_client.h"
#include "cmake_constants.h"

// imgui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_internal.h"

// std
#include <stdio.h>

// sdl
#include "SDL.h"

// cli11
#include "CLI/CLI.hpp"


static void MessageBubble(const char* text,
                          bool is_outgoing,
                          float avail_width,
                          ImU32 col_bg_incoming  = IM_COL32(235, 236, 240, 255), // light gray
                          ImU32 col_bg_outgoing  = IM_COL32(180, 232, 149, 255), // green
                          ImU32 col_text         = IM_COL32(0, 0, 0, 255))
{
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const float padding_x = 10.0f;
    const float padding_y = 8.0f;
    const float rounding  = 12.0f;
    const float space_y   = 6.0f;

    // Limit bubble width to ~65% of available space
    float max_width = ImClamp(avail_width * 0.65f, 80.0f, avail_width);

    // Measure text with wrapping
    ImVec2 text_size = ImGui::CalcTextSize(text, nullptr, true, max_width);

    // Bubble size = text + padding
    ImVec2 bubble_size(text_size.x + padding_x * 2.0f,
                       text_size.y + padding_y * 2.0f);

    // Cursor position in screen space
    ImVec2 cursor_screen = ImGui::GetCursorScreenPos();

    // Horizontal placement
    ImVec2 bubble_min;
    if (is_outgoing) {
        bubble_min.x = cursor_screen.x + avail_width - bubble_size.x;
    } else {
        bubble_min.x = cursor_screen.x;
    }
    bubble_min.y = cursor_screen.y;

    ImVec2 bubble_max(bubble_min.x + bubble_size.x,
                      bubble_min.y + bubble_size.y);

    // Draw bubble background
    ImU32 col_bg = is_outgoing ? col_bg_outgoing : col_bg_incoming;
    dl->AddRectFilled(bubble_min, bubble_max, col_bg, rounding);

    // Draw text
    ImVec2 text_pos(bubble_min.x + padding_x, bubble_min.y + padding_y);
    ImGui::SetCursorScreenPos(text_pos);

    float inner_wrap = text_pos.x + (bubble_size.x - padding_x * 2.0f);
    ImGui::PushTextWrapPos(inner_wrap);
    ImGui::PushStyleColor(ImGuiCol_Text, col_text);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();

    // Advance cursor for next bubble
    ImGui::SetCursorScreenPos(ImVec2(cursor_screen.x, bubble_max.y + space_y));
}


// Main code
int main(int argc, char** argv)
{
    CLI::App clientApplication(CLIENT_DESCRIPTION);
    clientApplication.set_version_flag("--version", PROJECT_VERSION);

    // Arguments for the client
    std::string username = "random";
    std::string serverIp;
    std::string loggingFolder = "./logs";
    u16 serverPort;

    clientApplication.add_option("-u,--username", username,
        "Username for the client to use when connecting");

    clientApplication.add_option("-i,--ip", serverIp,
        "IPv4 address of the server to connect to")
        ->required()
        ->check(CLI::ValidIPV4);

    clientApplication.add_option("-p,--port", serverPort,
        "Port of the server to connect to")
        ->required()
        ->check(CLI::Range(1, 65535));

    clientApplication.add_option("-l,--log-folder", loggingFolder, "Path tp the folder where the logs from the application will be generated.")
   ->check(CLI::ExistingDirectory);


    CLI11_PARSE(clientApplication, argc, argv);

    const auto timeStampStringForFile = makeSafeForFilename(getTimeStamp(currentSecondsSinceEpoch()));
    const std::string logFile = std::string(CLIENT_TARGET_NAME) + "_" + timeStampStringForFile + ".log";

    const auto logger = getLogger(
        CLIENT_TARGET_NAME,
        (std::filesystem::path(loggingFolder) / logFile).string()
    );

  logger->info("Starting {} version {}", CLIENT_TARGET_NAME, PROJECT_VERSION);

  SimpleTcpClient cli;
    std::vector<server::messages::NewMessageReceived> messages;

  cli.on_connect([&cli, &logger, &username]
  {
      client::messages::InitialConnection msg;
      msg.username = username;
      cli.write(msg);
      logger->info("Connected to server");
  });

  cli.on_disconnect([&cli, &logger]
  {
      client::messages::Disconnect msg;
      msg.reason = "Just left";
      cli.write(msg);
      logger->info("Disconnected from server");
  });

  cli.on_message([&cli , &logger, &messages](const server::messages::ServerMessage&& msg)
  {
      std::visit(overloaded{
  [](const server::messages::UserConnected& v)
  {
      std::cout << "Connected = " << v.username << "\n";
  },
  [](const server::messages::UserDisconnected& v) {
      std::cout << "User disconnected: = " << v.reason << "\n";
  },
  [&messages](const server::messages::NewMessageReceived& v)
  {
      messages.emplace_back(v);
      std::cout << "Message = " << v.username << "\n";
  }
      }, msg);
  });

  cli.connect(serverIp, serverPort);


    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        logger->error("Failed to initialize SDL - {}", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    constexpr auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        logger->error("Failed to create window - {}", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        logger->error("Failed to create renderer");
        return -1;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    logger->info("SDL_RendererInfo info: {}", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    constexpr auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Input"))
        {
            static char buf[256] = "";

            // Or if you want it to send on Enter:
            if (ImGui::InputText("##msg", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                client::messages::NewMessage msg;
                msg.message = buf;
                cli.write(msg);
                buf[0] = '\0';
            }
            ImGui::SameLine();
            if ( ImGui::Button("Send"))
            {
                client::messages::NewMessage msg;
                msg.message = buf;
                cli.write(msg);
                buf[0] = '\0';
            }
            ImGui::End();
        }

        if (ImGui::Begin("Chat"))
        {
            float avail_width = ImGui::GetContentRegionAvail().x;
            for (const auto& message : messages)
            {
                MessageBubble(message.message.c_str(), false, avail_width);
            }
            ImGui::End();
        }



        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }


    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // stop server
    cli.stop();


    return 0;
}
