#pragma once


#include "messages.h"


// sqlite3
#include "../sqlite3/sqlite3.h"

class DataBaseManager
{
public:
    explicit DataBaseManager(spdlog::logger* logger);
    ~DataBaseManager();

public:
    // Messages table functions
    void addMessageEntry(const server::messages::NewMessageReceived& message);
    [[nodiscard]] std::vector<server::messages::NewMessageReceived> getMessages() const noexcept;

    // User table functions
    //void addNewUser()

private:
    // helpers
    void ensureSchema() const noexcept;
    static void finalizeSilently(sqlite3_stmt* stmt) noexcept;

private:
    spdlog::logger* logger_;
    sqlite3* db_{nullptr};
};


