#pragma once
#include "STPacketHeader.h"
#include <vector>

namespace Network
{
	class AYPacket {
	public:
		STPacketHeader header;
		std::vector<uint8_t> payload;

		bool isValid() const;

		void updateHeader();
	private:
		uint32_t _calculateChecksum() const;
	};
}