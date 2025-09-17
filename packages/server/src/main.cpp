#include "tcp_server.h"
#include "cmake_constants.h"

// cli11
#include "CLI/CLI.hpp"

#include "db_manager.h"


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

    DataBaseManager dbManager{logger.get()};

    std::map<std::string, UserStatusType> currentUsers;


    SimpleTcpServerMulti srv(port);

    srv.on_connect([&](u64 id)
    {
        logger->info("Connected client with id {}", id);
    });

    srv.on_disconnect([&](u64 id)
    {
        logger->info("Disconnected client with id {}", id);
        server::messages::UserStatus status;
        status.timestamp = currentSecondsSinceEpoch();
        status.status = UserStatusType::OFFLINE;

        if (const auto username = srv.getUsername(id); username.has_value())
        {
            status.username = username.value();
            currentUsers[status.username] = UserStatusType::OFFLINE;
        }
        else
        {
            logger->error("Invalid connection id {}", id);
        }

        srv.broadcast(status);
    });

    srv.on_message([&](u64 id, const client::messages::ClientMessage& msg)
    {
        std::visit(overloaded{
            [&srv, id, &logger, &dbManager](const client::messages::NewMessage& v)
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

                dbManager.addMessageEntry(received);
                srv.broadcast(received);
            },
            [&](const client::messages::InitialConnection& v)
            {
                logger->info("New user connected message id {} with username {}", id, v.username);

                server::messages::UserStatus status;
                status.timestamp = currentSecondsSinceEpoch();
                status.status = UserStatusType::ONLINE;
                status.username = v.username;

                // Add user to users map
                currentUsers[status.username] = UserStatusType::ONLINE;

                const auto previousMessages = dbManager.getMessages();
                for (const auto& previousMessage : previousMessages)
                {
                    srv.write(id, previousMessage);
                }

                for (const auto& [username, status] : currentUsers)
                {
                    server::messages::UserStatus currentUserStatus;
                    currentUserStatus.username = username;
                    currentUserStatus.status = status;
                    currentUserStatus.timestamp = currentSecondsSinceEpoch();

                    srv.write(id, currentUserStatus);
                }

                srv.addNewUsername(id, v.username);
                srv.broadcast(status);
            }
    }, msg);
    });

    srv.start();

    spdlog::info("Server started on port {}", port);
    std::cin.get();
    srv.stop();
}
