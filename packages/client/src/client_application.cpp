#include "client_application.h"
#include "chat_imgui_components.h"
#include "compression_utils.h"

// imgui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

// hex_dumps
#include "logo.h"
#include "background.h"
#include "send_button.h"
#include "liberation_mono_regular.h"

// cmake_constants
#include "cmake_constants.h"


ClientApplication::ClientApplication(const DataManager &data,
                                     spdlog::logger *logger) : data(data), logger_(logger)
{
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
                               WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
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

    logger_->info("Loading liberation_mono_regular_ttf font");
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;

    std::ignore = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
        (void*)liberation_mono_regular_ttf,
        static_cast<int>(liberation_mono_regular_ttf_len),
        15.0f,
        &font_cfg,
        ImGui::GetIO().Fonts->GetGlyphRangesDefault()
    );

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer_);

    // Load logo texture
    logger_->info("Loading logo texture");
    textures_.logo = getTexture(logo_bmp_gz, logo_bmp_gz_len);

    // Load chat background texture
    logger_->info("Loading chat background texture");
    textures_.chatBackground = getTexture(background_bmp_gz, background_bmp_gz_len);

    // Load send button texture
    logger_->info("Loading send button texture");
    textures_.sendButton = getTexture(send_button_bmp_gz, send_button_bmp_gz_len);

    // set style
    setStyle(ImGui::GetStyle());

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

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* bg = ImGui::GetBackgroundDrawList(vp);
    bg->AddImage(reinterpret_cast<ImTextureID>(textures_.chatBackground),
                 vp->Pos,
                 ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y),
                 ImVec2(0,0), ImVec2(1,1), IM_COL32(255,255,255,255));

    renderUsersWindow(data.getUsers());

    if (ImGui::BeginMainMenuBar())
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(textures_.logo), ImVec2(32,32));
        ImGui::SameLine();
        ImGui::Text(PROJECT_VERSION);
    }
    ImGui::EndMainMenuBar();

    if (ImGui::Begin("Input"))
    {
        // Or if you want it to send on Enter:
        if (ImGui::InputText("##msg", messageBuff_, IM_ARRAYSIZE(messageBuff_), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            sendMessageContent();
        }
        ImGui::SameLine();

        ImVec2 size(20, 20);
        if (ImGui::ImageButton("send_button", reinterpret_cast<ImTextureID>(textures_.sendButton), size))
        {
            sendMessageContent();
        }
    }
    ImGui::End();

    if (ImGui::Begin("Chat"))
    {
        bool isAtBottom =  ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;

        for (const auto &message : data.getMessages())
        {
            if (const auto it = data.getUsers().find(message.username); it != data.getUsers().end())
            {
                renderServerMessage(message, data.getUsername(), data.getUsers().at(message.username).color);
            }
            else
            {
                renderServerMessage(message, data.getUsername(), server::messages::UserColor{100, 100, 100});
            }
        }

        if (isAtBottom)
        {
            ImGui::SetScrollHereY(0.0f);
        }

    }
    ImGui::End();


    // Post rendering functions needed for frame clean up
    postRender();
}

void ClientApplication::sendMessageContent()
{
    if (const auto response = data.sendMessage(std::string(messageBuff_)); response)
    {
        messageBuff_[0] = '\0';
    }
}

SDL_Texture *
ClientApplication::getTexture(const unsigned char *compressedSource,
                              size_t compressedLenght) const {
    const auto uncompressedData = gunzipInMemory(compressedSource, compressedLenght);
    return SDL_CreateTextureFromSurface(renderer_, SDL_LoadBMP_RW(SDL_RWFromConstMem(uncompressedData.data(), uncompressedData.size()), 1));
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

    ImGuiWindowFlags window_flags =  ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);


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
