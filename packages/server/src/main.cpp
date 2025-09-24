#include "tcp_server.h"
#include "cmake_constants.h"

// cli11
#include "CLI/CLI.hpp"

#include "db_manager.h"

// std
#include <random>
#include <cstddef>


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

[[nodiscard]] server::messages::UserColor getRandomColor() {
    static_assert(!colors.empty(), "colors array must not be empty");
    static thread_local std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<std::size_t> dist(0, colors.size() - 1);
    return colors[dist(rng)];
}

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

    std::map<std::string, UserData> currentUsers;


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
            status.color = currentUsers[status.username].color;
            currentUsers[status.username].status = UserStatusType::OFFLINE;
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

                // Add user to users map
                if (const auto it = currentUsers.find(v.username); it == currentUsers.end())
                {
                    // If its the first time a user is registered, we assign a random color
                    currentUsers[v.username].color =  getRandomColor();
                }
                currentUsers[v.username].status = UserStatusType::ONLINE;

                server::messages::UserStatus status;
                status.timestamp = currentSecondsSinceEpoch();
                status.status = currentUsers.at(v.username).status;
                status.username = v.username;
                status.color = currentUsers[status.username].color;


                const auto previousMessages = dbManager.getMessages();
                for (const auto& previousMessage : previousMessages)
                {
                    srv.write(id, previousMessage);
                }

                for (const auto& [username, data] : currentUsers)
                {
                    server::messages::UserStatus currentUserStatus;
                    currentUserStatus.username = username;
                    currentUserStatus.status = data.status;
                    currentUserStatus.color = data.color;
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
