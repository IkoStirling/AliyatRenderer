#pragma once
#include <iostream>
#include <boost/asio.hpp>

namespace Network
{
#pragma pack(push, 1)
	//
	struct STPacketHeader {
		uint16_t packetId;
		uint16_t payloadSize;
		uint32_t checksum;
	public:
		void hostToNetwork()
		{
			packetId = htons(packetId);
			payloadSize = htons(payloadSize);
			checksum = htonl(checksum);
		}
		void networkToHost()
		{
			packetId = ntohs(packetId);
			payloadSize = ntohs(payloadSize);
			checksum = ntohl(checksum);
		}
	};
#pragma pack(pop)
}