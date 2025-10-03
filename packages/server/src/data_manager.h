#pragma once

#include "db_manager.h"
#include "tcp_server.h"

namespace server
{

class DataManager
{
public:
    DataManager(DataBaseManager* dbManager, spdlog::logger* logger);
    ~DataManager();

public:
    void connect(const std::string& ip, u16 port);

public:
    void manageMessageContent(u64 id, const client::messages::Login &value);
    void manageMessageContent(u64 id, const client::messages::Register &value);
    void manageMessageContent(u64 id, const client::messages::InitialConnection &value);
    void manageMessageContent(u64 id, const client::messages::NewMessage &value);

private:
    void onConnect(u64 id);
    void onDisconnect(u64 id);

private:
    spdlog::logger* logger_;
    DataBaseManager* dbManager_;
    std::unique_ptr<TcpServerMulti> tcpServer_;

private:
    // data containers
    std::map<std::string, UserData> currentUsers_;
};

}