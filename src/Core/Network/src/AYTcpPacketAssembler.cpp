#include "AYTcpPacketAssembler.h"

Network::AYTcpPacketAssembler::AYTcpPacketAssembler(PacketCallback callback) :
	_callback(callback)
{
}

void Network::AYTcpPacketAssembler::feedData(const uint8_t* data, size_t length)
{
	std::cout << "传入数据大小（byte）：" << length << std::endl;
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
	while (_startPos + (_inHeader ? HEADER_SIZE : _expectedSize) <= _buffer.size()) {
		if (_inHeader) {
			if (!_tryParseHeader()) break;
		}
		else {
			if (!_tryParseBody()) break;
		}
	}

	// 自动回收已处理的内存（阈值可调整）
	if (_startPos > 4096) {
		_buffer.erase(_buffer.begin(), _buffer.begin() + _startPos);
		_startPos = 0;
	}
}

bool Network::AYTcpPacketAssembler::_tryParseHeader()
{
	if (_buffer.size() - _startPos < HEADER_SIZE) return false;

	memcpy(&_headerBuffer, _buffer.data() + _startPos, HEADER_SIZE);
	_headerBuffer.networkToHost();

	if (_headerBuffer.payloadSize > MAX_PACKET_SIZE) {
		throw std::runtime_error("Invalid packet size");
	}

	_expectedSize = _headerBuffer.payloadSize;
	_inHeader = false;

	std::cout << "头部解析完全:\r\n";
	std::cout << "\t header.checksum: " << _headerBuffer.checksum <<
		"\n\t header.packetId: " << _headerBuffer.packetId << "\n\t header.payloadSize: " << _headerBuffer.payloadSize << std::endl;
	return true;
}

bool Network::AYTcpPacketAssembler::_tryParseBody()
{
	if (_buffer.size() - _startPos - HEADER_SIZE < _expectedSize) return false;

	AYPacket packet;
	packet.header = _headerBuffer;
	packet.payload.assign(
		_buffer.begin() + _startPos + HEADER_SIZE,
		_buffer.begin() + _startPos + HEADER_SIZE + _expectedSize
	);

	if (packet.isValid()) {
		_callback(packet);
	}
	std::cout << "包体解析完全\r\n";
	printBytes(&packet.header, sizeof(STPacketHeader));
	printBytes(packet.payload.data(), packet.payload.size());

	// 更新位置
	_startPos += HEADER_SIZE + _expectedSize;
	_inHeader = true;
	_headerBuffer = {};

	return true;
}