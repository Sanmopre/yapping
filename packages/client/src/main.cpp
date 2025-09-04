#include "tcp_client.h"
#include <iostream>

int main() {
  SimpleTcpClient cli;

  cli.on_connect([]{
      std::cout << "Connected!\n";
  });

  cli.on_disconnect([]{
      std::cout << "Disconnected.\n";
  });

  cli.on_message([&](const std::string& msg){
      std::cout << "RX: " << msg << "\n";
  });

  cli.connect("127.0.0.1", 9000); // connect to your server

  std::cout << "Type lines to send. Ctrl+C to quit.\n";
  std::string line;
  while (std::getline(std::cin, line)) {
    cli.write(line);
  }

  cli.stop();
}