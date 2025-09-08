#include "tcp_client.h"
#include "messages.h"
#include "cmake_constants.h"

// cli11
#include "CLI/CLI.hpp"

// std
#include <iostream>

int main(int argc, char **argv)
{
    CLI::App clientApplication(CLIENT_DESCRIPTION);
    clientApplication.set_version_flag("--version", PROJECT_VERSION);

    // Arguments for the client
    std::string username = "random";
    std::string serverIp;
    std::string loggingFolder = "./logs";
    u16 serverPort;

    clientApplication.add_option("-u,--username", username,
        "Username for the client to use when connecting");

    clientApplication.add_option("-i,--ip", serverIp,
        "IPv4 address of the server to connect to")
        ->required()
        ->check(CLI::ValidIPV4);

    clientApplication.add_option("-p,--port", serverPort,
        "Port of the server to connect to")
        ->required()
        ->check(CLI::Range(1, 65535));

    clientApplication.add_option("-l,--log-folder", loggingFolder, "Path tp the folder where the logs from the application will be generated.")
   ->check(CLI::ExistingDirectory);


    CLI11_PARSE(clientApplication, argc, argv);

    const auto timeStampStringForFile = makeSafeForFilename(getTimeStamp(currentSecondsSinceEpoch()));
    const std::string logFile = std::string(CLIENT_TARGET_NAME) + "_" + timeStampStringForFile + ".log";

    const auto logger = getLogger(
        CLIENT_TARGET_NAME,
        (std::filesystem::path(loggingFolder) / logFile).string()
    );

  logger->info("Starting {} version {}", CLIENT_TARGET_NAME, PROJECT_VERSION);

  SimpleTcpClient cli;

  cli.on_connect([&cli, &logger, &username]
  {
      client::messages::InitialConnection msg;
      msg.username = username;
      cli.write(msg);
      logger->info("Connected to server");
  });

  cli.on_disconnect([&cli, &logger]
  {
      client::messages::Disconnect msg;
      msg.reason = "Just left";
      cli.write(msg);
      logger->info("Disconnected from server");
  });

  cli.on_message([&cli , &logger](const server::messages::ServerMessage&& msg)
  {
      std::visit(overloaded{
  [](const server::messages::UserConnected& v)
  {
      std::cout << "Connected = " << v.username << "\n";
  },
  [](const server::messages::UserDisconnected& v) {
      std::cout << "User disconnected: = " << v.reason << "\n";
  },
  [](const server::messages::NewMessageReceived& v) {
      std::cout << "Message = " << v.username << "\n";
  }
      }, msg);
  });

  cli.connect(serverIp, serverPort);

  std::cout << "Type lines to send. Ctrl+C to quit.\n";
  std::string line;
  while (std::getline(std::cin, line))
  {
    client::messages::NewMessage newMessage;
      newMessage.message = line;
    cli.write(newMessage);
  }

  cli.stop();
}