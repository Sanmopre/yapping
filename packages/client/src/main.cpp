#include "tcp_client.h"
#include "cmake_constants.h"
#include "chat_imgui_components.h"

// imgui
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

// std
#include <stdio.h>

// sdl
#include "SDL.h"

// cli11
#include "CLI/CLI.hpp"


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
    std::vector<server::messages::ServerMessage> messages;

  cli.on_connect([&cli, &logger, &username]
  {
      client::messages::InitialConnection msg;
      msg.username = username;
      cli.write(msg);
      logger->info("Connected to server");
  });

  cli.on_disconnect([&cli, &logger]
  {
      logger->info("Disconnected from server");
  });

  cli.on_message([&cli , &logger, &messages](const server::messages::ServerMessage&& msg)
  {
    messages.emplace_back(msg);
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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







        // If you want the dockspace to take the entire viewport:
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        // DockSpace ID
        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();













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

        }

        ImGui::End();
        if (ImGui::Begin("Chat"))
        {
            for (const auto& message : messages)
            {
                renderServerMessage(message, username);
            }

        }
        ImGui::End();


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
