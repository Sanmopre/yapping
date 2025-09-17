#pragma once

#include "global.h"

// nlohmann
#include "json.hpp"

// std
#include <string>
#include <chrono>
#include <variant>
#include <filesystem>

namespace server::messages
{

struct NewMessageReceived
{
    explicit NewMessageReceived(const nlohmann::json& data)
    {
        username = data[USERNAME_KEY].get<std::string>();
        message = data[MESSAGE_KEY].get<std::string>();
        timestamp = data[TIMESTAMP_KEY].get<u64>();
    }
    NewMessageReceived() = default;

    [[nodiscard]] std::string toString() const noexcept
    {
        nlohmann::json data;
        data[PACKET_HEADER_KEY] = ServerMessageType::RECEIVED_MESSAGE;

        nlohmann::json content;
        content[USERNAME_KEY] = username;
        content[TIMESTAMP_KEY] = timestamp;
        content[MESSAGE_KEY] = message;

        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    std::string username;
    std::string message;
    u64 timestamp;
};

struct UserStatus
{
    explicit UserStatus(const nlohmann::json& data)
    {
        username = data[USERNAME_KEY].get<std::string>();
        timestamp = data[TIMESTAMP_KEY].get<u64>();
        status = data[USER_STATUS_KEY].get<UserStatusType>();
    }
    UserStatus() = default;

    [[nodiscard]] std::string toString() const noexcept
    {
        nlohmann::json data;
        data[PACKET_HEADER_KEY] = ServerMessageType::USER_STATUS;

        nlohmann::json content;
        content[USERNAME_KEY] = username;
        content[TIMESTAMP_KEY] = timestamp;
        content[USER_STATUS_KEY] = status;

        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    std::string username;
    UserStatusType status;
    u64 timestamp;
};

using ServerMessage = std::variant<UserStatus, NewMessageReceived>;

}


namespace client::messages
{

struct InitialConnection
{
    explicit InitialConnection(const nlohmann::json& data)
    {
        username = data[USERNAME_KEY].get<std::string>();
    }
    InitialConnection() = default;

    [[nodiscard]] std::string toString() const noexcept
    {
        nlohmann::json data;
        data[PACKET_HEADER_KEY] = ClientMessageType::INITIAL_CONNECTION;

        nlohmann::json content;
        content[USERNAME_KEY] = username;

        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    std::string username;
};


struct NewMessage
{
    explicit NewMessage(const nlohmann::json& data)
    {
        message = data[MESSAGE_KEY].get<std::string>();
    }
    NewMessage() = default;

    [[nodiscard]] std::string toString() const noexcept
    {
        nlohmann::json data;
        data[PACKET_HEADER_KEY] = ClientMessageType::NEW_MESSAGE;

        nlohmann::json content;
        content[MESSAGE_KEY] = message;

        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    std::string message;
};

using ClientMessage = std::variant<NewMessage, InitialConnection>;

}

// Timestamp functions ////////////////////////////////////////////////////////////

[[nodiscard]] inline std::string getTimeStamp(u64 secondsSinceEpoch)
{
    const std::chrono::system_clock::time_point tp{std::chrono::seconds(secondsSinceEpoch)};
    const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    return std::ctime(&tt);
}

[[nodiscard]] inline std::string makeSafeForFilename(const std::string& input)
{
    std::string result = input;
    for (char& c : result)
    {
        if (std::isspace(static_cast<unsigned char>(c)) || c == ':' || c == '/' || c == '\\')
            c = '_';
    }
    // strip trailing underscores (from the newline replacement)
    while (!result.empty() && result.back() == '_')
        result.pop_back();
    return result;
}

[[nodiscard]] inline u64 currentSecondsSinceEpoch()
{
    const auto currentTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
        currentTime.time_since_epoch()).count();
}

