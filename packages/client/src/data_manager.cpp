#include "data_manager.h"

DataManager::DataManager(spdlog::logger *logger) :
 logger_(logger), tcpClient_(std::make_unique<TcpClient>(logger))
{
  tcpClient_->on_connect([this]{onConnect();});
  tcpClient_->on_disconnect([this]{onDisconnect();});
  tcpClient_->on_message([this](const server::messages::ServerMessage &&msg){onMessage(msg);});
}

DataManager::~DataManager()
{
  tcpClient_->stop();
}

bool DataManager::sendChatMessageContent(const std::string &message) const noexcept
{
  if (message.empty())
  {
    return false;
  }

  client::messages::NewMessage newMessage;
  newMessage.message = message;
  tcpClient_->write(newMessage);

  return true;
}
void DataManager::sendClientMessage(
    const client::messages::ClientMessage &clientMessage) const noexcept {
  tcpClient_->write(clientMessage);
}


void DataManager::setUsername(const std::string &username) const noexcept
{
  username_ = username;
}

std::string DataManager::getUsername() const noexcept {
  return username_;
}

std::vector<server::messages::NewMessageReceived>
DataManager::getMessages() const noexcept
{
  return messages_;
}

std::map<std::string, UserData> DataManager::getUsers() const noexcept
{
  return usersMap_;
}

ServerResponseCode DataManager::getServerResponseCode() const noexcept
{
  return latestServerResponse_;
}

void DataManager::connect(const std::string &host, u16 port) const noexcept
{
  tcpClient_->connect(host, port);
}

void DataManager::onConnect()
{
  logger_->info("Connected");
}

void DataManager::onDisconnect()
{
  logger_->info("Disconnected");
}

void DataManager::onMessage(const server::messages::ServerMessage &message)
{
  std::visit(
      overloaded{[&](const auto &value)
      {
        manageMessageContent(value);
      }},
      message);
}

void DataManager::manageMessageContent(
    const server::messages::NewMessageReceived &value)
{
  if (const auto it = usersMap_.find(value.username); it == usersMap_.end())
  {
    logger_->error("User {} not found in users map, can not display message with color", value.username);
  }

  messages_.emplace_back(value);
}

void DataManager::manageMessageContent(
    const server::messages::UserStatus &value)
{
  usersMap_[value.username].status = value.status;
  usersMap_[value.username].color = value.color;
}

void DataManager::manageMessageContent(
    const server::messages::ServerResponse &value)
{
  latestServerResponse_ = value.code;
}