#include "AYPacket.h"

namespace Network
{
	bool AYPacket::isValid() const {
		return _calculateChecksum() == header.checksum;
	}

	void AYPacket::updateHeader()
	{
		header.payloadSize = payload.size();
		header.checksum = _calculateChecksum();
	}

	uint32_t AYPacket::_calculateChecksum() const {
		uint32_t sum = 0;
		const uint8_t* data = reinterpret_cast<const uint8_t*>(&header);

		// ����ͷ��У�飨����checksum�ֶΣ�
		for (size_t i = 0; i < offsetof(STPacketHeader, checksum); i++) {
			sum = (sum << 5) | (sum >> 27);  // ��ת��λ
			sum += data[i];
		}

		// ����payloadУ��
		for (uint8_t byte : payload) {
			sum = (sum << 5) | (sum >> 27);
			sum += byte;
		}
		return sum;
	}
}