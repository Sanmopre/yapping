#include "tcp_client.h"
#include "cmake_constants.h"
#include "chat_imgui_components.h"

#include "client_application.h"

// imgui
#include "imgui_impl_sdl2.h"

// std
#include <stdio.h>

// sdl
#include "SDL.h"

// cli11
#include "CLI/CLI.hpp"


// Main code
int main(int argc, char** argv)
{
    CLI::App clientCliApplication(CLIENT_DESCRIPTION);
    clientCliApplication.set_version_flag("--version", PROJECT_VERSION);

    // Arguments for the client
    std::string username = "random";
    std::string serverIp;
    std::string loggingFolder = "./logs";
    u16 serverPort;

    clientCliApplication.add_option("-u,--username", username,
    "Username for the client to use when connecting")->check([](const std::string &input) {
        if (input.size() > 12)
        {
            return std::string("Username must be at most 12 characters long");
        }
        return std::string{}; // no error
    });

    clientCliApplication.add_option("-i,--ip", serverIp,
        "IPv4 address of the server to connect to")
        ->required()
        ->check(CLI::ValidIPV4);

    clientCliApplication.add_option("-p,--port", serverPort,
        "Port of the server to connect to")
        ->required()
        ->check(CLI::Range(1, 65535));

    clientCliApplication.add_option("-l,--log-folder", loggingFolder, "Path tp the folder where the logs from the application will be generated.")
   ->check(CLI::ExistingDirectory);


    CLI11_PARSE(clientCliApplication, argc, argv);

    const auto timeStampStringForFile = makeSafeForFilename(getTimeStamp(currentSecondsSinceEpoch()));
    const std::string logFile = std::string(CLIENT_TARGET_NAME) + "_" + timeStampStringForFile + ".log";

    const auto logger = getLogger(
        CLIENT_TARGET_NAME,
        (std::filesystem::path(loggingFolder) / logFile).string()
    );

    logger->info("Starting {} version {}", CLIENT_TARGET_NAME, PROJECT_VERSION);


    const auto clientApplication = std::make_unique<ClientApplication>(username, serverIp, serverPort, logger.get());

    if (!clientApplication->initialize())
    {
        logger->error("Failed to initialize client application");
        return EXIT_FAILURE;
    }

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
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(clientApplication->getWindow()))
                done = true;
        }
        if (SDL_GetWindowFlags(clientApplication->getWindow()) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        clientApplication->update();

    }

    return EXIT_SUCCESS;
}
