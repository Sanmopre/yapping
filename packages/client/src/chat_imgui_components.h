#pragma once

#include "messages.h"

// imgui
#include "imgui.h"
#include "imgui_internal.h"


constexpr auto METADATA_TEXT_COLOR = IM_COL32(120, 120, 120, 170);


// Pass an optional small_font (e.g., 12–13px). If null, it uses current font.
static void renderMessageBubble(const std::string& text, const std::string& author, const std::string& timestamp, bool isOutgoing,
                                float avail_width,
                                ImFont *small_font = nullptr,
                                u32 bubbleColor = IM_COL32(235, 236, 240, 255),
                                u32 textColor = IM_COL32(0, 0, 0, 255))
{
    ImDrawList *dl = ImGui::GetWindowDrawList();

    const float padding_x = 10.0f;
    const float padding_y = 8.0f;
    const float rounding = 12.0f;
    const float space_y = 6.0f;      // spacing after the bubble
    const float meta_space_y = 2.0f; // spacing between meta line and bubble

    // Limit bubble width to ~65% of available space
    float max_width = ImClamp(avail_width * 0.65f, 80.0f, avail_width);

    // Measure wrapped message text
    ImVec2 text_size = ImGui::CalcTextSize(text.c_str(), nullptr, true, max_width);
    ImVec2 bubble_size(text_size.x + padding_x * 2.0f, text_size.y + padding_y * 2.0f);

    // Build meta header: "author • when"
    char header[128];
    ImFormatString(header, IM_ARRAYSIZE(header), "%s  |  %s", author.c_str(), timestamp.c_str());

    // Measure header with small font if provided
    ImVec2 header_size = small_font ? small_font->CalcTextSizeA(small_font->FontSize, FLT_MAX, 0.0f, header)
                                    : ImGui::CalcTextSize(header);

    // Cursor (top-left of where we want to place this message block)
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    // Compute bubble rect (shifted down by header height)
    ImVec2 bubble_min;
    bubble_min.x = isOutgoing ? (cursor.x + avail_width - bubble_size.x) : cursor.x;
    bubble_min.y = cursor.y + header_size.y + meta_space_y;

    ImVec2 bubble_max = ImVec2(bubble_min.x + bubble_size.x, bubble_min.y + bubble_size.y);

    // Meta/header position: align with bubble (left for incoming, right for
    // outgoing)
    ImVec2 header_pos = isOutgoing ? ImVec2(bubble_max.x - header_size.x, cursor.y) : ImVec2(bubble_min.x, cursor.y);

    // Draw header (smaller, gray)
    if (small_font)
        dl->AddText(small_font, small_font->FontSize, header_pos, METADATA_TEXT_COLOR, header);
    else
        dl->AddText(header_pos, METADATA_TEXT_COLOR, header);

    // Draw bubble background
    dl->AddRectFilled(bubble_min, bubble_max, bubbleColor, rounding);

    // Draw wrapped message text inside bubble
    ImVec2 text_pos(bubble_min.x + padding_x, bubble_min.y + padding_y);
    ImGui::SetCursorScreenPos(text_pos);

    float inner_wrap = text_pos.x + (bubble_size.x - padding_x * 2.0f);
    ImGui::PushTextWrapPos(inner_wrap);
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();

    // Advance cursor for next message
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, bubble_max.y + space_y));
}

void inline renderServerMessage(const server::messages::NewMessageReceived &msg, const std::string &username, server::messages::UserColor color)
{
    renderMessageBubble(msg.message, msg.username, getTimeStamp(msg.timestamp),(username == msg.username),
        ImGui::GetContentRegionAvail().x
        ,nullptr, IM_COL32(color.red, color.green, color.blue, 255) );
}

void inline renderUsersWindow(const std::map<std::string, UserData> &usersMap)
{
    ImGui::Begin("Users");

    for (const auto &[user, data] : usersMap)
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