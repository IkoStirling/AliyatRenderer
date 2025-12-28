#define NOMINMAX
#include "AYSqlConnection.h"
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/postgresql/soci-postgresql.h>
#include <soci/mysql/soci-mysql.h>
#include <soci/odbc/soci-odbc.h>
#include <iostream>
#include <iomanip>

namespace ayt::engine::sql
{
    SqlConnection::SqlConnection(const SqlConfig& config)
        : _config(config) {
        connect();
    }

    SqlConnection& SqlConnection::operator=(SqlConnection&& other) noexcept {
        if (this != &other) {
            if (_inTransaction) {
                try { rollback(); }
                catch (...) {}
            }
            disconnect();

            _session = std::move(other._session);
            _config = std::move(other._config);
            _inTransaction = other._inTransaction;
            other._inTransaction = false;
        }
        return *this;
    }

    SqlConnection::~SqlConnection()
    {
        if (_inTransaction) {
            try { rollback(); }
            catch (...) {}
        }
        disconnect();
    }

    void SqlConnection::begin()
    {
        if (_inTransaction) {
            throw SqlException("Transaction already started");
        }
        session().begin();
        _inTransaction = true;
    }

    void SqlConnection::commit()
    {
        if (!_inTransaction) {
            throw SqlException("No transaction to commit");
        }
        session().commit();
        _inTransaction = false;
    }

    void SqlConnection::rollback()
    {
        if (!_inTransaction) {
            throw SqlException("No transaction to commit");
        }
        session().rollback();
        _inTransaction = false;
    }

    void SqlConnection::printQueryResult(soci::rowset<soci::row>& rows)
    {
        for (auto& row : rows)
        {
            for (int i = 0; i < row.size(); i++)
            {
                auto propertyName = row.get_properties(i).get_name();
                auto propertyType = row.get_properties(i).get_data_type();
                std::cout << row.get_properties(i).get_name() << ": ";
                if (row.get_indicator(i) != soci::i_null)
                {
                    switch (propertyType)
                    {
                    case soci::dt_string:
                        std::cout << row.get<std::string>(i);
                        break;
                    case soci::dt_date:
                    {
                        auto tm = row.get<std::tm>(i);
                        std::cout << std::put_time(&tm, "%F %T");
                        break;
                    }
                    case soci::dt_double:
                        std::cout << row.get<double>(i);
                        break;
                    case soci::dt_integer:
                        std::cout << row.get<int>(i);
                        break;
                    case soci::dt_long_long:
                        std::cout << row.get<long long>(i);
                        break;
                    case soci::dt_unsigned_long_long:
                        std::cout << row.get<unsigned long long>(i);
                        break;
                    case soci::dt_blob:
                        //std::cout << row.get<std::vector<char>>(i);
                        break;
                    case soci::dt_xml:
                        //std::cout << row.get<std::string>(i);
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    std::cout << "NULL";
                }

                if (i != row.size() - 1)
                    std::cout << ", ";
            }
            std::cout << std::endl;
        }
    }

    void SqlConnection::printQueryResult(soci::rowset<soci::row>& rows, std::ostream& os)
    {
        // 第一次遍历：收集列名和计算列宽
        std::vector<std::string> columnNames;
        std::vector<std::size_t> columnWidths;
        bool firstRow = true;
        std::vector<std::vector<std::string>> allRows; // 存储所有行的数据
        std::vector<soci::column_properties> columnProps;

        for (const soci::row& row : rows) {
            if (firstRow) {
                // 初始化列名和列宽
                for (std::size_t i = 0; i != row.size(); ++i) {
                    columnNames.push_back(row.get_properties(i).get_name());
                    columnWidths.push_back(columnNames.back().size());
                    columnProps.push_back(row.get_properties(i));
                }
                firstRow = false;
            }

            // 处理当前行数据
            std::vector<std::string> currentRow;
            for (std::size_t i = 0; i != row.size(); ++i) {
                std::string value;
                try {
                    // 根据列类型获取数据
                    if (row.get_indicator(i) == soci::i_null) {
                        value = "NULL";
                    }
                    else
                    {
                        switch (columnProps[i].get_data_type()) {
                        case soci::dt_string:
                            value = row.get<std::string>(i);
                            break;
                        case soci::dt_integer:
                            value = std::to_string(row.get<int>(i));
                            break;
                        case soci::dt_double:
                            value = std::to_string(row.get<double>(i));
                            break;
                        case soci::dt_long_long:
                            value = std::to_string(row.get<long long>(i));
                            break;
                        case soci::dt_unsigned_long_long:
                            value = std::to_string(row.get<unsigned long long>(i));
                            break;
                        case soci::dt_date: {
                            std::tm when = row.get<std::tm>(i);
                            char buf[20];
                            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &when);
                            value = buf;
                            break;
                        }
                        case soci::dt_blob:
                            value = "[BLOB]";
                            break;
                        case soci::dt_xml:
                            value = "[XML]";
                            break;
                        default:
                            try {
                                value = row.get<std::string>(i);
                            }
                            catch (...) {
                                value = "[unknown]";
                            }
                        }
                    }
                }
                catch (const std::exception& e) {
                    value = "[error: " + std::string(e.what()) + "]";
                }
                catch (...) {
                    value = "[unknown error]";
                }
                currentRow.push_back(value);
                columnWidths[i] = std::max(columnWidths[i], value.size());
            }
            allRows.push_back(currentRow);
        }

        // 如果没有数据，只打印列名
        if (allRows.empty()) {
            for (std::size_t i = 0; i != columnNames.size(); ++i) {
                os << columnNames[i];
                if (i != columnNames.size() - 1) os << " ";
            }
            os << "\nNo rows found\n";
            return;
        }

        // 打印表头
        for (std::size_t i = 0; i != columnNames.size(); ++i) {
            os << std::left << std::setw(columnWidths[i] + 2) << columnNames[i];
        }
        os << "\n";

        // 打印分隔线
        for (std::size_t i = 0; i != columnNames.size(); ++i) {
            os << std::string(columnWidths[i] + 1, '-') << " ";
        }
        os << "\n";

        // 打印数据行
        for (const auto& row : allRows) {
            for (std::size_t i = 0; i != row.size(); ++i) {
                os << std::left << std::setw(columnWidths[i] + 2) << row[i];
            }
            os << "\n";
        }
    }

