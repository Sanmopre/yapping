#include "tcp_client.h"
#include "messages.h"
#include "cmake_constants.h"

// cli11
#include "CLI/CLI.hpp"

// std
#include <iostream>

int main(int argc, char **argv)
{
    CLI::App clientApplication(CLIENT_TARGET_NAME);
    clientApplication.set_version_flag("--version", PROJECT_VERSION);

CLI11_PARSE(clientApplication, argc, argv);

  const std::string logFile = std::string(CLIENT_TARGET_NAME).append(getTimeStamp(currentSecondsSinceEpoch())).append(".log");
  const auto logger = getLogger(CLIENT_TARGET_NAME, logFile);
  logger->info("Starting {} version {}", CLIENT_TARGET_NAME, PROJECT_VERSION);

  SimpleTcpClient cli;

  cli.on_connect([&cli, &logger]
  {
      client::messages::InitialConnection msg;
      msg.username = "testUser";
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

  cli.connect("127.0.0.1", 9000);

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