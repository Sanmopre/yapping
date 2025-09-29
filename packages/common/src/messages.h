#pragma once

#include "global.h"

// nlohmann
#include "json.hpp"

// std
#include <chrono>
#include <filesystem>
#include <string>
#include <variant>

namespace server::messages
{

struct UserColor
{
    u8 red = 0;
    u8 green = 0;
    u8 blue = 0;
};

struct ServerResponse
{
    explicit ServerResponse(const nlohmann::json &data)
    {
        code = data[SERVER_RESPONSE_CODE_KEY].get<ServerResponseCode>();
    }

    ServerResponse() = default;

    [[nodiscard]] std::string toString() const noexcept
    {
        nlohmann::json data;
        data[PACKET_HEADER_KEY] = ServerMessageType::SERVER_RESPONSE;

        nlohmann::json content;
        content[SERVER_RESPONSE_CODE_KEY] = code;
        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    ServerResponseCode code;
};

struct NewMessageReceived
{
    explicit NewMessageReceived(const nlohmann::json &data)
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
    explicit UserStatus(const nlohmann::json &data)
    {
        username = data[USERNAME_KEY].get<std::string>();
        timestamp = data[TIMESTAMP_KEY].get<u64>();
        status = data[USER_STATUS_KEY].get<UserStatusType>();
        color.red = data[USER_COLOR_KEY][COLOR_RED_KEY].get<u8>();
        color.blue = data[USER_COLOR_KEY][COLOR_BLUE_KEY].get<u8>();
        color.green = data[USER_COLOR_KEY][COLOR_GREEN_KEY].get<u8>();
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

        nlohmann::json colorKey;
        colorKey[COLOR_RED_KEY] = color.red;
        colorKey[COLOR_BLUE_KEY]= color.blue;
        colorKey[COLOR_GREEN_KEY]= color.green;

        content[USER_COLOR_KEY] = colorKey;

        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    std::string username;
    UserStatusType status;
    UserColor color;
    u64 timestamp;
};

using ServerMessage = std::variant<UserStatus, NewMessageReceived, ServerResponse>;

} // namespace server::messages

namespace client::messages
{

struct InitialConnection
{
    explicit InitialConnection(const nlohmann::json &data)
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
    explicit NewMessage(const nlohmann::json &data)
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


struct Login
{
    explicit Login(const nlohmann::json &data)
    {
        username = data[USERNAME_KEY].get<std::string>();
        passwordHash = data[PASSWORD_HASH_KEY].get<u64>();
    }
    Login() = default;

    [[nodiscard]] std::string toString() const noexcept
    {
        nlohmann::json data;
        data[PACKET_HEADER_KEY] = ClientMessageType::LOGIN;

        nlohmann::json content;
        content[USERNAME_KEY] = username;
        content[PASSWORD_HASH_KEY] = passwordHash;

        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    std::string username;
    u64 passwordHash;
};

struct Register
{
    explicit Register(const nlohmann::json &data)
    {
        username = data[USERNAME_KEY].get<std::string>();
        passwordHash = data[PASSWORD_HASH_KEY].get<u64>();
    }
    Register() = default;

    [[nodiscard]] std::string toString() const noexcept
    {
        nlohmann::json data;
        data[PACKET_HEADER_KEY] = ClientMessageType::REGISTER;

        nlohmann::json content;
        content[USERNAME_KEY] = username;
        content[PASSWORD_HASH_KEY] = passwordHash;

        data[PACKET_CONTENT_KEY] = content;

        return data.dump();
    }

    std::string username;
    u64 passwordHash;
};


using ClientMessage = std::variant<NewMessage, InitialConnection, Login, Register>;

} // namespace client::messages

// Timestamp functions
// ////////////////////////////////////////////////////////////

[[nodiscard]] inline std::string getTimeStamp(u64 secondsSinceEpoch)
{
    const std::chrono::system_clock::time_point tp{std::chrono::seconds(secondsSinceEpoch)};
    const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    return std::ctime(&tt);
}

[[nodiscard]] inline std::string makeSafeForFilename(const std::string &input)
{
    std::string result = input;
    for (char &c : result)
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
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count();
}

struct UserData
{
    UserStatusType status;
    server::messages::UserColor color;
};