    std::string SqlConnection::replaceNamedParams(const std::string& sql)
    {
        static const std::regex namedParamRegex(R"(:(\w+))");
        std::unordered_map<std::string, int> paramIndexMap;
        std::string result;
        int counter = 1;

        auto it = std::sregex_iterator(sql.begin(), sql.end(), namedParamRegex);
        auto end = std::sregex_iterator();
        size_t lastPos = 0;

        for (; it != end; ++it) {
            // 添加非参数部分
            result += sql.substr(lastPos, it->position() - lastPos);

            const std::string paramName = it->str();
            // 检查是否已存在相同参数名
            if (paramIndexMap.find(paramName) == paramIndexMap.end()) {
                paramIndexMap[paramName] = counter++;
            }
            // 替换为位置参数
            result += ":" + std::to_string(paramIndexMap[paramName]);

            lastPos = it->position() + it->length();
        }
        // 添加剩余部分
        result += sql.substr(lastPos);

        return result;
    }

    void SqlConnection::connect()
    {
        try {
            _session = std::make_unique<soci::session>();

            switch (_config.type) {
            case SqlType::SQLite3:
                _session->open(soci::sqlite3, _config.connectionString);
                break;
            case SqlType::MySQL:
                _session->open(soci::mysql, _config.connectionString);
                break;
            case SqlType::PostgreSQL:
                _session->open(soci::postgresql, _config.connectionString);
                break;
            default:
                throw SqlException("Unsupported database type");
            }

            // 设置连接超时等参数
            if (_config.timeout > 0) {
                try {
                    switch (_config.type) {
                    case SqlType::MySQL:
                        *_session << "SET SESSION wait_timeout = " << _config.timeout;
                        break;
                    case SqlType::PostgreSQL:
                        *_session << "SET statement_timeout = " << _config.timeout * 1000;
                        break;
                    default:
                        break;
                    }
                }
                catch (const std::exception& e) {
                    // 超时设置失败不影响主要功能
                }
            }

        }
        catch (const std::exception& e) {
            throw SqlException(std::string("Connection failed: ") + e.what());
        }
    }

    void SqlConnection::disconnect()
    {
        if (_session && _session->is_connected()) {
            try {
                _session->close();
            }
            catch (const std::exception& e) {
                // 记录日志，但不抛出异常
            }
        }
    }
}