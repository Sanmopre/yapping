#include "client_application.h"
#include "chat_imgui_components.h"

// imgui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

// hex_dumps
#include "logo.h"

// cmake_constants
#include "cmake_constants.h"

ClientApplication::ClientApplication(const std::string &username, const std::string &host, u16 port,
                                     spdlog::logger *logger)
    : username_(username), logger_(logger), tcpClient_(std::make_unique<SimpleTcpClient>())
{
    tcpClient_->on_connect([&] {
        client::messages::InitialConnection msg;
        msg.username = username_;
        tcpClient_->write(msg);
        logger_->info("Connected to server");
    });

    tcpClient_->on_disconnect([&] { logger_->info("Disconnected from server"); });

    tcpClient_->on_message([&](const server::messages::ServerMessage &&msg) {
        std::visit(
            overloaded{[&](const server::messages::NewMessageReceived &value) { messages_.emplace_back(value); },
                       [&](const server::messages::UserStatus &value) { usersMap_[value.username] = value.status; }},
            msg);
    });

    tcpClient_->connect(host, port);
}

ClientApplication::~ClientApplication()
{
    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();

    // stop server
    tcpClient_->stop();
}

bool ClientApplication::initialize()
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        logger_->error("Failed to initialize SDL - {}", SDL_GetError());
        return false;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    constexpr auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ = SDL_CreateWindow(CLIENT_TARGET_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               1280, 720, window_flags);
    if (window_ == nullptr)
    {
        logger_->error("Failed to create window - {}", SDL_GetError());
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer_)
    {
        logger_->error("Failed to create renderer");
        return false;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer_, &info);
    logger_->info("SDL_RendererInfo info: {}", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer_);


    // Load logo texture
    SDL_RWops* rw = SDL_RWFromConstMem(logo_bmp, logo_bmp_len);
    SDL_Surface* surface = SDL_LoadBMP_RW(rw, 0);
    logoTexture_ = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);

    return true;
}

void ClientApplication::update()
{
    render();
}

void ClientApplication::render()
{
    // Pre render functions needed for rendering
    preRender();

    renderUsersWindow(usersMap_);

    if (ImGui::Begin("Input"))
    {
        static char messageBuff[MAX_MESSAGE_LENGTH] = "";

        // Or if you want it to send on Enter:
        if (ImGui::InputText("##msg", messageBuff, IM_ARRAYSIZE(messageBuff), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            client::messages::NewMessage msg;
            msg.message = messageBuff;
            tcpClient_->write(msg);
            messageBuff[0] = '\0';
        }
        ImGui::SameLine();
        if (ImGui::Button("Send"))
        {
            client::messages::NewMessage msg;
            msg.message = messageBuff;
            tcpClient_->write(msg);
            messageBuff[0] = '\0';
        }
    }
    ImGui::End();

    if (ImGui::Begin("Chat"))
    {
        bool isAtBottom =  ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;

        for (const auto &message : messages_)
        {            renderServerMessage(message, username_);}

        if (isAtBottom)
        {            ImGui::SetScrollHereY(0.0f);}

    }
    ImGui::End();


    if (ImGui::Begin("Logo"))
    {
    ImGui::Image(reinterpret_cast<ImTextureID>(logoTexture_), ImVec2(128,128));
    }

    ImGui::End();

    // Post rendering functions needed for frame clean up
    postRender();
}

SDL_Window *ClientApplication::getWindow() const noexcept
{
    return window_;
}

void ClientApplication::preRender()
{
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // If you want the dockspace to take the entire viewport:
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    // DockSpace ID
    const ImGuiID dockspaceId = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void ClientApplication::postRender()
{
    // Rendering
    ImGui::Render();
    SDL_RenderSetScale(renderer_, ImGui::GetIO().DisplayFramebufferScale.x, ImGui::GetIO().DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
    SDL_RenderClear(renderer_);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
}
