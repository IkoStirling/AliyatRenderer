#pragma once

#include <string>
#include <unordered_map>
namespace ayt::engine::sql
{
    // 支持的数据库类型
    enum class SqlType {
        SQLite3,
        MySQL,
        PostgreSQL
        // ...
    };

    struct SqlConfig {
        SqlType type;
        std::string connectionString;

        // 连接池参数
        int poolSize = 5;
        int timeout = 30;

        std::string username;
        std::string password;
        std::string dbName;
        std::string host;
        int port = 0;
        std::string filePath;

        static SqlConfig loadFromFile(const std::string& configPath);

        static SqlConfig SQLiteConfig(const std::string& dbFilePath);
        static SqlConfig MySQLConfig(const std::string& host, int port,
            const std::string& dbName,
            const std::string& user,
            const std::string& password);
        static SqlConfig PostgreSQLConfig(const std::string& host, int port,
            const std::string& dbName,
            const std::string& user,
            const std::string& password);
    };
}