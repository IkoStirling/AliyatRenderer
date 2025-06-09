#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <mutex>

namespace Network
{
	using boost::asio::ip::tcp;
	namespace asio = boost::asio;

	const size_t MAX_PACKET_SIZE = 1024 * 1024 * 10; // 10MB

    using port_id = unsigned short;

#pragma pack(push, 1)
    //
    struct STPacketHeader {
        uint16_t packetId;
        uint16_t payloadSize;
        uint32_t checksum;
    };
#pragma pack(pop)

    class AYPacket {
    public:
        STPacketHeader header;
        std::vector<uint8_t> payload;

        bool isValid() const {
            return _calculateChecksum() == header.checksum;
        }

        void updateHeader()
        {
            header.payloadSize = payload.size();
            header.checksum = _calculateChecksum();
        }
    private:
        uint32_t _calculateChecksum() const {
            uint32_t sum = 0;
            const uint8_t* data = reinterpret_cast<const uint8_t*>(&header);

            // 计算头部校验（跳过checksum字段）
            for (size_t i = 0; i < offsetof(STPacketHeader, checksum); i++) {
                sum = (sum << 5) | (sum >> 27);  // 旋转移位
                sum += data[i];
            }

            // 计算payload校验
            for (uint8_t byte : payload) {
                sum = (sum << 5) | (sum >> 27);
                sum += byte;
            }
            return sum;
        }
    };



    class ByteBuffer {
    public:
        // 构造函数
        ByteBuffer() = default;
        explicit ByteBuffer(size_t initialCapacity) : _dataBuffer(initialCapacity) {}

    private:
        std::vector<uint8_t> _dataBuffer; // 底层存储
        size_t _dataSize = 0;             // 有效数据大小
        size_t _readPos = 0;              // 读取位置
    public:
        // 基础功能
        uint8_t* data() noexcept { return _dataBuffer.data(); }
        const uint8_t* data() const noexcept { return _dataBuffer.data(); }
        size_t size() const noexcept { return _dataSize; }
        size_t capacity() const noexcept { return _dataBuffer.size(); }
        void clear() noexcept { _dataSize = 0; _readPos = 0; }

        // 扩容与调整
        void reserve(size_t capacity) {
            if (capacity > _dataBuffer.size()) {
                _dataBuffer.resize(capacity);
            }
        }
        void resize(size_t newSize) {
            if (newSize > _dataBuffer.size()) {
                _dataBuffer.resize(newSize);
            }
            _dataSize = std::min(_dataSize, newSize);
        }

        // 写入数据（支持任意类型）
        template<typename T>
        void write(const T& value) {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
                "Write requires arithmetic or enum type");

            const size_t typeSize = sizeof(T);
            ensureCapacity(_dataSize + typeSize);

            // 字节序转换
            T netValue = hostToNetwork(value);
            std::memcpy(_dataBuffer.data() + _dataSize, &netValue, typeSize);
            _dataSize += typeSize;
        }

        // 读取数据（支持任意类型）
        template<typename T>
        T read() {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
                "Read requires arithmetic or enum type");

            if (_readPos + sizeof(T) > _dataSize) {
                throw std::out_of_range("Read beyond buffer size");
            }

            T value;
            std::memcpy(&value, _dataBuffer.data() + _readPos, sizeof(T));
            _readPos += sizeof(T);

            // 字节序转换
            return networkToHost(value);
        }

        // 流式写入（支持链式调用）
        template<typename T>
        ByteBuffer& operator<<(const T& data) {
            write(data);
            return *this;
        }

        // 流式读取（支持链式调用）
        template<typename T>
        ByteBuffer& operator>>(T& data) {
            data = read<T>();
            return *this;
        }

        // 原始块操作
        void append(const uint8_t* data, size_t size) {
            ensureCapacity(_dataSize + size);
            std::memcpy(_dataBuffer.data() + _dataSize, data, size);
            _dataSize += size;
        }

        const uint8_t* readBytes(size_t size) {
            if (_readPos + size > _dataSize) {
                throw std::out_of_range("Read bytes beyond buffer size");
            }
            const uint8_t* result = _dataBuffer.data() + _readPos;
            _readPos += size;
            return result;
        }

        // 读写位置控制
        size_t getReadPosition() const noexcept { return _readPos; }
        void setReadPosition(size_t pos) {
            if (pos > _dataSize) {
                throw std::out_of_range("Invalid read position");
            }
            _readPos = pos;
        }
        size_t getWritePosition() const noexcept { return _dataSize; }
        void setWritePosition(size_t pos) {
            if (pos > _dataBuffer.size()) {
                throw std::out_of_range("Invalid write position");
            }
            _dataSize = pos;
        }

        // 字节序转换（大端序）
        template<typename T>
        T hostToNetwork(T value) {
            if constexpr (std::is_same_v<T, uint16_t>) {
                return htons(value);  // 主机序 → 网络序（大端）
            }
            else if constexpr (std::is_same_v<T, uint32_t>) {
                return htonl(value);  // 主机序 → 网络序（大端）
            }
            else {
                return value;         // 其他类型不转换（如 uint8_t）
            }
        }

        template<typename T>
        T networkToHost(T value) {
            if constexpr (std::is_same_v<T, uint16_t>) {
                return ntohs(value);  // 网络序 → 主机序（大端）
            }
            else if constexpr (std::is_same_v<T, uint32_t>) {
                return ntohl(value);  // 网络序 → 主机序（大端）
            }
            else {
                return value;         // 其他类型不转换（如 uint8_t）
            }
        }

        // 确保容量足够
        void ensureCapacity(size_t requiredCapacity) {
            if (requiredCapacity > _dataBuffer.size()) {
                // 按需扩容（类似 vector 的增长策略）
                _dataBuffer.resize(std::max(requiredCapacity, _dataBuffer.size() * 2));
            }
        }
    };



    enum class AYMessageType {
        Text,
        Binary,
        Command,
        VideoFrame
    };

    struct STNetworkMessage {
        AYMessageType type;
        boost::uuids::uuid id;
        std::vector<uint8_t> data;
    };

   
    using MessageHandler = std::function<void(const STNetworkMessage&)>;
    using ResponseHandler = std::function<void(const STNetworkMessage&)>;
}