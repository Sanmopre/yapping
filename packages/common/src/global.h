#pragma once

// std
#include <cstdint>
#include <string>

// overloaded
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

// Content keys
constexpr std::string_view USERNAME_KEY = "username";
constexpr std::string_view TIMESTAMP_KEY = "timestamp";
constexpr std::string_view MESSAGE_KEY = "message";
constexpr std::string_view REASON_KEY = "reason";

// Packet keys
constexpr std::string_view PACKET_HEADER_KEY = "header";
constexpr std::string_view PACKET_CONTENT_KEY = "content";


enum class ServerMessageType
{
    RECEIVED_MESSAGE = 0,
    USER_DISCONNECTED = 1,
    USER_CONNECTED = 2
};

enum class ClientMessageType
{
    INITIAL_CONNECTION = 0,
    DISCONNECTED = 1,
    NEW_MESSAGE = 2
};