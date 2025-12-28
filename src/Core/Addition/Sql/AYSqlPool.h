#pragma once
#include "AYSqlConnection.h"
#include <queue>
#include <mutex>
#include <condition_variable>
namespace ayt::engine::sql
{
    class SqlPool {
    public:
        static SqlPool& getInstance() {
            static SqlPool instance;
            return instance;
        }

        SqlPool(const SqlPool&) = delete;
        SqlPool& operator=(const SqlPool&) = delete;

        void initialize(const SqlConfig& config, size_t poolSize = 5);
        void shutdown();

        // 获取连接（RAII方式）
        class ConnectionGuard {
        public:
            ConnectionGuard(SqlPool& pool, std::unique_ptr<SqlConnection> conn)
                : _pool(pool), _connection(std::move(conn)) {
            }

            ~ConnectionGuard() {
                if (_connection) {
                    _pool.returnConnection(std::move(_connection));
                }
            }

            // 修改位置：完善智能指针接口
            SqlConnection& operator*() { return *_connection; }
            SqlConnection* operator->() { return _connection.get(); }
            SqlConnection* get() { return _connection.get(); }

            // 禁止拷贝，允许移动
            ConnectionGuard(const ConnectionGuard&) = delete;
            ConnectionGuard& operator=(const ConnectionGuard&) = delete;
            ConnectionGuard(ConnectionGuard&&) = default;
            ConnectionGuard& operator=(ConnectionGuard&&) = default;

        private:
            SqlPool& _pool;
            std::unique_ptr<SqlConnection> _connection;
        };

        ConnectionGuard getConnection() {
            return ConnectionGuard(*this, getConnectionImpl());
        }

        // 修改位置：增加带超时的连接获取
        ConnectionGuard getConnectionWithTimeout(int timeoutMs) {
            return ConnectionGuard(*this, getConnectionImpl(timeoutMs));
        }

        size_t availableCount() const;
        size_t totalCount() const;

    private:
        SqlPool() = default;
        ~SqlPool();

        std::unique_ptr<SqlConnection> getConnectionImpl(int timeoutMs = -1);
        void returnConnection(std::unique_ptr<SqlConnection> conn);
        bool createNewConnection();

        SqlConfig _config;
        std::queue<std::unique_ptr<SqlConnection>> _pool;
        mutable std::mutex _mutex;
        std::condition_variable _condition;
        size_t _maxSize = 0;
        std::atomic<size_t> _currentSize{ 0 };
        std::atomic<bool> _shutdown{ false };
    };
}