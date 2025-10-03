#pragma once

// std
#include <cstdint>
#include <string>

// spdlog
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

// overloaded
template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

[[nodiscard]] inline std::shared_ptr<spdlog::logger> getLogger(const std::string &name, const std::string &path)
{
    const auto logger = spdlog::basic_logger_mt(name, path);
    spdlog::set_default_logger(logger);
    logger->flush_on(spdlog::level::debug);
    return logger;
}

// types
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

constexpr u16 MAX_MESSAGE_LENGTH = 256;

// Content keys
constexpr std::string_view USERNAME_KEY = "username";
constexpr std::string_view TIMESTAMP_KEY = "timestamp";
constexpr std::string_view MESSAGE_KEY = "message";
constexpr std::string_view REASON_KEY = "reason";
constexpr std::string_view USER_STATUS_KEY = "status";
constexpr std::string_view USER_COLOR_KEY = "color";
constexpr std::string_view COLOR_RED_KEY = "red";
constexpr std::string_view COLOR_GREEN_KEY = "green";
constexpr std::string_view COLOR_BLUE_KEY = "blue";
constexpr std::string_view SERVER_RESPONSE_CODE_KEY = "responseCode";
constexpr std::string_view PASSWORD_HASH_KEY = "passwordHash";

// Packet keys
constexpr std::string_view PACKET_HEADER_KEY = "header";
constexpr std::string_view PACKET_CONTENT_KEY = "content";

enum class ServerResponseCode
{
    SUCCESSFUL_REGISTRATION,
    SUCCESSFUL_LOGIN,
    USERNAME_ALREADY_EXISTS,
    INCORRECT_PASSWORD,
};

enum class UserStatusType
{
    ONLINE,
    AWAY,
    OFFLINE
};

enum class ServerMessageType
{
    RECEIVED_MESSAGE = 0,
    USER_STATUS = 1,
    SERVER_RESPONSE = 2
};

enum class ClientMessageType
{
    INITIAL_CONNECTION = 0,
    NEW_MESSAGE = 2,
    REGISTER = 3,
    LOGIN = 4
};

// FNV-1a (64-bit) implementation
[[nodiscard]] inline u64 hashImpl(const std::string& string)
{
    u64 hash = 0xcbf29ce484222325ULL;
    for (const unsigned char c : string)
    {
        hash ^= c;
        hash *= 0x100000001b3ULL;
    }
    return hash;
}
