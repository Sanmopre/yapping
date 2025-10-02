#pragma once

#include "scene.h"

class ServerSelectionScene final : public Scene
{
public:
  ServerSelectionScene(spdlog::logger* logger ,const std::unordered_map<TexturesEnum, SDL_Texture*>& textures
    , const DataManager& data, const std::string& defaultIp, u16 defaultPort);
  ~ServerSelectionScene() override = default;

public:
  [[nodiscard]] std::optional<ScenesEnum> update() override;

private:
  [[nodiscard]] std::optional<ScenesEnum>  drawServerSelection();

private:
  char ipv4Address_[16] = "";
  i32 port_;
};