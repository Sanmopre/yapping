#include "tcp_client.h"
#include "messages.h"

// std
#include <iostream>


int main()
{
  SimpleTcpClient cli;

  cli.on_connect([&cli]
  {
      client::messages::InitialConnection msg;
      msg.username = "testUser";
      cli.write(msg);
      std::cout << "Connected!\n";
  });

  cli.on_disconnect([&cli]
  {
      client::messages::Disconnect msg;
      msg.reason = "Just left";
      cli.write(msg);
      std::cout << "Disconnected.\n";
  });

  cli.on_message([&](const server::messages::ServerMessage&& msg)
  {
      std::visit(overloaded{
  [](const server::messages::UserConnected& v) {
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

  cli.connect("127.0.0.1", 9000); // connect to your server

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