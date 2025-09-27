#pragma once

#include "tcp_client.h"

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

class ClientApplication
{
  public:
    ClientApplication(const std::string &username, const std::string &host, u16 port, spdlog::logger *logger);
    ~ClientApplication();

  public:
    [[nodiscard]] bool initialize();
    void update();

  public:
    [[nodiscard]] SDL_Window *getWindow() const noexcept;

  private:
    void preRender();
    void postRender();
    void render();

private:
  [[nodiscard]] SDL_Texture* getTexture(const unsigned char* compressedSource, size_t compressedLenght) const;

  private:
    spdlog::logger *logger_;
    std::unique_ptr<SimpleTcpClient> tcpClient_;

  private:
    // sdl
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    ApplicationTextures textures_;

  private:
    // Data containers
    const std::string username_;
    std::map<std::string, UserData> usersMap_;
    std::vector<server::messages::NewMessageReceived> messages_;
};
