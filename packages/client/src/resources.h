#pragma once

#include "compression_utils.h"

// sdl2
#include "SDL.h"

// imgui
#include "imgui.h"

// spdlog
#include "spdlog/logger.h"

// std
#include <unordered_map>

enum class TexturesEnum
{
  LOGO_TEXTURE,
  BACKGROUND_TEXTURE,
  SEND_BUTTON_TEXTURE
};

enum class FontsEnum
{
  LIBERATION_MONO_REGULAR_FONT
};

[[nodiscard]] inline std::unordered_map<TexturesEnum, SDL_Texture*> loadTextureMap(SDL_Renderer* renderer ,spdlog::logger* logger)
{
  // hex dumps
  #include "background.h"
  #include "logo.h"
  #include "send_button.h"

  const auto getTexture = [renderer](const unsigned char *compressedSource, size_t compressedLenght)
  {
    const auto uncompressedData = gunzipInMemory(compressedSource, compressedLenght);
    return SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP_RW(SDL_RWFromConstMem(uncompressedData.data(), uncompressedData.size()), 1));
  };

  std::unordered_map<TexturesEnum, SDL_Texture*> textureMap;

  // Load logo texture
  logger->info("Loading logo texture");
  textureMap.try_emplace(TexturesEnum::LOGO_TEXTURE, getTexture(logo_bmp_gz, logo_bmp_gz_len));

  // Load chat background texture
  logger->info("Loading chat background texture");
  textureMap.try_emplace(TexturesEnum::BACKGROUND_TEXTURE, getTexture(background_bmp_gz, background_bmp_gz_len));

  // Load send button texture
  logger->info("Loading send button texture");
  textureMap.try_emplace(TexturesEnum::SEND_BUTTON_TEXTURE, getTexture(send_button_bmp_gz, send_button_bmp_gz_len));

  return textureMap;
}

[[nodiscard]] inline std::unordered_map<FontsEnum, ImFont*> loadFontsMap(spdlog::logger* logger)
{
  // hex dumps
  #include "liberation_mono_regular.h"

  std::unordered_map<FontsEnum, ImFont*> fontsMap;

  ImFontConfig font_cfg;
  font_cfg.FontDataOwnedByAtlas = false;

  logger->info("Loading liberation_mono_regular font");
  fontsMap.try_emplace(FontsEnum::LIBERATION_MONO_REGULAR_FONT, ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
      (void*)liberation_mono_regular_ttf,
      static_cast<int>(liberation_mono_regular_ttf_len),
      15.0f,
      &font_cfg,
      ImGui::GetIO().Fonts->GetGlyphRangesDefault()
  )
);

  return fontsMap;
}