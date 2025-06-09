#include "AYTcpPacketAssembler.h"

Network::AYTcpPacketAssembler::AYTcpPacketAssembler(PacketCallback callback):
    _callback(callback)
{
}

void Network::AYTcpPacketAssembler::feedData(const uint8_t* data, size_t length)
{
    // ����ʵ�ʿ��ÿռ�
    const size_t availableSpace = _buffer.size() - (_startPos + (_buffer.size() - _startPos));

    // ��Ҫ����ʱ
    if (length > availableSpace) {
        // �ƶ���Ч���ݵ�ͷ��
        if (_startPos > 0) {
            std::memmove(_buffer.data(), _buffer.data() + _startPos, _buffer.size() - _startPos);
            _buffer.resize(_buffer.size() - _startPos);
            _startPos = 0;
        }
        // ���ݻ�����
        _buffer.reserve(_buffer.size() + length * 2);
    }

    // ���������
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

    // ����ͷ��
    STPacketHeader header;
    memcpy(&header, _buffer.data() + _processedPos, HEADER_SIZE);

    // ��֤ͷ����Ч��
    if (header.payloadSize > MAX_PACKET_SIZE) {
        throw std::runtime_error("Invalid packet size");
    }

    // ׼��������Ϣ��
    _expectedSize = header.payloadSize;
    _processedPos += HEADER_SIZE;
    _inHeader = false;

    std::cout << "ͷ��������ȫ\r\n";
    return true;
}

bool Network::AYTcpPacketAssembler::_tryParseBody()
{
    const size_t available = _buffer.size() - _startPos - HEADER_SIZE;
    if (available < _expectedSize) return false;

    // ʹ��������ͼ���⿽��
    AYPacket packet;
    const uint8_t* headerStart = _buffer.data() + _startPos;
    const uint8_t* payloadStart = headerStart + HEADER_SIZE;

    // ֱ�����û���������
    std::memcpy(&packet.header, headerStart, HEADER_SIZE);
    packet.payload.assign(payloadStart, payloadStart + _expectedSize);  // ������⿽��

    if (packet.isValid()) {
        _callback(packet);
    }
    std::cout << "���������ȫ\r\n";

    // ����λ��
    _startPos += HEADER_SIZE + _expectedSize;
    _inHeader = true;

    // �����ڴ棺���Ѵ������ݳ�����ֵʱ
    if (_startPos > 4096) {
        std::memmove(_buffer.data(), _buffer.data() + _startPos, _buffer.size() - _startPos);
        _buffer.resize(_buffer.size() - _startPos);
        _startPos = 0;
    }

    return true;
}