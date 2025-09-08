#include "tcp_server.h"
#include "cmake_constants.h"

// cli11
#include "CLI/CLI.hpp"

// std
#include <iostream>


int main(int argc, char **argv)
{
    CLI::App serverApplication(SERVER_DESCRIPTION);
    serverApplication.set_version_flag("--version", PROJECT_VERSION);

    std::string loggingFolder = "./logs";
    u16 port;

    serverApplication.add_option("-l,--log-folder", loggingFolder, "Path tp the folder where the logs from the application will be generated.")
       ->check(CLI::ExistingDirectory);

    serverApplication.add_option("-p,--port", port, "Server port for incoming connections")
       ->required()
       ->check(CLI::Range(1, 65535));

    CLI11_PARSE(serverApplication, argc, argv);

    const auto timeStampStringForFile = makeSafeForFilename(getTimeStamp(currentSecondsSinceEpoch()));
    const std::string logFile = std::string(SERVER_TARGET_NAME) + "_" + timeStampStringForFile + ".log";

    const auto logger = getLogger(
        SERVER_TARGET_NAME,
        (std::filesystem::path(loggingFolder) / logFile).string()
    );

    logger->info("Starting {} version {}", SERVER_TARGET_NAME, PROJECT_VERSION);

    SimpleTcpServerMulti srv(port);

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
        std::visit(overloaded{
    [&srv, id, &logger](const client::messages::Disconnect& v)
            {
                logger->info("Disconnected message client with id {}", id);

                server::messages::UserDisconnected disconnect;
                disconnect.reason = v.reason;
                disconnect.timestamp = currentSecondsSinceEpoch();

                if (const auto username = srv.getUsername(id); username.has_value())
                {
                    disconnect.username = username.value();
                }
                else
                {
                    logger->error("Invalid connection id {}", id);
                }

                srv.broadcast(disconnect);
            },
            [&srv, id, &logger](const client::messages::NewMessage& v)
            {
                logger->info("User {} said {}", id, v.message);

                server::messages::NewMessageReceived received;
                received.timestamp = currentSecondsSinceEpoch();
                received.message = v.message;

                if (const auto username = srv.getUsername(id); username.has_value())
                {
                    received.username = username.value();
                }
                else
                {
                    logger->error("Invalid connection id {}", id);
                }

                srv.broadcast(received);
            },
            [&srv, id, &logger](const client::messages::InitialConnection& v)
            {
                logger->info("New user connected message id {} with username {}", id, v.username);

                server::messages::UserConnected connect;
                connect.timestamp = currentSecondsSinceEpoch();
                connect.username = v.username;

                srv.addNewUsername(id, v.username);
                srv.broadcast(connect);
            }
    }, msg);
    });

    srv.start();

    spdlog::info("Server started on port {}", port);
    std::cin.get();
    srv.stop();
}
