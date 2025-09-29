#include "data_manager.h"

DataManager::DataManager(const std::string &username, const std::string &host,
                         u16 port, spdlog::logger *logger) :
 logger_(logger), tcpClient_(std::make_unique<TcpClient>(logger)), username(username)
{
  tcpClient_->on_connect([this]{onConnect();});
  tcpClient_->on_disconnect([this]{onDisconnect();});
  tcpClient_->on_message([this](const server::messages::ServerMessage &&msg){onMessage(msg);});

  tcpClient_->connect(host, port);
}

DataManager::~DataManager()
{
  tcpClient_->stop();
}

bool DataManager::sendMessage(const std::string &message) const noexcept
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

std::string DataManager::getUsername() const noexcept
{
  return username;
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

void DataManager::onConnect()
{
  client::messages::InitialConnection msg;
  msg.username = username;
  tcpClient_->write(msg);
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
  std::ignore = value;
}