#include "tcp_server.h"

// std
#include <iostream>

int main()
{
    const auto logger = getLogger("server", "logs/cli_chat.log");
    logger->info("Starting server");

    SimpleTcpServerMulti srv(9000);

    srv.on_connect([](u64 id)
    {
        std::cout << "Client +" << id << "\n";
    });

    srv.on_disconnect([](u64 id)
    {
        std::cout << "Client -" << id << "\n";
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
