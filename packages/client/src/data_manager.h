#pragma once

#include "tcp_client.h"

class DataManager
{
public:
  DataManager(const std::string &username, const std::string &host, u16 port, spdlog::logger *logger);
  ~DataManager() = default;

public:
  [[nodiscard]] bool sendMessage(const std::string& message) const noexcept;
  [[nodiscard]] std::string getUsername() const noexcept;
  [[nodiscard]] std::vector<server::messages::NewMessageReceived> getMessages() const noexcept;
  [[nodiscard]] std::map<std::string, UserData> getUsers() const noexcept;

private:
  void onConnect();
  void onDisconnect();
  void onMessage(const server::messages::ServerMessage& message);

private:
  void manageMessageContent(const server::messages::NewMessageReceived &value);
  void manageMessageContent(const server::messages::UserStatus &value);
  void manageMessageContent(const server::messages::ServerResponse &value);

private:
  spdlog::logger* logger_;
  std::unique_ptr<TcpClient> tcpClient_;

private:
  // Data containers
  const std::string username;
  std::map<std::string, UserData> usersMap_;
  std::vector<server::messages::NewMessageReceived> messages_;
};

