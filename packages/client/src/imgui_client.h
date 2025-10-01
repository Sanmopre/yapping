#pragma once

#include "data_manager.h"

// sdl2
#include "SDL.h"

constexpr u16 WINDOW_WIDTH = 1280;
constexpr u16 WINDOW_HEIGHT = 720;

enum class ScenesEnum
{
  SELECT_SERVER_SCENE,
  REGISTER_LOGIN_SCENE,
  CHAT_SCENE
};

class Scene;

class ImguiClient
{
public:
  ImguiClient(spdlog::logger *logger);
  ~ImguiClient();

public:
  [[nodiscard]] bool initialize();
  void update();

public:
  [[nodiscard]] SDL_Window *getWindow() const noexcept;
  [[nodiscard]] SDL_Renderer *getRenderer() const noexcept;
  void addScene(ScenesEnum sceneType, std::shared_ptr<Scene> scene);
  void setCurrentScene(ScenesEnum sceneType);

private:
  void preRender();
  void postRender();

private:
  std::unordered_map<ScenesEnum, std::shared_ptr<Scene>> scenes_;
  std::shared_ptr<Scene> currentScene_;
  spdlog::logger *logger_;

private:
  // sdl
  SDL_Window *window_;
  SDL_Renderer *renderer_;
};
