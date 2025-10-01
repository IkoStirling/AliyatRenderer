#pragma once
#include "AYSqlConfig.h"
#include <soci/soci.h>
#include <memory>
#include <stdexcept>
#include <regex>

// 自定义异常类
class AYSqlException : public std::runtime_error {
public:
    explicit AYSqlException(const std::string& msg) : std::runtime_error(msg) {}
};

class AYSqlConnection {
public:
    // 构造函数 - 根据配置创建连接
    explicit AYSqlConnection(const AYSqlConfig& config);

    // 析构函数 - 自动关闭连接
    ~AYSqlConnection();

    // 禁止拷贝
    AYSqlConnection(const AYSqlConnection&) = delete;
    AYSqlConnection& operator=(const AYSqlConnection&) = delete;

    // 允许移动
    AYSqlConnection(AYSqlConnection&&) noexcept = default;
    AYSqlConnection& operator=(AYSqlConnection&&) noexcept;

    soci::session& session() {
        if (!_session || !_session->is_connected()) {
            throw AYSqlException("AYSql connection is not available");
        }
        return *_session;
    }

    const soci::session& session() const {
        if (!_session || !_session->is_connected()) {
            throw AYSqlException("AYSql connection is not available");
        }
        return *_session;
    }

    // 连接状态检查
    bool isConnected() const {
        return _session && _session->is_connected();
    }

    // 事务管理
    void begin();
    void commit();
    void rollback();
    bool inTransaction() const { return _inTransaction; }

    void execute(const std::string& query) {
        session() << query;
    }

    template<typename... Args>
    void execute(const std::string& query, Args&&... args) {
        std::string rquery = replaceNamedParams(query);
        auto& sess = session();

        soci::statement st = (sess.prepare << rquery);

        st.alloc();
        st.prepare(rquery);

        bindParameters(st, std::forward<Args>(args)...);

        st.define_and_bind();

        try {
            st.execute(true);
        }
        catch (const soci::soci_error& e) {
            throw AYSqlException(
                "Execute failed: " + query + "\n" +
                "Reason: " + e.what() + "\n"
            );
        }
    }

    static void printQueryResult(soci::rowset<soci::row>& rows);
    static void printQueryResult(soci::rowset<soci::row>& rows, std::ostream& output);

    template<typename T>
    T queryValue(const std::string& query) {
        T result;
        session() << query, soci::into(result);
        return result;
    }

    soci::rowset<soci::row> query(const std::string& query) {
        return soci::rowset<soci::row>((session().prepare << query));
    }

    void connect();

private:
    std::unique_ptr<soci::session> _session;
    AYSqlConfig _config;
    bool _inTransaction = false;

    template<typename T>
    void bindParameter(soci::statement& st, T&& value, int& index) {
        st.exchange(soci::use(std::forward<T>(value), std::to_string(++index)));
    }

    void bindParameter(soci::statement&, int& index) {}  // 终止条件

    template<typename Arg, typename... Args>
    void bindParameter(soci::statement& st, int& index, Arg&& arg, Args&&... args) {
        bindParameter(st, std::forward<Arg>(arg), index);
        bindParameter(st, index, std::forward<Args>(args)...);  // 递归处理剩余参数
    }

    // 多参数绑定（递归展开）
    template<typename Arg, typename... Args>
    void bindParameters(soci::statement& st, Arg&& arg, Args&&... args) {
        int index = 0;  // 每个参数组独立索引
        bindParameter(st, std::forward<Arg>(arg), index);
        bindParameter(st, index, std::forward<Args>(args)...);  // 递归处理剩余参数
    }

    std::string replaceNamedParams(const std::string& sql);

    // 实际建立连接的方法
    
    void disconnect();
};