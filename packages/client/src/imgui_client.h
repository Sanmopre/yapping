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

enum class TexturesEnum {
  LOGO_TEXTURE,
  BACKGROUND_TEXTURE,
  SEND_BUTTON_TEXTURE
};


struct ApplicationTextures
{
  SDL_Texture* logo;
  SDL_Texture* chatBackground;
  SDL_Texture* sendButton;
};

class Scene;

class ImguiClient
{
public:
  ImguiClient(const DataManager& data, spdlog::logger *logger);
  ~ImguiClient();

public:
  [[nodiscard]] bool initialize();
  void update();

public:
  [[nodiscard]] SDL_Window *getWindow() const noexcept;
  void addScene(ScenesEnum sceneType, std::shared_ptr<Scene> scene);
  void setCurrentScene(ScenesEnum sceneType);

private:
  void preRender();
  void postRender();

private:
  [[nodiscard]] SDL_Texture* getTexture(const unsigned char* compressedSource, size_t compressedLenght) const;

private:
  std::unordered_map<ScenesEnum, std::shared_ptr<Scene>> scenes_;
  std::shared_ptr<Scene> currentScene_;
  const DataManager& data;
  spdlog::logger *logger_;

private:
  // sdl
  SDL_Window *window_;
  SDL_Renderer *renderer_;
  ApplicationTextures textures_;

private:
  // Data containers
  char messageBuff_[MAX_MESSAGE_LENGTH] = "";
};
