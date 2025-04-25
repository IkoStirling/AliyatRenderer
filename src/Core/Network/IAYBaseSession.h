#pragma once
#include "ECNetworkDependence.h"

namespace Network
{
	class IAYBaseSession
	{
	public:
		virtual void start() = 0;
		virtual void send(const STNetworkMessage& msg) = 0;
		virtual void close() = 0;
		virtual ~IAYBaseSession() = default;
		virtual std::wstring remoteAddress() const = 0;
	};
}