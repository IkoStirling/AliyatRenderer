#pragma once
#include  "Tcp/AYTcpServer.h"

namespace Network {
	class AYNetworkHandler
	{
	public:
		AYNetworkHandler(AYTcpServer* server) :
			_server(server)
		{

		}
		void onPacketReceived(AYTcpSession* session, const AYPacket& packet) {
			std::string rstr(packet.payload.begin(), packet.payload.end());
			std::cout << rstr << std::endl;
		}
		void onConnectError(const std::string& error_msg) { std::cout << error_msg; }
		void onSessionClosed(AYTcpSession* session) { std::cout << "session closed\n"; }
	private:
		AYTcpServer* _server;
	};
}
