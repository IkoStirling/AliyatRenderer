#pragma once
#include "AYSqlConnection.h"
#include <queue>
#include <mutex>
#include <condition_variable>
namespace ayt::engine::sql
{
    class AYSqlPool {
    public:
        static AYSqlPool& getInstance() {
            static AYSqlPool instance;
            return instance;
        }

        AYSqlPool(const AYSqlPool&) = delete;
        AYSqlPool& operator=(const AYSqlPool&) = delete;

        void initialize(const AYSqlConfig& config, size_t poolSize = 5);
        void shutdown();

        // 获取连接（RAII方式）
        class ConnectionGuard {
        public:
            ConnectionGuard(AYSqlPool& pool, std::unique_ptr<AYSqlConnection> conn)
                : _pool(pool), _connection(std::move(conn)) {
            }

            ~ConnectionGuard() {
                if (_connection) {
                    _pool.returnConnection(std::move(_connection));
                }
            }

            // 修改位置：完善智能指针接口
            AYSqlConnection& operator*() { return *_connection; }
            AYSqlConnection* operator->() { return _connection.get(); }
            AYSqlConnection* get() { return _connection.get(); }

            // 禁止拷贝，允许移动
            ConnectionGuard(const ConnectionGuard&) = delete;
            ConnectionGuard& operator=(const ConnectionGuard&) = delete;
            ConnectionGuard(ConnectionGuard&&) = default;
            ConnectionGuard& operator=(ConnectionGuard&&) = default;

        private:
            AYSqlPool& _pool;
            std::unique_ptr<AYSqlConnection> _connection;
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
        AYSqlPool() = default;
        ~AYSqlPool();

        std::unique_ptr<AYSqlConnection> getConnectionImpl(int timeoutMs = -1);
        void returnConnection(std::unique_ptr<AYSqlConnection> conn);
        bool createNewConnection();

        AYSqlConfig _config;
        std::queue<std::unique_ptr<AYSqlConnection>> _pool;
        mutable std::mutex _mutex;
        std::condition_variable _condition;
        size_t _maxSize = 0;
        std::atomic<size_t> _currentSize{ 0 };
        std::atomic<bool> _shutdown{ false };
    };
}