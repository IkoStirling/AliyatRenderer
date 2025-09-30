#include <iostream>
#include <AYSqlPool.h>
#include <iomanip>
#include <AYCmdInterface.h>
#include <fstream>

struct User {
	int id;
	std::string name;
	int age;
	std::string email;
	double salary;
};

void printHelp() {
    std::cout << "Usage: dbcli [options]\n"
        << "Options:\n"
        << "  -d, --database TYPE     Database type (mysql/sqlite/postgresql)\n"
        << "  -u, --username USER     Database username\n"
        << "  -p, --password PASS     Database password\n"
        << "  -n, --dbname NAME       Database name\n"
        << "  -h, --host HOST         Database host\n"
        << "  -P, --port PORT         Database port\n"
        << "  -f, --file FILE         Local database file (for SQLite)\n"
        << "  --help                  Show this help message\n";
}

struct CommandLineArgs {
    STSqlType dbType = STSqlType::SQLite3;
    std::string username;
    std::string password;
    std::string dbName;
    std::string host;
    int port = 0;
    std::string filePath;
    bool helpRequested = false;
};

CommandLineArgs parseArguments(int argc, char* argv[]) {
    CommandLineArgs args;

    if (argc == 1) {
        args.helpRequested = true;
        return args;
    }

    for (int i = 1; i < argc; ) {
        std::string current = argv[i];

        // 检查是否是参数标记（以-开头）
        if (current[0] == '-') {
            // 检查下一个参数是否也是标记
            if (i + 1 < argc && argv[i + 1][0] == '-') {
                // 如果是--help之类的特殊标记则允许
                if (current == "--help" || argv[i + 1] == "--help") {
                    i++;
                    continue;
                }
                throw std::runtime_error("Missing value for parameter: " + current);
            }
            i += 2; // 跳过当前标记和它的值
        }
        else {
            throw std::runtime_error("Unexpected positional argument: " + current);
        }
    }

    std::vector<std::string> arguments(argv + 1, argv + argc);

    for (size_t i = 0; i < arguments.size(); ++i) {
        const std::string& arg = arguments[i];

        if (arg == "-d" || arg == "--database") {
            if (i + 1 >= arguments.size()) {
                throw std::runtime_error("Missing database type after " + arg);
            }
            std::string type = arguments[++i];
            std::transform(type.begin(), type.end(), type.begin(), ::tolower);

            if (type == "mysql") {
                args.dbType = STSqlType::MySQL;
            }
            else if (type == "postgresql" || type == "postgres") {
                args.dbType = STSqlType::PostgreSQL;
            }
            else if (type == "sqlite" || type == "sqlite3") {
                args.dbType = STSqlType::SQLite3;
            }
            else {
                throw std::runtime_error("Unsupported database type: " + type);
            }
        }
        else if (arg == "-u" || arg == "--username") {
            if (i + 1 >= arguments.size()) {
                throw std::runtime_error("Missing username after " + arg);
            }
            args.username = arguments[++i];
        }
        else if (arg == "-p" || arg == "--password") {
            if (i + 1 >= arguments.size()) {
                throw std::runtime_error("Missing password after " + arg);
            }
            args.password = arguments[++i];
        }
        else if (arg == "-n" || arg == "--dbname") {
            if (i + 1 >= arguments.size()) {
                throw std::runtime_error("Missing database name after " + arg);
            }
            args.dbName = arguments[++i];
        }
        else if (arg == "-h" || arg == "--host") {
            if (i + 1 >= arguments.size()) {
                throw std::runtime_error("Missing host after " + arg);
            }
            args.host = arguments[++i];
        }
        else if (arg == "-P" || arg == "--port") {
            if (i + 1 >= arguments.size()) {
                throw std::runtime_error("Missing port after " + arg);
            }
            args.port = std::stoi(arguments[++i]);
        }
        else if (arg == "-f" || arg == "--file") {
            if (i + 1 >= arguments.size()) {
                throw std::runtime_error("Missing file path after " + arg);
            }
            args.filePath = arguments[++i];
        }
        else if (arg == "--help") {
            args.helpRequested = true;
        }
        else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    return args;
}

int main(int argc, char* argv[])
{
	try {
        CommandLineArgs args = parseArguments(argc, argv);

        if (args.helpRequested || argc == 1) {
            printHelp();
            return 0;
        }
        AYCommandInterface cmdInterface;
        std::unique_ptr<IAYCommandParser> parser;
        AYSqlPool& sqlPool = AYSqlPool::getInstance();
        AYSqlConfig config;

        switch (args.dbType) {
        case STSqlType::SQLite3:
        {
            if (args.filePath.empty()) {
                throw std::runtime_error("SQLite requires a database file path (-f/--file)");
            }
            std::ifstream testFile(args.filePath);
            if (!testFile.good())
            {
                testFile.close();
                std::cout << "SQLite database file dose not exists, should create new one? (don't enter space)[y/n]";
                std::string input;
                std::getline(std::cin, input);
                std::transform(input.begin(), input.end(), input.begin(), ::tolower);
                if (input == "y" || input == "yes")
                {
                    std::cout << "Create new SQLite database, path: " << args.filePath << std::endl;
                }
                else
                {
                    std::cout << "Exit!\n";
                    return -1;
                }
            }

            config = AYSqlConfig::SQLiteConfig(args.filePath);
            sqlPool.initialize(config);
            auto conn = sqlPool.getConnection();
            auto& session = conn->session();
            parser = cmdInterface.loadConfig("assets/core/config/SQL/sqlite_ui.json");
            cmdInterface.setExecution([&session](STParsedCommand& cmd) {
                try {
                    // 判断是否是SELECT查询
                    std::string prefix = cmd.commandName.substr(0, 6);
                    std::transform(prefix.begin(), prefix.end(), prefix.begin(),
                        [](unsigned char c) { return std::tolower(c); });
                    bool isSelect = (prefix == "select");

                    if (isSelect) {
                        soci::rowset<soci::row> rows = (session.prepare << cmd.normalized);
                        std::stringstream output;
                        AYSqlConnection::printQueryResult(rows, output);
                        return STCommandResult{ true, output.str(), "" };
                    }
                    else {
                        soci::statement st = (session.prepare << cmd.normalized);
                        st.alloc();
                        st.prepare(cmd.normalized);
                        st.define_and_bind();
                        st.execute(true);

                        int affected = 0;
                        try {
                            affected = st.get_affected_rows();
                        }
                        catch (...) {
                            // 某些操作可能无法获取影响行数
                        }

                        std::string msg = "Query executed";
                        if (affected > 0) {
                            msg += ". Affected rows: " + std::to_string(affected);
                        }
                        return STCommandResult{ true, msg, "" };
                    }
                }
                catch (const std::exception& e) {
                    return STCommandResult{ false, "", "SQL Error: " + std::string(e.what()) };
                }
                });
            break;
        }
        case STSqlType::MySQL:
        {
            if (args.host.empty() || args.dbName.empty() || args.username.empty()) {
                throw std::runtime_error("MySQL requires host, database name and username");
            }
            config = AYSqlConfig::MySQLConfig(args.host, args.port,
                args.dbName, args.username, args.password);
            sqlPool.initialize(config);
            auto conn = sqlPool.getConnection();
            auto& session = conn->session();
            parser = cmdInterface.loadConfig("assets/core/config/SQL/mysql_ui.json");
            cmdInterface.setExecution([&session](STParsedCommand& cmd) {
                try {
                    // 特殊处理MySQL的SHOW命令
                    std::string lowerCmd;
                    std::transform(cmd.commandName.begin(), cmd.commandName.end(),
                        std::back_inserter(lowerCmd), ::tolower);

                    if (lowerCmd == "show") {
                        soci::rowset<soci::row> rows = (session.prepare << cmd.normalized);
                        std::stringstream output;
                        AYSqlConnection::printQueryResult(rows, output);
                        return STCommandResult{ true, output.str(), "" };
                    }

                    // 处理其他命令
                    soci::statement st = (session.prepare << cmd.normalized);
                    st.alloc();
                    st.prepare(cmd.normalized);
                    st.define_and_bind();
                    st.execute(true);

                    int affected = 0;
                    try {
                        affected = st.get_affected_rows();
                    }
                    catch (...) {}

                    std::string msg = "Query executed";
                    if (affected > 0) {
                        msg += ". Affected rows: " + std::to_string(affected);
                    }
                    return STCommandResult{ true, msg, "" };
                }
                catch (const std::exception& e) {
                    return STCommandResult{ false, "", "MySQL Error: " + std::string(e.what()) };
                }
                });
            break;
        }
        case STSqlType::PostgreSQL:
        {
            if (args.host.empty() || args.dbName.empty() || args.username.empty()) {
                throw std::runtime_error("PostgreSQL requires host, database name and username");
            }
            config = AYSqlConfig::PostgreSQLConfig(args.host, args.port,
                args.dbName, args.username, args.password);
            sqlPool.initialize(config);
            auto conn = sqlPool.getConnection();
            auto& session = conn->session();
            parser = cmdInterface.loadConfig("assets/core/config/SQL/postgres_ui.json");
            cmdInterface.setExecution([&session](STParsedCommand& cmd) {
                try {
                    // 特殊处理PostgreSQL的元命令
                    if (cmd.commandName[0] == '\\') {
                        return STCommandResult{ false, "", "PostgreSQL meta-commands not supported" };
                    }

                    soci::statement st = (session.prepare << cmd.normalized);
                    st.alloc();
                    st.prepare(cmd.normalized);
                    st.define_and_bind();
                    st.execute(true);

                    // 尝试获取结果集（适用于SELECT等查询）
                    try {
                        soci::rowset<soci::row> rows = (session.prepare << cmd.normalized);
                        if (rows.begin() != rows.end()) { // 有结果集
                            std::stringstream output;
                            AYSqlConnection::printQueryResult(rows, output);
                            return STCommandResult{ true, output.str(), "" };
                        }
                    }
                    catch (...) {
                        // 不是查询语句
                    }

                    // 处理影响行数
                    int affected = 0;
                    try {
                        affected = st.get_affected_rows();
                    }
                    catch (...) {}

                    std::string msg = "Query executed";
                    if (affected > 0) {
                        msg += ". Affected rows: " + std::to_string(affected);
                    }
                    return STCommandResult{ true, msg, "" };
                }
                catch (const std::exception& e) {
                    return STCommandResult{ false, "", "PostgreSQL Error: " + std::string(e.what()) };
                }
                });
            break;
        }
        default:
            throw std::runtime_error("Unsupported database type");
        }

        cmdInterface.run(parser.release());
		sqlPool.shutdown();
	}
	catch (std::exception& e)
	{
		std::cout << "SQL cli error: " << e.what() << std::endl;
	}
	return 0;
}
