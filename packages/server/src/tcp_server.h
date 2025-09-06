#pragma once

#ifndef ASIO_STANDALONE
#  define ASIO_STANDALONE
#endif

#include "global.h"
#include "messages.h"

// asio
#include "asio.hpp"

// std
#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

class SimpleTcpServerMulti {
public:
    explicit SimpleTcpServerMulti(u16 port,
                                  const asio::ip::address& addr = asio::ip::address_v4::any())
        : io_(),
          acceptor_(io_, asio::ip::tcp::endpoint(addr, port)) {}

    ~SimpleTcpServerMulti() { stop(); }

    // Start the server (spawns the io thread)
    void start() {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true)) return;
        do_accept();
        io_thread_ = std::thread([this]{ io_.run(); });
    }

    // Stop accepting and close all connections
    void stop() {
        bool expected = true;
        if (!running_.compare_exchange_strong(expected, false)) return;
        asio::post(io_, [this]{
            std::error_code ec;
            acceptor_.close(ec);
            for (auto& [id, c] : conns_) {
                if (c && c->socket.is_open()) c->socket.close(ec);
            }
        });
        if (io_thread_.joinable()) io_thread_.join();
        io_.restart();
        conns_.clear();
        next_id_ = 1;
    }

    // Send a line to a specific client. Appends \n if absent.
    void write(u64 client_id, server::messages::ServerMessage serverMsg)
    {
        std::string msg = std::visit([](auto const& m)
        {
            return m.toString();
        }, serverMsg);

        if (msg.empty() || msg.back() != '\n') msg.push_back('\n');
        asio::post(io_, [this, client_id, m = std::move(msg)]() mutable {
            auto it = conns_.find(client_id);
            if (it == conns_.end() || !it->second) return;
            auto& conn = *it->second;
            if (!conn.socket.is_open()) return;
            conn.outbox.push_back(std::move(m));
            if (!conn.writing) do_write_next(it->second);
        });
    }

    // Broadcast a line to all connected clients
    void broadcast(server::messages::ServerMessage serverMsg)
    {
        std::string msg = std::visit([](auto const& m)
        {
            return m.toString();
        }, serverMsg);

        if (msg.empty() || msg.back() != '\n') msg.push_back('\n');
        asio::post(io_, [this, m = std::move(msg)]() mutable {
            for (auto& [id, c] : conns_) {
                if (!c || !c->socket.is_open()) continue;
                c->outbox.push_back(m);
                if (!c->writing) do_write_next(c);
            }
        });
    }

    // Callbacks
    template <typename H>
    void on_message(H&& h) { on_message_ = std::forward<H>(h); }

    template <typename H>
    void on_connect(H&& h) { on_connect_ = std::forward<H>(h); }

    template <typename H>
    void on_disconnect(H&& h) { on_disconnect_ = std::forward<H>(h); }

private:
    struct Conn : std::enable_shared_from_this<Conn> {
        explicit Conn(asio::io_context& io, u64 id)
            : socket(io), id(id) {}
        asio::ip::tcp::socket socket;
        u64 id;
        asio::streambuf read_buf;
        std::deque<std::string> outbox;
        bool writing{false};
    };

    void do_accept() {
        // Keep a socket alive for the duration of accept
        auto sock = std::make_shared<asio::ip::tcp::socket>(io_);
        acceptor_.async_accept(*sock, [this, sock](std::error_code ec){
            if (ec) {
                if (running_) {
                    // Try accepting again
                    asio::post(io_, [this]{ if (running_) do_accept(); });
                }
                return;
            }
            if (!running_) return;

            u64 id = next_id_++;
            auto c = std::make_shared<Conn>(io_, id);
            c->socket = std::move(*sock);
            conns_.emplace(id, c);
            if (on_connect_) on_connect_(id);

            do_read_loop(c);

            // Continue accepting more connections
            if (running_) do_accept();
        });
    }

    void do_read_loop(const std::shared_ptr<Conn>& c) {
        asio::async_read_until(c->socket, c->read_buf, '\n',
            [this, self=c](std::error_code ec, std::size_t){
                if (ec) { handle_disconnect(self, ec); return; }
                std::istream is(&self->read_buf);
                std::string line;
                std::getline(is, line); // strips '\n'

                if (on_message_)
                {
                    const nlohmann::json data = nlohmann::json::parse(line, nullptr, false);
                    if (data.is_discarded())
                    {
                        std::cerr << "Invalid json for message : " << line <<"\n";
                    }

                    const auto& content = data[PACKET_CONTENT_KEY];
                    switch (data[PACKET_HEADER_KEY].get<ClientMessageType>())
                    {
                    case ClientMessageType::DISCONNECTED:
                        on_message_(self->id, client::messages::Disconnect{content});
                        break;
                    case ClientMessageType::INITIAL_CONNECTION:
                        on_message_(self->id, client::messages::InitialConnection{content});
                        break;
                    case ClientMessageType::NEW_MESSAGE:
                        on_message_(self->id, client::messages::NewMessage{content});
                        break;
                    }
                }

                if (self->socket.is_open())
                {
                    do_read_loop(self);
                }
            });
    }

    void do_write_next(const std::shared_ptr<Conn>& c) {
        if (c->outbox.empty() || !c->socket.is_open()) { c->writing = false; return; }
        c->writing = true;
        auto& front = c->outbox.front();
        asio::async_write(c->socket, asio::buffer(front),
            [this, self=c](std::error_code ec, std::size_t){
                if (ec) { handle_disconnect(self, ec); return; }
                self->outbox.pop_front();
                do_write_next(self);
            });
    }

    void handle_disconnect(const std::shared_ptr<Conn>& c, const std::error_code& ec) {
        if (!c) return;
        if (c->socket.is_open()) {
            std::error_code ignore;
            c->socket.close(ignore);
        }
        if (on_disconnect_) on_disconnect_(c->id);
        conns_.erase(c->id);
    }

private:
    asio::io_context io_;
    asio::ip::tcp::acceptor acceptor_;
    std::thread io_thread_;
    std::atomic<bool> running_{false};

    std::unordered_map<u64, std::shared_ptr<Conn>> conns_;
    u64 next_id_{1};

    // Callbacks
    std::function<void(u64)> on_connect_;
    std::function<void(u64)> on_disconnect_;
    std::function<void(u64, const client::messages::ClientMessage&)> on_message_;
};
