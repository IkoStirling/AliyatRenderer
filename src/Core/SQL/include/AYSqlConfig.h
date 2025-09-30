#pragma once

#include <string>
#include <unordered_map>

// ֧�ֵ����ݿ�����
enum class STSqlType {
    SQLite3,
    MySQL,
    PostgreSQL
    // ...
};

struct AYSqlConfig {
    STSqlType type;
    std::string connectionString;

    // ���ӳز���
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