#pragma once

#include "messages.h"

// std
#include <random>
#include <cstddef>

[[nodiscard]] inline server::messages::UserColor getRandomColor()
{
    constexpr auto red        = server::messages::UserColor(200, 10, 10);
    constexpr auto green      = server::messages::UserColor(20, 170, 20);
    constexpr auto blue       = server::messages::UserColor(20, 60, 200);
    constexpr auto orange     = server::messages::UserColor(230, 120, 20);
    constexpr auto yellow     = server::messages::UserColor(230, 210, 20);
    constexpr auto purple     = server::messages::UserColor(150, 50, 200);
    constexpr auto cyan       = server::messages::UserColor(20, 200, 200);
    constexpr auto magenta    = server::messages::UserColor(200, 20, 140);
    constexpr auto teal       = server::messages::UserColor(10, 140, 140);
    constexpr auto lime       = server::messages::UserColor(150, 230, 30);
    constexpr auto pink       = server::messages::UserColor(230, 110, 170);
    constexpr auto brown      = server::messages::UserColor(120, 75, 40);
    constexpr auto navy       = server::messages::UserColor(20, 30, 80);
    constexpr auto olive      = server::messages::UserColor(110, 120, 30);
    constexpr auto maroon     = server::messages::UserColor(120, 20, 40);
    constexpr auto gold       = server::messages::UserColor(220, 180, 20);
    constexpr auto sky        = server::messages::UserColor(90, 170, 230);
    constexpr auto violet     = server::messages::UserColor(170, 70, 230);
    constexpr auto coral      = server::messages::UserColor(240, 100, 90);
    constexpr auto indigo     = server::messages::UserColor(75, 0, 130);

    constexpr std::array<server::messages::UserColor, 20> colors{
        red, green, blue, orange, yellow,
        purple, cyan, magenta, teal, lime,
        pink, brown, navy, olive, maroon,
        gold, sky, violet, coral, indigo
    };


    static_assert(!colors.empty(), "colors array must not be empty");
    static thread_local std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<std::size_t> dist(0, colors.size() - 1);
    return colors[dist(rng)];
}