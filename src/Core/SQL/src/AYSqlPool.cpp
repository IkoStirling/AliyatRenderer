#include "AYSqlPool.h"
#include <chrono>

void AYSqlPool::initialize(const AYSqlConfig& config, size_t poolSize) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_currentSize > 0) {
        throw AYSqlException("Pool already initialized");
    }

    _config = config;
    _maxSize = poolSize;
    _shutdown = false;

    // ������ʼ����
    for (size_t i = 0; i < poolSize; ++i) {
        if (!createNewConnection()) {
            throw AYSqlException("Failed to create initial connections");
        }
    }
}

void AYSqlPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _shutdown = true;
    }

    _condition.notify_all();

    std::lock_guard<std::mutex> lock(_mutex);
    while (!_pool.empty()) {
        _pool.pop();
    }
    _currentSize = 0;
}

AYSqlPool::~AYSqlPool() {
    shutdown();
}

std::unique_ptr<AYSqlConnection> AYSqlPool::getConnectionImpl(int timeoutMs) {
    std::unique_lock<std::mutex> lock(_mutex);

    auto waitPredicate = [this]() {
        return !_pool.empty() || _shutdown;
        };

    if (timeoutMs > 0) {
        // ����ʱ�ȴ�
        if (!_condition.wait_for(lock, std::chrono::milliseconds(timeoutMs), waitPredicate)) {
            throw AYSqlException("Timeout waiting for database connection");
        }
    }
    else {
        // ���޵ȴ�
        _condition.wait(lock, waitPredicate);
    }

    if (_shutdown) {
        throw AYSqlException("Database pool is shutdown");
    }

    if (_pool.empty()) {
        // ���Դ���������
        if (!createNewConnection()) {
            throw AYSqlException("Failed to create new connection");
        }
    }

    auto conn = std::move(_pool.front());
    _pool.pop();

    // ��������Ƿ���Ч
    if (!conn->isConnected()) {
        try {
            // ��������
            *conn = AYSqlConnection(_config);
        }
        catch (const std::exception& e) {
            _currentSize--;
            throw AYSqlException(std::string("Connection is invalid and reconnect failed: ") + e.what());
        }
    }

    return conn;
}

void AYSqlPool::returnConnection(std::unique_ptr<AYSqlConnection> conn) {
    if (!conn) return;

    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_shutdown) {
            return; // ���ѹرգ�ֱ�Ӷ�������
        }

        // ��������Ƿ񻹿���
        if (!conn->isConnected()) {
            _currentSize--;
            return;
        }

        // ������ӻ��������У��ع�
        if (conn->inTransaction()) {
            try {
                conn->rollback();
            }
            catch (...) {
                // �ع�ʧ�ܣ���������
                _currentSize--;
                return;
            }
        }

        _pool.push(std::move(conn));
    }

    _condition.notify_one();
}

bool AYSqlPool::createNewConnection() {
    if (_currentSize >= _maxSize) {
        return false;
    }

    try {
        auto conn = std::make_unique<AYSqlConnection>(_config);
        _pool.push(std::move(conn));
        _currentSize++;
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

size_t AYSqlPool::availableCount() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _pool.size();
}

size_t AYSqlPool::totalCount() const {
    return _currentSize.load();
}