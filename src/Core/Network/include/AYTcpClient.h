#pragma once
#include "IAYBaseClient.h"
#include "AYTcpSession.h"

namespace Network
{
	class AYTcpClient : public IAYBaseClient
	{
	public:
		AYTcpClient();
		void connectServer(const std::string& ip_str, port_id port) override;
		void send(const AYPacket& packet) override;
		void close();
		
	private:
		AYTcpSession::pointer _session;
	};
}