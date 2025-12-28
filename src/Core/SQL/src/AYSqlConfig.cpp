#include "AYSqlConfig.h"
#include <fstream>
#include <iostream>
#include <sstream>
namespace ayt::engine::sql
{
    AYSqlConfig AYSqlConfig::loadFromFile(const std::string& configPath)
    {
        AYSqlConfig config;

        std::ifstream file(configPath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + configPath);
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                // 去除前后空格
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (key == "type") {
                    if (value == "SQLite3") config.type = STSqlType::SQLite3;
                    else if (value == "MySQL") config.type = STSqlType::MySQL;
                    else if (value == "PostgreSQL") config.type = STSqlType::PostgreSQL;
                }
                else if (key == "connectionString") {
                    config.connectionString = value;
                }
                else if (key == "poolSize") {
                    config.poolSize = std::stoi(value);
                }
                else if (key == "timeout") {
                    config.timeout = std::stoi(value);
                }
            }
        }

        return config;
    }

    AYSqlConfig AYSqlConfig::SQLiteConfig(const std::string& dbFilePath)
    {
        AYSqlConfig config;
        config.type = STSqlType::SQLite3;
        config.connectionString = dbFilePath;
        return config;
    }

    AYSqlConfig AYSqlConfig::MySQLConfig(const std::string& host, int port, const std::string& dbName, const std::string& user, const std::string& password)
    {
        AYSqlConfig config;
        config.type = STSqlType::MySQL;
        config.connectionString = "host=" + host + " port=" + std::to_string(port) +
            " dbname=" + dbName + " user=" + user +
            " password='" + password + "'";
        return config;
    }

    AYSqlConfig AYSqlConfig::PostgreSQLConfig(const std::string& host, int port,
        const std::string& dbName,
        const std::string& user,
        const std::string& password) {
        AYSqlConfig config;
        config.type = STSqlType::PostgreSQL;
        config.connectionString = "host=" + host + " port=" + std::to_string(port) +
            " dbname=" + dbName + " user=" + user +
            " password='" + password + "'";
        return config;
    }
}