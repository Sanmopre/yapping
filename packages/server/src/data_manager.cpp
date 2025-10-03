#include "data_manager.h"
#include "utils.h"

namespace server
{

DataManager::DataManager(DataBaseManager* dbManager, spdlog::logger* logger)
    : logger_(logger), dbManager_(dbManager)
{
}

DataManager::~DataManager()
{
}

void DataManager::connect(const std::string& ip, u16 port)
{
    tcpServer_ = std::make_unique<TcpServerMulti>(port);
    // TODO: use ip to select an interface
    std::ignore = ip;

    // callbacks
    tcpServer_->on_connect([this](u64 id){onConnect(id);});
    tcpServer_->on_disconnect([this](u64 id){onDisconnect(id);});
    tcpServer_->on_message([&](u64 id, const client::messages::ClientMessage& msg)
    {
        std::visit(overloaded{
            [id, this](const auto& value)
            {
                manageMessageContent(id, value);
            }
    }, msg);
    });

    // start server
    tcpServer_->start();
}

void DataManager::manageMessageContent(u64 id, const client::messages::Login& value)
{
}

void DataManager::manageMessageContent(u64 id, const client::messages::Register& value)
{
}

void DataManager::manageMessageContent(u64 id, const client::messages::InitialConnection& value)
{
    logger_->info("New user connected message id {} with username {}", id, value.username);

    // Add user to users map
    if (const auto it = currentUsers_.find(value.username); it == currentUsers_.end())
    {
        // If its the first time a user is registered, we assign a random color
        currentUsers_[value.username].color =  getRandomColor();
    }
    currentUsers_[value.username].status = UserStatusType::ONLINE;

    server::messages::UserStatus status;
    status.timestamp = currentSecondsSinceEpoch();
    status.status = currentUsers_.at(value.username).status;
    status.username = value.username;
    status.color = currentUsers_[status.username].color;


    const auto previousMessages = dbManager_->getMessages();
    for (const auto& previousMessage : previousMessages)
    {
        tcpServer_->write(id, previousMessage);
    }

    for (const auto& [username, data] : currentUsers_)
    {
        server::messages::UserStatus currentUserStatus;
        currentUserStatus.username = username;
        currentUserStatus.status = data.status;
        currentUserStatus.color = data.color;
        currentUserStatus.timestamp = currentSecondsSinceEpoch();

        tcpServer_->write(id, currentUserStatus);
    }

    tcpServer_->addNewUsername(id, value.username);
    tcpServer_->broadcast(status);
}

void DataManager::manageMessageContent(u64 id, const client::messages::NewMessage& value)
{
    logger_->info("User {} said {}", id, value.message);

    server::messages::NewMessageReceived received;
    received.timestamp = currentSecondsSinceEpoch();
    received.message = value.message;

    if (const auto username = tcpServer_->getUsername(id); username.has_value())
    {
        received.username = username.value();
    }
    else
    {
        logger_->error("Invalid connection id {}", id);
    }

    dbManager_->addMessageEntry(received);
    tcpServer_->broadcast(received);
}

void DataManager::onConnect(u64 id)
{
    logger_->info("Connected client with id {}", id);
}

void DataManager::onDisconnect(u64 id)
{
    logger_->info("Disconnected client with id {}", id);
    server::messages::UserStatus status;
    status.timestamp = currentSecondsSinceEpoch();
    status.status = UserStatusType::OFFLINE;

    if (const auto username = tcpServer_->getUsername(id); username.has_value())
    {
        status.username = username.value();
        status.color = currentUsers_[status.username].color;
        currentUsers_[status.username].status = UserStatusType::OFFLINE;
    }
    else
    {
        logger_->error("Invalid connection id {}", id);
    }

    tcpServer_->broadcast(status);
}

}
