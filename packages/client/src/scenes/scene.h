#pragma once

#include "../imgui_client.h"

//std
#include <string>

class Scene
{
protected:

  explicit Scene(spdlog::logger* logger ,const std::unordered_map<TexturesEnum, SDL_Texture*>& textures
    , const DataManager& data) :
  logger_(logger),
  textures(textures),
  data(data)
  {
  }

  virtual ~Scene() = default;

public:
  // The return type of the update is an optional ScenesEnum, if it contains a value
  // it means the scene want to transition to the next scene
  [[nodiscard]] virtual std::optional<ScenesEnum> update() = 0;

protected:
  [[nodiscard]] SDL_Texture* getTexture(const TexturesEnum& textureName) const
  {
    if (const auto it = textures.find(textureName); it != textures.end())
    {
      return it->second;
    }

    logger_->error("Could not find texture '{}'", static_cast<u8>(textureName));
    return nullptr;
  }

  [[nodiscard]] const DataManager& getData()const noexcept {return data;}

private:
  spdlog::logger* logger_;
  const std::unordered_map<TexturesEnum, SDL_Texture*>& textures;
  const DataManager& data;
};
