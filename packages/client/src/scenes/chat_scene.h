#pragma once

#include "scene.h"

class ChatScene final : public Scene
{
public:
  ChatScene(spdlog::logger* logger ,const std::unordered_map<TexturesEnum, SDL_Texture*>& textures
    , const DataManager& data);
  ~ChatScene() override = default;

public:

  [[nodiscard]] std::optional<ScenesEnum> update() override;

private:
  void drawChat();
  void drawUsers();
  void drawInput();
  void drawTopBar();

private:
  void sendMessageContent();

private:
  // Data containers
  char messageBuff_[MAX_MESSAGE_LENGTH] = "";
};