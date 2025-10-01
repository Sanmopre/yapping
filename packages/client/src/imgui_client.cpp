#include "chat_imgui_components.h"
#include "compression_utils.h"
#include "imgui_client.h"

// imgui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

// cmake_constants
#include "cmake_constants.h"
#include "scenes/scene.h"

ImguiClient::ImguiClient(spdlog::logger *logger) : logger_(logger)
{
}

ImguiClient::~ImguiClient()
{
    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool ImguiClient::initialize()
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

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer_);

    // set style
    setStyle(ImGui::GetStyle());

    return true;
}

void ImguiClient::update()
{
    // Pre render functions needed for rendering
    preRender();

    if (currentScene_ == nullptr)
    {
        logger_->error("Current scene is nullptr");
        return;
    }

    // Update the current scene, and check if the scene requested a scene change
    if (const auto maybeNextScene = currentScene_->update(); maybeNextScene.has_value())
    {
        if (const auto it = scenes_.find(maybeNextScene.value()); it != scenes_.end())
        {
            currentScene_ = scenes_.at(maybeNextScene.value());
        }
        else
        {
            logger_->error("Requested scene {} not found", static_cast<u8>(maybeNextScene.value()));
        }
    }

    // Post rendering functions needed for frame clean up
    postRender();
}

SDL_Window *ImguiClient::getWindow() const noexcept
{
    return window_;
}

SDL_Renderer *ImguiClient::getRenderer() const noexcept
{
    return renderer_;
}

void ImguiClient::addScene(ScenesEnum sceneType, std::shared_ptr<Scene> scene)
{
    scenes_.try_emplace(sceneType, scene);
}

void ImguiClient::setCurrentScene(ScenesEnum sceneType)
{
    if (const auto it = scenes_.find(sceneType); it != scenes_.end())
    {
        currentScene_ = it->second;
    }
}

void ImguiClient::preRender()
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

void ImguiClient::postRender()
{
    // Rendering
    ImGui::Render();
    SDL_RenderSetScale(renderer_, ImGui::GetIO().DisplayFramebufferScale.x, ImGui::GetIO().DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
    SDL_RenderClear(renderer_);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
}
