// src/main.cpp
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io;
    asio::steady_timer t(io, std::chrono::seconds(1));
    t.async_wait([](const std::error_code&){ std::cout << "tick\n"; });
    io.run();
}