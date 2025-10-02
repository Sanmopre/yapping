#pragma once

#include "scene.h"

class LoginRegisterScene final : public Scene
{
public:
  LoginRegisterScene(spdlog::logger* logger ,const std::unordered_map<TexturesEnum, SDL_Texture*>& textures
    , const DataManager& data, const std::string& defaultUsername);
  ~LoginRegisterScene() override = default;

public:
  [[nodiscard]] std::optional<ScenesEnum> update() override;

private:
  [[nodiscard]] std::optional<ScenesEnum>  drawLogin();

private:
  // Data containers
  char usernameBuff_[12] = "";
  char passwordBuff_[32] = "";
};