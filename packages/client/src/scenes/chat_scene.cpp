#include "chat_scene.h"
#include "../chat_imgui_components.h"

// imgui
#include "imgui.h"

// cmake globals
#include "cmake_constants.h"


ChatScene::ChatScene(
    spdlog::logger *logger,
    const std::unordered_map<TexturesEnum, SDL_Texture *> &textures,
    const DataManager &data)
      : Scene(logger, textures, data)
{

}


std::optional<ScenesEnum> ChatScene::update()
{
  drawBackground();
  drawUsers();
  drawInput();
  drawTopBar();
  drawChat();

  return std::nullopt;
}

void ChatScene::drawChat()
{
  if (ImGui::Begin("Chat"))
  {
    bool isAtBottom =  ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;

    for (const auto &message : getData().getMessages())
    {
      const auto& users = getData().getUsers();
      if (const auto it = users.find(message.username); it != users.end())
      {
        renderServerMessage(message, getData().getUsername(), getData().getUsers().at(message.username).color);
      }
      else
      {
        renderServerMessage(message, getData().getUsername(), server::messages::UserColor{100, 100, 100});
      }
    }

    if (isAtBottom)
    {
      ImGui::SetScrollHereY(0.0f);
    }

  }
  ImGui::End();
}

void ChatScene::drawUsers()
{
  ImGui::Begin("Users");

  for (const auto &[user, data] : getData().getUsers())
  {
    // Current cursor position in window space
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float textHeight = ImGui::GetTextLineHeight();

    // Circle radius based on text size
    float radius = textHeight * 0.35f;
    ImVec2 center = ImVec2(pos.x + radius + 2.0f, pos.y + textHeight * 0.5f);

    ImU32 color;
    switch (data.status)
    {
    case UserStatusType::AWAY:
      color = IM_COL32(0, 200, 0, 255);
      break;
    case UserStatusType::OFFLINE:
      color = IM_COL32(128, 128, 128, 255);
      break;
    case UserStatusType::ONLINE:
      color = IM_COL32(0, 200, 0, 255);
      break;
    }

    // Draw circle
    ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, color, 16);

    // Add horizontal spacing so text doesn't overlap circle
    ImGui::Dummy(ImVec2(radius * 2.5f + 4.0f, textHeight));

    // Draw username on the same line, aligned with the circle
    ImGui::SameLine();
    ImGui::TextUnformatted(user.c_str());
  }

  ImGui::End();
}

void ChatScene::drawInput()
{
  if (ImGui::Begin("Input"))
  {
    // Or if you want it to send on Enter:
    if (ImGui::InputText("##msg", messageBuff_, IM_ARRAYSIZE(messageBuff_), ImGuiInputTextFlags_EnterReturnsTrue))
    {
      sendMessageContent();
    }
    ImGui::SameLine();

    ImVec2 size(20, 20);
    if (ImGui::ImageButton("send_button", reinterpret_cast<ImTextureID>(getTexture(TexturesEnum::SEND_BUTTON_TEXTURE)), size))
    {
      sendMessageContent();
    }
  }
  ImGui::End();
}

void ChatScene::drawTopBar()
{
  if (ImGui::BeginMainMenuBar())
  {
    ImGui::Image(reinterpret_cast<ImTextureID>(getTexture(TexturesEnum::LOGO_TEXTURE)), ImVec2(32,32));
    ImGui::SameLine();
    ImGui::Text(PROJECT_VERSION);
  }
  ImGui::EndMainMenuBar();
}

void ChatScene::drawBackground()
{
  ImGuiViewport* vp = ImGui::GetMainViewport();
  ImDrawList* bg = ImGui::GetBackgroundDrawList(vp);
  bg->AddImage(reinterpret_cast<ImTextureID>(getTexture(TexturesEnum::BACKGROUND_TEXTURE)),
               vp->Pos,
               ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y),
               ImVec2(0,0), ImVec2(1,1), IM_COL32(255,255,255,255));
}

void ChatScene::sendMessageContent()
{
  if (const auto response = getData().sendMessage(std::string(messageBuff_)); response)
  {
    messageBuff_[0] = '\0';
  }
}