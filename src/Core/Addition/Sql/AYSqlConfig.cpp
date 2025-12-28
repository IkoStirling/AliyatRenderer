#include "AYSqlConfig.h"
#include <fstream>
#include <iostream>
#include <sstream>
namespace ayt::engine::sql
{
    SqlConfig SqlConfig::loadFromFile(const std::string& configPath)
    {
        SqlConfig config;

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
                    if (value == "SQLite3") config.type = SqlType::SQLite3;
                    else if (value == "MySQL") config.type = SqlType::MySQL;
                    else if (value == "PostgreSQL") config.type = SqlType::PostgreSQL;
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

    SqlConfig SqlConfig::SQLiteConfig(const std::string& dbFilePath)
    {
        SqlConfig config;
        config.type = SqlType::SQLite3;
        config.connectionString = dbFilePath;
        return config;
    }

    SqlConfig SqlConfig::MySQLConfig(const std::string& host, int port, const std::string& dbName, const std::string& user, const std::string& password)
    {
        SqlConfig config;
        config.type = SqlType::MySQL;
        config.connectionString = "host=" + host + " port=" + std::to_string(port) +
            " dbname=" + dbName + " user=" + user +
            " password='" + password + "'";
        return config;
    }

    SqlConfig SqlConfig::PostgreSQLConfig(const std::string& host, int port,
        const std::string& dbName,
        const std::string& user,
        const std::string& password) {
        SqlConfig config;
        config.type = SqlType::PostgreSQL;
        config.connectionString = "host=" + host + " port=" + std::to_string(port) +
            " dbname=" + dbName + " user=" + user +
            " password='" + password + "'";
        return config;
    }
}