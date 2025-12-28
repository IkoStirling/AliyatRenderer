#include <iostream>
#include <AYSqlPool.h>
#include <iomanip>
#include <AYCmdInterface.h>
#include <fstream>
#include <soci/mysql/soci-mysql.h>

using namespace ayt::engine::cmd;
using namespace ayt::engine::sql;

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

#include <cctype>
#include <algorithm>

CommandResult handlePostgreSQLMetaCommand(soci::session& session, const ParsedCommand& cmd) {
    std::string metaCmd = cmd.commandName.substr(1); // 去掉开头的反斜杠
    std::transform(metaCmd.begin(), metaCmd.end(), metaCmd.begin(), ::tolower);

    try {
        // === 解析命令和参数 ===
        std::string baseCmd;
        std::string param;

        size_t spacePos = metaCmd.find(' ');
        if (spacePos != std::string::npos) {
            baseCmd = metaCmd.substr(0, spacePos);
            param = metaCmd.substr(spacePos + 1);
            // 去除参数前后的空格
            param.erase(0, param.find_first_not_of(" \t"));
            param.erase(param.find_last_not_of(" \t") + 1);
        }
        else {
            baseCmd = metaCmd;
        }

        // === 数据库连接操作 ===
        if (baseCmd == "c" || baseCmd == "connect") {
            if (param.empty()) {
                // 显示当前连接信息
                soci::row currentDB;
                session << "SELECT current_database(), current_user", soci::into(currentDB);

                std::stringstream output;
                output << "You are connected to database \"" << currentDB.get<std::string>(0)
                    << "\" as user \"" << currentDB.get<std::string>(1) << "\"\n";
                return CommandResult{ true, output.str(), "" };
            }
            else {
                // 在实际应用中，这里应该重新初始化一个新的 session 连接到指定数据库
                std::stringstream output;
                output << "Would connect to database: " << param
                    << " (Reconnect functionality not implemented in this interface)\n";
                output << "Hint: Use different connection parameters or restart the tool with -n " << param;
                return CommandResult{ true, output.str(), "" };
            }
        }

        // === 列出数据库 ===
        else if (baseCmd == "l" || baseCmd == "list") {
            std::string whereClause = "";
            if (!param.empty()) {
                whereClause = "WHERE datname LIKE :pattern ";
            }

            std::string sql = R"(
                SELECT d.datname as "Name",
                       pg_catalog.pg_get_userbyid(d.datdba) as "Owner",
                       pg_catalog.pg_encoding_to_char(d.encoding) as "Encoding",
                       CASE WHEN d.datallowconn THEN 'Yes' ELSE 'No' END as "Access"
                FROM pg_catalog.pg_database d
            )" + whereClause + "ORDER BY 1";

            std::stringstream output;
            output << "List of databases\n";
            output << "Name | Owner | Encoding | Access\n";
            output << "--------------------------------\n";
            if (!param.empty()) {
                soci::rowset<soci::row> rows = (session.prepare << sql, soci::use(param));
                for (const auto& row : rows) {
                    output << row.get<std::string>(0) << " | "
                        << row.get<std::string>(1) << " | "
                        << row.get<std::string>(2) << " | "
                        << row.get<std::string>(3) << "\n";
                }
            }
            else {
                soci::rowset<soci::row> rows = (session.prepare << sql);
                for (const auto& row : rows) {
                    output << row.get<std::string>(0) << " | "
                        << row.get<std::string>(1) << " | "
                        << row.get<std::string>(2) << " | "
                        << row.get<std::string>(3) << "\n";
                }
            }


            return CommandResult{ true, output.str(), "" };
        }

        // === 列出表 ===
        else if (baseCmd == "dt") {
            std::string pattern = param.empty() ? "%" : param;

            soci::rowset<soci::row> rows = (session.prepare <<
                R"(SELECT n.nspname as "Schema",
                          c.relname as "Name",
                          CASE c.relkind 
                            WHEN 'r' THEN 'table' 
                            WHEN 'v' THEN 'view' 
                            WHEN 'm' THEN 'materialized view'
                            ELSE c.relkind::text 
                          END as "Type",
                          pg_catalog.pg_get_userbyid(c.relowner) as "Owner"
                   FROM pg_catalog.pg_class c
                   LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
                   WHERE c.relkind IN ('r','v','m')
                   AND n.nspname NOT IN ('pg_catalog', 'information_schema')
                   AND c.relname LIKE :pattern
                   ORDER BY 1,2)")
                << pattern;

            std::stringstream output;
            output << "List of relations\n";
            output << "Schema | Name | Type | Owner\n";
            output << "------------------------------\n";
            for (const auto& row : rows) {
                output << row.get<std::string>(0) << " | "
                    << row.get<std::string>(1) << " | "
                    << row.get<std::string>(2) << " | "
                    << row.get<std::string>(3) << "\n";
            }
            return CommandResult{ true, output.str(), "" };
        }

        // === 描述表结构 ===
        else if (baseCmd == "d") {
            if (param.empty()) {
                return CommandResult{ false, "", "Object name required after \\d" };
            }

            // 检查对象是否存在并获取类型
            soci::row typeRow;
            session << R"(SELECT c.relkind, n.nspname 
                        FROM pg_class c 
                        JOIN pg_namespace n ON c.relnamespace = n.oid 
                        WHERE c.relname = :name 
                        AND n.nspname NOT IN ('pg_catalog', 'information_schema'))",
                soci::into(typeRow), soci::use(param);

            if (!session.got_data()) {
                return CommandResult{ false, "", "Object not found: " + param };
            }

            char relkind = typeRow.get<std::string>(0)[0];
            std::string schema = typeRow.get<std::string>(1);

            if (relkind == 'r' || relkind == 'v' || relkind == 'm') {
                // 表、视图或物化视图
                soci::rowset<soci::row> rows = (session.prepare <<
                    R"(SELECT a.attname as "Column",
                              pg_catalog.format_type(a.atttypid, a.atttypmod) as "Type",
                              CASE WHEN a.attnotnull THEN 'not null' ELSE '' END as "Modifiers"
                       FROM pg_catalog.pg_attribute a
                       WHERE a.attnum > 0 AND NOT a.attisdropped
                       AND a.attrelid = (SELECT c.oid FROM pg_class c 
                                         JOIN pg_namespace n ON c.relnamespace = n.oid 
                                         WHERE c.relname = :name AND n.nspname = :schema)
                       ORDER BY a.attnum)")
                    << param << schema;

                std::stringstream output;
                output << "Table \"" << schema << "\".\"" << param << "\"\n";
                output << "Column | Type | Modifiers\n";
                output << "-------------------------\n";
                for (const auto& row : rows) {
                    output << row.get<std::string>(0) << " | "
                        << row.get<std::string>(1) << " | "
                        << row.get<std::string>(2) << "\n";
                }
                return CommandResult{ true, output.str(), "" };
            }
            else if (relkind == 'i') {
                // 索引
                soci::rowset<soci::row> rows = (session.prepare <<
                    R"(SELECT i.relname as "Name",
                              am.amname as "Type",
                              pg_catalog.pg_get_indexdef(i.oid) as "Definition"
                       FROM pg_index x 
                       JOIN pg_class c ON c.oid = x.indrelid
                       JOIN pg_class i ON i.oid = x.indexrelid
                       JOIN pg_am am ON i.relam = am.oid
                       WHERE c.relname = :name)")
                    << param;

                std::stringstream output;
                output << "Indexes for table \"" << param << "\"\n";
                output << "Name | Type | Definition\n";
                output << "--------------------------\n";
                for (const auto& row : rows) {
                    output << row.get<std::string>(0) << " | "
                        << row.get<std::string>(1) << " | "
                        << row.get<std::string>(2) << "\n";
                }
                return CommandResult{ true, output.str(), "" };
            }
        }

        // === 退出 ===
        else if (baseCmd == "q" || baseCmd == "quit") {
            return CommandResult{ true, "Goodbye!", "EXIT" };
        }

        // === 帮助 ===
        else if (baseCmd == "?" || baseCmd == "help") {
            std::string help = R"(
Available meta-commands:
  \l [pattern]       List databases (optionally matching pattern)
  \dt [pattern]      List tables (optionally matching pattern)
  \d name           Describe table, view, or index
  \c [dbname]       Connect to another database (show current if no dbname)
  \q, \quit         Quit
  \?, \help         Show this help

Examples:
  \l                 List all databases
  \l test%           List databases starting with 'test'
  \dt                List all tables
  \dt user%          List tables starting with 'user'
  \d users           Describe users table
  \c soci_db         Connect to soci_db database
)";
            return CommandResult{ true, help, "" };
        }

        else {
            return CommandResult{ false, "", "Unsupported meta-command: \\" + metaCmd };
        }
    }
    catch (const std::exception& e) {
        return CommandResult{ false, "", "Error executing meta-command: " + std::string(e.what()) };
    }
}

