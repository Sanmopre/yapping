#include "tcp_server.h"

// std
#include <iostream>

int main() {
    SimpleTcpServerMulti srv(9000);

    srv.on_connect([](u64 id){
        std::cout << "Client +" << id << "\n";
    });

    srv.on_disconnect([](u64 id){
        std::cout << "Client -" << id << "\n";
    });

    srv.on_message([&](u64 id, const std::string& line)
    {
        std::cout << "[" << id << "] " << line << "\n";
        //srv.write(id, "Echo: " + line);       // reply to just that client
        srv.broadcast("All: " + line);     // or blast to everyone
    });

    srv.start();

    std::cout << "Server on :9000. Press Enter to quit.\n";
    std::cin.get();
    srv.stop();
}
