#pragma once
#include "ECNetworkDependence.h"

namespace Network
{
	class AYTcpPacketAssembler
	{
	public:
		using PacketCallback = std::function<void(const AYPacket&)>;
	private:
		static constexpr size_t HEADER_SIZE = sizeof(STPacketHeader);
		static constexpr size_t MAX_PACKET_SIZE = 65536;

	public:
		AYTcpPacketAssembler(PacketCallback callback);
		void feedData(const uint8_t* data, size_t length);
		void reset();

	private:
		void _processBuffer();
		bool _tryParseHeader();
		bool _tryParseBody();

	private:
		PacketCallback _callback;

		std::vector<uint8_t> _buffer;
		size_t _startPos = 0;
		size_t _expectedSize = 0;
		size_t _processedPos = 0;
		bool _inHeader = true;

		
	};
}