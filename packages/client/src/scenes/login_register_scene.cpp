#include "login_register_scene.h"


LoginRegisterScene::LoginRegisterScene(
    spdlog::logger *logger,
    const std::unordered_map<TexturesEnum, SDL_Texture *> &textures,
    const DataManager &data
    , const std::string& defaultUsername)
        : Scene(logger, textures, data)
{
  std::strncpy(usernameBuff_, defaultUsername.c_str(), sizeof(usernameBuff_) - 1);
  usernameBuff_[sizeof(usernameBuff_) - 1] = '\0';
}

std::optional<ScenesEnum> LoginRegisterScene::update()
{
 return drawLogin();
}

[[nodiscard]] std::optional<ScenesEnum> LoginRegisterScene::drawLogin()
{
  ImGui::Begin("Login");

  ImGui::Text("Username: ");
  if (ImGui::InputText("##usr", usernameBuff_, IM_ARRAYSIZE(usernameBuff_), ImGuiInputTextFlags_EnterReturnsTrue))
  {
  }

  ImGui::Text("Username: ");
  if (ImGui::InputText("##psw", passwordBuff_, IM_ARRAYSIZE(passwordBuff_), ImGuiInputTextFlags_Password|ImGuiInputTextFlags_EnterReturnsTrue))
  {
  }

  if (ImGui::Button("Login"))
  {
    ImGui::End();
    client::messages::Login lgnMessage;
    lgnMessage.passwordHash = hashImpl(std::string(passwordBuff_));
    lgnMessage.username = std::string(usernameBuff_);
    std::ignore = getData().sendMessage(lgnMessage.toString());
    return ScenesEnum::CHAT_SCENE;
  }
  ImGui::SameLine();

  if (ImGui::Button("Register"))
  {
    ImGui::End();
    client::messages::Register rgsMessage;
    rgsMessage.passwordHash = hashImpl(std::string(passwordBuff_));
    rgsMessage.username = std::string(usernameBuff_);
    std::ignore = getData().sendMessage(rgsMessage.toString());
    return ScenesEnum::CHAT_SCENE;
  }
  ImGui::End();

  return std::nullopt;
}