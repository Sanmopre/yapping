#pragma once

#include "messages.h"

// asio
#include "asio.hpp"

// std
#include <atomic>
#include <deque>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

class SimpleTcpClient
{
  public:
    SimpleTcpClient() : socket_(io_)
    {
    }

    ~SimpleTcpClient()
    {
        stop();
    }

    // Connect to host:port (IPv4 or hostname). Starts internal IO thread.
    void connect(const std::string &host, u16 port)
    {
        if (running_.exchange(true))
            return;
        host_ = host;
        port_ = port;

        io_thread_ = std::thread([this] {
            do_connect();
            io_.run();
        });
    }

    // Close the connection and stop IO.
    void stop()
    {
        if (!running_.exchange(false))
            return;

        asio::post(io_, [this] {
            std::error_code ec;
            socket_.close(ec);
            resolver_.cancel();
        });

        if (io_thread_.joinable())
            io_thread_.join();
        io_.restart();
        connected_ = false;
        writing_ = false;
        write_queue_.clear();
    }

    // Send a line of text to the server. Appends '\n' if not present.
    void write(client::messages::ClientMessage clientMsg)
    {
        std::string msg = std::visit([](auto const &m) { return m.toString(); }, clientMsg);

        if (msg.empty() || msg.back() != '\n')
            msg.push_back('\n');
        asio::post(io_, [this, m = std::move(msg)]() mutable {
            if (!connected_ || !socket_.is_open())
                return;
            write_queue_.push_back(std::move(m));
            if (!writing_)
                do_write_next();
        });
    }

    // Callbacks
    template <typename Handler> void on_message(Handler &&h)
    {
        message_handler_ = std::forward<Handler>(h);
    }

    template <typename Handler> void on_connect(Handler &&h)
    {
        on_connect_ = std::forward<Handler>(h);
    }

    template <typename Handler> void on_disconnect(Handler &&h)
    {
        on_disconnect_ = std::forward<Handler>(h);
    }

  private:
    void do_connect()
    {
        resolver_ = asio::ip::tcp::resolver(io_);

        // Resolve host:port (IPv4 preferred)
        resolver_.async_resolve(
            host_, std::to_string(port_), [this](std::error_code ec, asio::ip::tcp::resolver::results_type results) {
                if (ec)
                {
                    std::cerr << "Resolve error: " << ec.message() << "\n";
                    handle_disconnect(ec);
                    return;
                }

                // Filter to IPv4 endpoints first
                std::vector<asio::ip::tcp::endpoint> v4;
                for (auto &r : results)
                {
                    if (r.endpoint().address().is_v4())
                        v4.push_back(r.endpoint());
                }
                auto begin = v4.empty() ? results.begin() : asio::ip::tcp::resolver::results_type::iterator();
                if (!v4.empty())
                {
                    // If we built a v4 vector, connect to the first v4 endpoint
                    socket_.async_connect(v4.front(), [this](std::error_code ec2) {
                        if (ec2)
                        {
                            handle_disconnect(ec2);
                            return;
                        }
                        finish_connected();
                    });
                }
                else
                {
                    // Fall back to whatever resolve returned
                    asio::async_connect(socket_, results, [this](std::error_code ec2, const asio::ip::tcp::endpoint &) {
                        if (ec2)
                        {
                            handle_disconnect(ec2);
                            return;
                        }
                        finish_connected();
                    });
                }
            });
    }

    void finish_connected()
    {
        connected_ = true;
        if (on_connect_)
            on_connect_();
        do_read_loop();
    }

    void do_read_loop()
    {
        // Read newline-terminated frames
        asio::async_read_until(socket_, read_buf_, '\n', [this](std::error_code ec, std::size_t) {
            if (ec)
            {
                handle_disconnect(ec);
                return;
            }

            std::istream is(&read_buf_);
            std::string line;
            std::getline(is, line);

            if (message_handler_)
            {
                const nlohmann::json data = nlohmann::json::parse(line, nullptr, false);
                if (data.is_discarded())
                {
                    std::cerr << "Invalid json for message : " << line << "\n";
                }

                const auto &content = data[PACKET_CONTENT_KEY];
                switch (data[PACKET_HEADER_KEY].get<ServerMessageType>())
                {
                case ServerMessageType::RECEIVED_MESSAGE:
                    message_handler_(server::messages::NewMessageReceived{content});
                    break;
                case ServerMessageType::USER_STATUS:
                    message_handler_(server::messages::UserStatus{content});
                    break;
                }
            }

            if (connected_)
            {
                do_read_loop();
            }
        });
    }

    void do_write_next()
    {
        if (write_queue_.empty() || !connected_)
        {
            writing_ = false;
            return;
        }
        writing_ = true;
        auto &front = write_queue_.front();
        asio::async_write(socket_, asio::buffer(front), [this](std::error_code ec, std::size_t) {
            if (ec)
            {
                handle_disconnect(ec);
                return;
            }
            write_queue_.pop_front();
            do_write_next();
        });
    }

    void handle_disconnect(const std::error_code &ec)
    {
        if (connected_)
        {
            if (ec != asio::error::operation_aborted)
            {
                std::cerr << "Disconnected: " << ec.message() << "\n";
            }
            std::error_code ignore;
            socket_.close(ignore);
            connected_ = false;
            writing_ = false;
            write_queue_.clear();
            if (on_disconnect_)
                on_disconnect_();
        }
        // No auto-reconnect (keep it simple). Call connect() again if desired.
    }

  private:
    // IO
    asio::io_context io_;
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::resolver resolver_{io_};
    std::thread io_thread_;
    std::atomic<bool> running_{false};
    bool connected_{false};

    // Read
    asio::streambuf read_buf_;

    // Write queue
    std::deque<std::string> write_queue_;
    bool writing_{false};

    // Callbacks
    std::function<void()> on_connect_;
    std::function<void()> on_disconnect_;
    std::function<void(const server::messages::ServerMessage &&)> message_handler_;

    // Target
    std::string host_;
    u16 port_{0};
};