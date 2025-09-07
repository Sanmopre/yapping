#include "tcp_server.h"
#include "cmake_constants.h"

// cli11
#include "CLI/CLI.hpp"

// std
#include <iostream>

int main(int argc, char **argv)
{
    CLI::App serverApplication(SERVER_TARGET_NAME);
    serverApplication.set_version_flag("--version", PROJECT_VERSION);

    CLI11_PARSE(serverApplication, argc, argv);

    const std::string logFile = std::string(SERVER_TARGET_NAME).append(getTimeStamp(currentSecondsSinceEpoch())).append(".log");
    const auto logger = getLogger(SERVER_TARGET_NAME, logFile);
    logger->info("Starting {} version {}", SERVER_TARGET_NAME, PROJECT_VERSION);

    SimpleTcpServerMulti srv(9000);

    srv.on_connect([&logger](u64 id)
    {
        logger->info("Connected client with id {}", id);
    });

    srv.on_disconnect([&logger](u64 id)
    {
        logger->info("Disconnected client with id {}", id);
    });

    srv.on_message([&](u64 id, const client::messages::ClientMessage& msg)
    {
        std::cout << "[" << id << "]  message index [" << msg.index() << "]\n";

        std::visit(overloaded{
    [](const client::messages::Disconnect& v) {
        std::cout << "Disconnect, reason = " << v.reason << "\n";
    },
    [](const client::messages::NewMessage& v) {
        std::cout << "NewMessage, text = " << v.message << "\n";
    },
    [](const client::messages::InitialConnection& v) {
        std::cout << "InitialConnection, username = " << v.username << "\n";
    }
        }, msg);




        server::messages::NewMessageReceived newMsg;

        newMsg.message = "new message";
        newMsg.timestamp = currentSecondsSinceEpoch();
        newMsg.username = "server";

        srv.broadcast(newMsg);
    });

    srv.start();

    std::cout << "Server on :9000. Press Enter to quit.\n";
    std::cin.get();
    srv.stop();
}
