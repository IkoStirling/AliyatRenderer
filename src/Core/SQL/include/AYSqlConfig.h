#pragma once

#include <string>
#include <unordered_map>
namespace ayt::engine::sql
{
    // 支持的数据库类型
    enum class STSqlType {
        SQLite3,
        MySQL,
        PostgreSQL
        // ...
    };

    struct AYSqlConfig {
        STSqlType type;
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

        static AYSqlConfig loadFromFile(const std::string& configPath);

        static AYSqlConfig SQLiteConfig(const std::string& dbFilePath);
        static AYSqlConfig MySQLConfig(const std::string& host, int port,
            const std::string& dbName,
            const std::string& user,
            const std::string& password);
        static AYSqlConfig PostgreSQLConfig(const std::string& host, int port,
            const std::string& dbName,
            const std::string& user,
            const std::string& password);
    };
}