int main(int argc, char* argv[])
{
	try {
        CommandLineArgs args = parseArguments(argc, argv);

        if (args.helpRequested || argc == 1) {
            printHelp();
            return 0;
        }
        CommandInterface cmdInterface;
        std::unique_ptr<ICommandParser> parser;
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
            cmdInterface.setExecution([&session](ParsedCommand& cmd) {
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
                        return CommandResult{ true, output.str(), "" };
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
                        return CommandResult{ true, msg, "" };
                    }
                }
                catch (const std::exception& e) {
                    return CommandResult{ false, "", "SQL Error: " + std::string(e.what()) };
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
            std::cout << "MySQL connection: \n\t"
                << config.connectionString << std::endl;;
            sqlPool.initialize(config);
            
            parser = cmdInterface.loadConfig("assets/core/config/SQL/mysql_ui.json");
            cmdInterface.setExecution([&sqlPool,&config](ParsedCommand& cmd) {
                try {
                    auto conn = sqlPool.getConnection();
                    auto& session = conn->session();
                    if (!conn->isConnected())
                    {
                        std::cout << "mysql reconnecting ...\n";
                        conn->connect();
                    }
                    std::string lowerCmd = cmd.commandName;
                    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

                    if (lowerCmd.find("show index") == 0) {
                        // 暂时不处理该情况
                        return CommandResult{ false, "", "This command now is not support" };
                        try {
                            soci::rowset<soci::row> rows = (session.prepare << cmd.normalized);
                            std::stringstream output;
                            output << "Table\tNon_unique\tKey_name\tSeq_in_index\tColumn_name\tIndex_type\n";
                            output << "---------------------------------------------------------------------\n";

                            for (const auto& row : rows) {
                                output << row.get<std::string>(0) << "\t"  // Table
                                    << row.get<int>(1) << "\t\t"         // Non_unique
                                    << row.get<std::string>(2) << "\t"   // Key_name
                                    << row.get<int>(3) << "\t\t"         // Seq_in_index
                                    << row.get<std::string>(4) << "\t"   // Column_name
                                    << row.get<std::string>(10) << "\n"; // Index_type
                            }
                            return CommandResult{ true, output.str(), "" };
                        }
                        catch (const std::exception& e) {
                            return CommandResult{ false, "", "SHOW INDEX error: " + std::string(e.what()) };
                        }
                    }

                    if (lowerCmd.find("select") == 0 ||
                        lowerCmd.find("show") == 0 ||
                        lowerCmd.find("describe") == 0
                        ) {
                        // 查询语句
                        soci::rowset<soci::row> rows = (session.prepare << cmd.normalized);
                        std::stringstream output;
                        AYSqlConnection::printQueryResult(rows, output);
                        return CommandResult{ true, output.str(), "" };
                    }
                    else {
                        // DML/DDL 语句
                        session << cmd.normalized;
                        return CommandResult{ true, "Query executed successfully", "" };
                    }
                }
                catch (const std::exception& e) {
                    std::string errorMsg = e.what();
                    // 检查特定的 MySQL 错误
                    if (errorMsg.find("Lost connection") != std::string::npos) {
                        return CommandResult{ false, "",
                            "MySQL connection lost. Please check:\n"
                            "1. MySQL server is running\n"
                            "2. Network connectivity\n"
                            "3. wait_timeout and interactive_timeout settings\n"
                            "4. Max allowed packet size" };
                    }

                    return CommandResult{ false, "", "MySQL Error: " + errorMsg };
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
            std::cout << "PostgreSQL connection: \n\t"
                << config.connectionString << std::endl;
            std::cout << "Postgres is not completed, do not support meta command\n";
            sqlPool.initialize(config);

            parser = cmdInterface.loadConfig("assets/core/config/SQL/postgres_ui.json");
            cmdInterface.setExecution([&sqlPool, &config](ParsedCommand& cmd) {
                try {
                    auto conn = sqlPool.getConnection();
                    auto& session = conn->session();
                    if (!conn->isConnected())
                    {
                        std::cout << "mysql reconnecting ...\n";
                        conn->connect();
                    }
                    // 特殊处理PostgreSQL的元命令
                    if (cmd.commandName[0] == '\\') {
                        return handlePostgreSQLMetaCommand(session, cmd);
                    }

                    // 判断语句类型
                    std::string upperCmd = cmd.normalized;
                    std::transform(upperCmd.begin(), upperCmd.end(), upperCmd.begin(), ::toupper);

                    bool isQuery = (upperCmd.find("SELECT") == 0) ||
                        (upperCmd.find("SHOW") == 0) ||
                        (upperCmd.find("WITH") == 0);

                    bool isDDL = (upperCmd.find("CREATE") == 0) ||
                        (upperCmd.find("DROP") == 0) ||
                        (upperCmd.find("ALTER") == 0) ||
                        (upperCmd.find("TRUNCATE") == 0);

                    if (isQuery) {
                        // 处理查询语句
                        soci::rowset<soci::row> rows = (session.prepare << cmd.normalized);
                        std::stringstream output;
                        AYSqlConnection::printQueryResult(rows, output);
                        return CommandResult{ true, output.str(), "" };
                    }
                    else {
                        // 处理 DML 和 DDL 语句
                        soci::statement st = (session.prepare << cmd.normalized);
                        st.execute(true);

                        // 尝试获取影响行数（仅对 DML 有效）
                        int affected = 0;
                        try {
                            affected = st.get_affected_rows();
                        }
                        catch (...) {
                            // DDL 语句没有影响行数
                        }

                        std::string msg = "Query executed successfully";
                        if (affected > 0) {
                            msg += ". Affected rows: " + std::to_string(affected);
                        }
                        return CommandResult{ true, msg, "" };
                    }
                }
                catch (const std::exception& e) {
                    return CommandResult{ false, "", "PostgreSQL Error: " + std::string(e.what()) };
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
