#pragma once
#include "ECNetworkDependence.h"

namespace Network
{
	class IAYBaseClient
	{
	public:
		virtual void connectServer(const std::string& ip_str, port_id port) = 0;
		virtual void send(const AYPacket& packet) = 0;

	};
}