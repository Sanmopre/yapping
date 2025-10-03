#include "db_manager.h"

#include <cassert>

static constexpr std::string_view defaultDbPath = "chat.db";

static constexpr std::string_view createMessagesTableSQL = R"SQL(
CREATE TABLE IF NOT EXISTS messages (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    username  TEXT    NOT NULL,
    message   TEXT    NOT NULL,
    timestamp INTEGER NOT NULL
);
)SQL";


static constexpr std::string_view createUsersTableSQL = R"SQL(
CREATE TABLE IF NOT EXISTS users (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    username  TEXT    NOT NULL,
    password  TEXT    NOT NULL
);
)SQL";

DataBaseManager::DataBaseManager(spdlog::logger *logger)
    : logger_(logger)
{
    if (const int rc = sqlite3_open(defaultDbPath.data(), &db_); rc != SQLITE_OK)
    {
        const std::string err = sqlite3_errmsg(db_ ? db_ : nullptr);
        if (db_)
        {
            sqlite3_close(db_); db_ = nullptr;
        }
        logger_->error("Failed to open SQLite DB: {}", err);
    }

    ensureSchema();
}

DataBaseManager::~DataBaseManager()
{
    if (db_)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void DataBaseManager::ensureSchema() const noexcept
{
    char* errMsg = nullptr;

    if (const int rc = sqlite3_exec(db_, createMessagesTableSQL.data(), nullptr, nullptr, &errMsg); rc != SQLITE_OK)
    {
        std::string err = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        logger_->error("Failed to ensure schema: {}" ,err);
    }

    if (const int rc = sqlite3_exec(db_, createUsersTableSQL.data(), nullptr, nullptr, &errMsg); rc != SQLITE_OK)
    {
        std::string err = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        logger_->error("Failed to ensure schema: {}" ,err);
    }
}

void DataBaseManager::finalizeSilently(sqlite3_stmt* stmt) noexcept
{
    if (stmt)
    {
        sqlite3_finalize(stmt);
    }
}

void DataBaseManager::addMessageEntry(const server::messages::NewMessageReceived& message)
{
    static constexpr std::string_view kInsertSQL =
        "INSERT INTO messages (username, message, timestamp) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, kInsertSQL.data(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        const std::string err = sqlite3_errmsg(db_);
        finalizeSilently(stmt);
        logger_->error("Failed to prepare INSERT: {}", err);
    }

    sqlite3_bind_text(stmt, 1, message.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, message.message.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(message.timestamp));

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::string err = sqlite3_errmsg(db_);
        finalizeSilently(stmt);
        logger_->error("Failed to execute INSERT: {}", err);
    }

    finalizeSilently(stmt);
}

std::vector<server::messages::NewMessageReceived> DataBaseManager::getMessages() const noexcept
{
    std::vector<server::messages::NewMessageReceived> out;

    static constexpr std::string_view kSelectSQL =
        "SELECT username, message, timestamp FROM messages ORDER BY id ASC;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, kSelectSQL.data(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logger_->error("Failed to prepare SELECT: {}", sqlite3_errmsg(db_));
        finalizeSilently(stmt);
        return out;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const unsigned char* u = sqlite3_column_text(stmt, 0);
        const unsigned char* m = sqlite3_column_text(stmt, 1);
        const sqlite3_int64 ts       = sqlite3_column_int64(stmt, 2);

        const std::string username  = u ? reinterpret_cast<const char*>(u) : "";
        const std::string message   = m ? reinterpret_cast<const char*>(m) : "";
        const u64 timestamp    = ts;

        server::messages::NewMessageReceived newMessage;
        newMessage.username = username;
        newMessage.message = message;
        newMessage.timestamp = timestamp;

        out.emplace_back(newMessage);
    }

    finalizeSilently(stmt);
    return out;
}

void DataBaseManager::addNewUser(const std::string& username, u64 passwordHash)
{
}

bool DataBaseManager::userExists(const std::string& username)
{
}

u64 DataBaseManager::userPasswordHash(const std::string& username)
{
}
