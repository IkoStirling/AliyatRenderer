#define NOMINMAX
#include "AYSqlConnection.h"
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/postgresql/soci-postgresql.h>
#include <soci/mysql/soci-mysql.h>
#include <soci/odbc/soci-odbc.h>
#include <iostream>
#include <iomanip>


AYSqlConnection::AYSqlConnection(const AYSqlConfig& config)
    : _config(config) {
    connect();
}

AYSqlConnection& AYSqlConnection::operator=(AYSqlConnection&& other) noexcept {
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

AYSqlConnection::~AYSqlConnection()
{
    if (_inTransaction) {
        try { rollback(); }
        catch (...) {}
    }
    disconnect();
}

void AYSqlConnection::begin()
{
    if (_inTransaction) {
        throw AYSqlException("Transaction already started");
    }
    session().begin();
    _inTransaction = true;
}

void AYSqlConnection::commit()
{
    if (!_inTransaction) {
        throw AYSqlException("No transaction to commit");
    }
    session().commit();
    _inTransaction = false;
}

void AYSqlConnection::rollback()
{
    if (!_inTransaction) {
        throw AYSqlException("No transaction to commit");
    }
    session().rollback();
    _inTransaction = false;
}

void AYSqlConnection::printQueryResult(soci::rowset<soci::row>& rows)
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
                    auto tm = row.get<std::tm>(i);
                    std::cout << std::put_time(&tm, "%F %T");
                    break;
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

void AYSqlConnection::printQueryResult(soci::rowset<soci::row>& rows, std::ostream& os)
{
    // ��һ�α������ռ������ͼ����п�
    std::vector<std::string> columnNames;
    std::vector<std::size_t> columnWidths;
    bool firstRow = true;
    std::vector<std::vector<std::string>> allRows; // �洢�����е�����
    std::vector<soci::column_properties> columnProps;

    for (const soci::row& row : rows) {
        if (firstRow) {
            // ��ʼ���������п�
            for (std::size_t i = 0; i != row.size(); ++i) {
                columnNames.push_back(row.get_properties(i).get_name());
                columnWidths.push_back(columnNames.back().size());
                columnProps.push_back(row.get_properties(i));
            }
            firstRow = false;
        }

        // ����ǰ������
        std::vector<std::string> currentRow;
        for (std::size_t i = 0; i != row.size(); ++i) {
            std::string value;
            try {
                // ���������ͻ�ȡ����
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
                default:
                    // �����Ͳ����ӡ
                    value = "[unsupported]";
                }
            }
            catch (...) {
                value = "[error]";
            }
            currentRow.push_back(value);
            columnWidths[i] = std::max(columnWidths[i], value.size());
        }
        allRows.push_back(currentRow);
    }

    // ���û�����ݣ�ֻ��ӡ����
    if (allRows.empty()) {
        for (std::size_t i = 0; i != columnNames.size(); ++i) {
            os << columnNames[i];
            if (i != columnNames.size() - 1) os << " ";
        }
        os << "\nNo rows found\n";
        return;
    }

    // ��ӡ��ͷ
    for (std::size_t i = 0; i != columnNames.size(); ++i) {
        os << std::left << std::setw(columnWidths[i] + 2) << columnNames[i];
    }
    os << "\n";

    // ��ӡ�ָ���
    for (std::size_t i = 0; i != columnNames.size(); ++i) {
        os << std::string(columnWidths[i] + 1, '-') << " ";
    }
    os << "\n";

    // ��ӡ������
    for (const auto& row : allRows) {
        for (std::size_t i = 0; i != row.size(); ++i) {
            os << std::left << std::setw(columnWidths[i] + 2) << row[i];
        }
        os << "\n";
    }
}

std::string AYSqlConnection::replaceNamedParams(const std::string& sql)
{
    static const std::regex namedParamRegex(R"(:(\w+))");
    std::unordered_map<std::string, int> paramIndexMap;
    std::string result;
    int counter = 1;

    auto it = std::sregex_iterator(sql.begin(), sql.end(), namedParamRegex);
    auto end = std::sregex_iterator();
    size_t lastPos = 0;

    for (; it != end; ++it) {
        // ��ӷǲ�������
        result += sql.substr(lastPos, it->position() - lastPos);

        const std::string paramName = it->str();
        // ����Ƿ��Ѵ�����ͬ������
        if (paramIndexMap.find(paramName) == paramIndexMap.end()) {
            paramIndexMap[paramName] = counter++;
        }
        // �滻Ϊλ�ò���
        result += ":" + std::to_string(paramIndexMap[paramName]);

        lastPos = it->position() + it->length();
    }
    // ���ʣ�ಿ��
    result += sql.substr(lastPos);

    return result;
}

void AYSqlConnection::connect()
{
    try {
        _session = std::make_unique<soci::session>();

        switch (_config.type) {
        case STSqlType::SQLite3:
            _session->open(soci::sqlite3, _config.connectionString);
            break;
        case STSqlType::MySQL:
            _session->open(soci::mysql, _config.connectionString);
            break;
        case STSqlType::PostgreSQL:
            _session->open(soci::postgresql, _config.connectionString);
            break;
        default:
            throw AYSqlException("Unsupported database type");
        }

        // �������ӳ�ʱ�Ȳ���
        if (_config.timeout > 0) {
            try {
                switch (_config.type) {
                case STSqlType::MySQL:
                    *_session << "SET SESSION wait_timeout = " << _config.timeout;
                    break;
                case STSqlType::PostgreSQL:
                    *_session << "SET statement_timeout = " << _config.timeout * 1000;
                    break;
                default:
                    break;
                }
            }
            catch (const std::exception& e) {
                // ��ʱ����ʧ�ܲ�Ӱ����Ҫ����
            }
        }

    }
    catch (const std::exception& e) {
        throw AYSqlException(std::string("Connection failed: ") + e.what());
    }
}

void AYSqlConnection::disconnect()
{
    if (_session && _session->is_connected()) {
        try {
            _session->close();
        }
        catch (const std::exception& e) {
            // ��¼��־�������׳��쳣
        }
    }
}