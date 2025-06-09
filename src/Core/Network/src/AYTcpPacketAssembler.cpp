#include "AYTcpPacketAssembler.h"

Network::AYTcpPacketAssembler::AYTcpPacketAssembler(PacketCallback callback):
    _callback(callback)
{
}

void Network::AYTcpPacketAssembler::feedData(const uint8_t* data, size_t length)
{
    // 计算实际可用空间
    const size_t availableSpace = _buffer.size() - (_startPos + (_buffer.size() - _startPos));

    // 需要扩容时
    if (length > availableSpace) {
        // 移动有效数据到头部
        if (_startPos > 0) {
            std::memmove(_buffer.data(), _buffer.data() + _startPos, _buffer.size() - _startPos);
            _buffer.resize(_buffer.size() - _startPos);
            _startPos = 0;
        }
        // 扩容缓冲区
        _buffer.reserve(_buffer.size() + length * 2);
    }

    // 添加新数据
    _buffer.insert(_buffer.end(), data, data + length);
    _processBuffer();
}

void Network::AYTcpPacketAssembler::reset()
{
    _buffer.clear();
    _expectedSize = 0;
    _processedPos = 0;
    _inHeader = true;
}

void Network::AYTcpPacketAssembler::_processBuffer()
{
    while (true) {
        if (_inHeader) {
            if (!_tryParseHeader()) break;
        }

        if (!_inHeader) {
            if (!_tryParseBody()) break;
        }
    }
}

bool Network::AYTcpPacketAssembler::_tryParseHeader()
{
    size_t available = _buffer.size() - _processedPos;
    if (available < HEADER_SIZE) return false;

    // 解析头部
    STPacketHeader header;
    memcpy(&header, _buffer.data() + _processedPos, HEADER_SIZE);

    // 验证头部有效性
    if (header.payloadSize > MAX_PACKET_SIZE) {
        throw std::runtime_error("Invalid packet size");
    }

    // 准备解析消息体
    _expectedSize = header.payloadSize;
    _processedPos += HEADER_SIZE;
    _inHeader = false;

    std::cout << "头部解析完全\r\n";
    return true;
}

bool Network::AYTcpPacketAssembler::_tryParseBody()
{
    const size_t available = _buffer.size() - _startPos - HEADER_SIZE;
    if (available < _expectedSize) return false;

    // 使用数据视图避免拷贝
    AYPacket packet;
    const uint8_t* headerStart = _buffer.data() + _startPos;
    const uint8_t* payloadStart = headerStart + HEADER_SIZE;

    // 直接引用缓冲区数据
    std::memcpy(&packet.header, headerStart, HEADER_SIZE);
    packet.payload.assign(payloadStart, payloadStart + _expectedSize);  // 避免额外拷贝

    if (packet.isValid()) {
        _callback(packet);
    }
    std::cout << "包体解析完全\r\n";

    // 更新位置
    _startPos += HEADER_SIZE + _expectedSize;
    _inHeader = true;

    // 回收内存：当已处理数据超过阈值时
    if (_startPos > 4096) {
        std::memmove(_buffer.data(), _buffer.data() + _startPos, _buffer.size() - _startPos);
        _buffer.resize(_buffer.size() - _startPos);
        _startPos = 0;
    }

    return true;
}