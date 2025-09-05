#pragma once

#include "global.h"

// nlohmann
#include "json.hpp"

// std
#include <string>
#include <chrono>
#include <variant>

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

    std::string username;
    std::string message;
    u64 timestamp;
};

struct UserConnected
{
    explicit UserConnected(const nlohmann::json& data)
    {
        username = data[USERNAME_KEY].get<std::string>();
        timestamp = data[TIMESTAMP_KEY].get<u64>();
    };

    std::string username;
    u64 timestamp;
};

struct UserDisconnected
{
    explicit UserDisconnected(const nlohmann::json& data)
    {
        username = data[USERNAME_KEY].get<std::string>();
        reason = data[REASON_KEY].get<std::string>();
        timestamp = data[TIMESTAMP_KEY].get<u64>();
    };

    std::string username;
    std::string reason;
    u64 timestamp;
};

using ServerMessage = std::variant<UserConnected, UserConnected, NewMessageReceived>;

}


namespace client::messages
{

struct InitialConnection
{
    explicit InitialConnection(const nlohmann::json& data)
    {
        username = data[USERNAME_KEY].get<std::string>();
    };

    std::string username;
};

struct Disconnect
{
    explicit Disconnect(const nlohmann::json& data)
    {
        reason = data[REASON_KEY].get<std::string>();
    }

    std::string reason;
};

struct NewMessage
{
    explicit NewMessage(const nlohmann::json& data)
    {
        message = data[MESSAGE_KEY].get<std::string>();
    }

    std::string message;
};

using ClientMessage = std::variant<Disconnect, NewMessage, InitialConnection>;

}

// Timestamp functions ////////////////////////////////////////////////////////////

[[nodiscard]] inline std::string getTimeStamp(u64 secondsSinceEpoch)
{
    const std::chrono::system_clock::time_point tp{std::chrono::seconds(secondsSinceEpoch)};
    const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    return std::ctime(&tt);
}

[[nodiscard]] inline u64 currentSecondsSinceEpoch()
{
    const auto currentTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
        currentTime.time_since_epoch()).count();
}

