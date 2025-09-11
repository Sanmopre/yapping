#pragma once

#include "messages.h"

// imgui
#include "imgui.h"
#include "imgui_internal.h"



// Pass an optional small_font (e.g., 12–13px). If null, it uses current font.
static void renderMessageBubble(const char* text,
                                const char* author,
                                const char* when,
                                bool is_outgoing,
                                float avail_width,
                                ImFont* small_font = nullptr,
                                ImU32 col_bg_incoming  = IM_COL32(235,236,240,255),
                                ImU32 col_bg_outgoing  = IM_COL32(180,232,149,255),
                                ImU32 col_text         = IM_COL32(0,0,0,255),
                                ImU32 col_meta         = IM_COL32(120,120,120,255))
{
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const float padding_x    = 10.0f;
    const float padding_y    = 8.0f;
    const float rounding     = 12.0f;
    const float space_y      = 6.0f;   // spacing after the bubble
    const float meta_space_y = 2.0f;   // spacing between meta line and bubble

    // Limit bubble width to ~65% of available space
    float max_width = ImClamp(avail_width * 0.65f, 80.0f, avail_width);

    // Measure wrapped message text
    ImVec2 text_size = ImGui::CalcTextSize(text, nullptr, true, max_width);
    ImVec2 bubble_size(text_size.x + padding_x * 2.0f,
                       text_size.y + padding_y * 2.0f);

    // Build meta header: "author • when"
    char header[256];
    ImFormatString(header, IM_ARRAYSIZE(header), "%s  |  %s", author, when);

    // Measure header with small font if provided
    ImVec2 header_size = small_font
        ? small_font->CalcTextSizeA(small_font->FontSize, FLT_MAX, 0.0f, header)
        : ImGui::CalcTextSize(header);

    // Cursor (top-left of where we want to place this message block)
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    // Compute bubble rect (shifted down by header height)
    ImVec2 bubble_min;
    bubble_min.x = is_outgoing ? (cursor.x + avail_width - bubble_size.x) : cursor.x;
    bubble_min.y = cursor.y + header_size.y + meta_space_y;

    ImVec2 bubble_max = ImVec2(bubble_min.x + bubble_size.x,
                               bubble_min.y + bubble_size.y);

    // Meta/header position: align with bubble (left for incoming, right for outgoing)
    ImVec2 header_pos = is_outgoing
        ? ImVec2(bubble_max.x - header_size.x, cursor.y)
        : ImVec2(bubble_min.x,                 cursor.y);

    // Draw header (smaller, gray)
    if (small_font)
        dl->AddText(small_font, small_font->FontSize, header_pos, col_meta, header);
    else
        dl->AddText(header_pos, col_meta, header);

    // Draw bubble background
    ImU32 col_bg = is_outgoing ? col_bg_outgoing : col_bg_incoming;
    dl->AddRectFilled(bubble_min, bubble_max, col_bg, rounding);

    // Draw wrapped message text inside bubble
    ImVec2 text_pos(bubble_min.x + padding_x, bubble_min.y + padding_y);
    ImGui::SetCursorScreenPos(text_pos);

    float inner_wrap = text_pos.x + (bubble_size.x - padding_x * 2.0f);
    ImGui::PushTextWrapPos(inner_wrap);
    ImGui::PushStyleColor(ImGuiCol_Text, col_text);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();

    // Advance cursor for next message
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, bubble_max.y + space_y));
}

void inline renderUserDisconnected(const server::messages::UserDisconnected& value)
{
    const std::string msg = "User " + value.username + " disconnected at " + getTimeStamp(value.timestamp);
    ImGui::TextUnformatted(msg.c_str());
}

void inline renderUserConnected(const server::messages::UserConnected& value)
{
    const std::string msg = "User " + value.username + " connected at " + getTimeStamp(value.timestamp);
    ImGui::TextUnformatted(msg.c_str());
}
void inline renderServerMessage(const server::messages::ServerMessage& msg, const std::string& username)
{
    std::visit(overloaded{
    [&msg](const server::messages::UserConnected& value)
    {
        renderUserConnected(value);
    },
    [&msg](const server::messages::UserDisconnected& value)
    {
        renderUserDisconnected(value);
    },
    [&msg, &username](const server::messages::NewMessageReceived& value)
    {
        renderMessageBubble(value.message.c_str(), value.username.c_str() , getTimeStamp(value.timestamp).c_str() , (username == value.username), ImGui::GetContentRegionAvail().x);
    }
    }, msg);
}