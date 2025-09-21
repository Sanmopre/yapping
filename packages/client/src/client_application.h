#pragma once

#include "tcp_client.h"

// sdl2
#include "SDL.h"

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
    spdlog::logger *logger_;
    std::unique_ptr<SimpleTcpClient> tcpClient_;

  private:
    // sdl
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture* logoTexture_;

  private:
    // Data containers
    const std::string username_;
    std::map<std::string, UserStatusType> usersMap_;
    std::vector<server::messages::NewMessageReceived> messages_;
};
