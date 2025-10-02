#include "server_selection_scene.h"
ServerSelectionScene::ServerSelectionScene(
    spdlog::logger *logger,
    const std::unordered_map<TexturesEnum, SDL_Texture *> &textures,
    const DataManager &data,
    const std::string& defaultIp,
    u16 defaultPort)
        : Scene(logger, textures, data)
{
  std::strncpy(ipv4Address_, defaultIp.c_str(), sizeof(ipv4Address_) - 1);
  ipv4Address_[sizeof(ipv4Address_) - 1] = '\0';
  port_ = defaultPort;
}

std::optional<ScenesEnum> ServerSelectionScene::update()
{
  return drawServerSelection();
}

std::optional<ScenesEnum> ServerSelectionScene::drawServerSelection()
{
  ImGui::Begin("Server Selection");

  ImGui::Text("Address:");
  if (ImGui::InputText("##ip", ipv4Address_, IM_ARRAYSIZE(ipv4Address_), ImGuiInputTextFlags_EnterReturnsTrue))
  {
  }

  ImGui::Text("Port: ");
  ImGui::InputInt("##port", &port_,1,100,0);


  if (ImGui::Button("Connect"))
  {
    getData().connect(std::string(ipv4Address_), static_cast<u16>(port_));
    ImGui::End();
    return ScenesEnum::REGISTER_LOGIN_SCENE;
  }

  ImGui::End();
  return std::nullopt;
}