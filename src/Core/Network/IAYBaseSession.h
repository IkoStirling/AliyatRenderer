#pragma once
#include "ECNetworkDependence.h"

namespace Network
{
	class IAYBaseSession
	{
	public:
		virtual void start(MessageHandler handler) = 0;
		virtual void send(AYMessageType type, std::vector<uint8_t>& data, ResponseHandler onResponse = nullptr) = 0;
		virtual void close() = 0;
		virtual ~IAYBaseSession() = default;
		virtual std::string remoteAddress() const = 0;
		virtual boost::uuids::uuid getSessionID() const = 0;
	};
}