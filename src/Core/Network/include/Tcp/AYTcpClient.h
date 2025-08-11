#pragma once
#include "Base/IAYBaseClient.h"
#include "AYTcpSession.h"

namespace Network
{
	class AYTcpClient : public IAYBaseClient
	{
	public:
		AYTcpClient(boost::asio::io_context& io_context);
		void start();
		void close();
		void send(const AYPacket& packet) override;
		void connectServer(const std::string& ip_str, port_id port) override;

	private:
		AYTcpSession::pointer _session;
	};
}