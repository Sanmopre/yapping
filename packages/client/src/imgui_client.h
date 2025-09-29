#pragma once

#include "data_manager.h"

// sdl2
#include "SDL.h"

constexpr u16 WINDOW_WIDTH = 1280;
constexpr u16 WINDOW_HEIGHT = 720;

struct ApplicationTextures
{
  SDL_Texture* logo;
  SDL_Texture* chatBackground;
  SDL_Texture* sendButton;
};

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

private:
  void preRender();
  void postRender();
  void render();
  void sendMessageContent();

private:
  [[nodiscard]] SDL_Texture* getTexture(const unsigned char* compressedSource, size_t compressedLenght) const;

private:
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
