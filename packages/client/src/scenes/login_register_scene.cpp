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
  if (waitingForServerResponse_)
  {
   switch (getData().getServerResponseCode())
   {
   case ServerResponseCode::INCORRECT_PASSWORD:
     break;
   case ServerResponseCode::NONE:
      break;
   case ServerResponseCode::SUCCESSFUL_LOGIN:
     {
       client::messages::InitialConnection initialConnection;
       initialConnection.username = usernameBuff_;
       std::ignore = getData().sendMessage(initialConnection.toString());
     getData().setUsername(usernameBuff_);
       return ScenesEnum::CHAT_SCENE;
     }
   case ServerResponseCode::SUCCESSFUL_REGISTRATION:
     {
       client::messages::InitialConnection initialConnection;
       initialConnection.username = usernameBuff_;
       std::ignore = getData().sendMessage(initialConnection.toString());
     getData().setUsername(usernameBuff_);
       return ScenesEnum::CHAT_SCENE;
     }
   case ServerResponseCode::USERNAME_ALREADY_EXISTS:
     break;
   case ServerResponseCode::USERNAME_DOES_NOT_EXIST:
      break;
   }
  }
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
    client::messages::Login lgnMessage;
    lgnMessage.passwordHash = hashImpl(std::string(passwordBuff_));
    lgnMessage.username = std::string(usernameBuff_);
    std::ignore = getData().sendMessage(lgnMessage.toString());
    waitingForServerResponse_ = true;
  }
  ImGui::SameLine();

  if (ImGui::Button("Register"))
  {
    client::messages::Register rgsMessage;
    rgsMessage.passwordHash = hashImpl(std::string(passwordBuff_));
    rgsMessage.username = std::string(usernameBuff_);
    std::ignore = getData().sendMessage(rgsMessage.toString());
    waitingForServerResponse_ = true;
  }
  ImGui::End();

  return std::nullopt;
}