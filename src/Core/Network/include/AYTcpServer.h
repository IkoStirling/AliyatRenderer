#pragma once
#include "ECNetworkDependence.h"
#include "IAYBaseServer.h"
#include "IAYBaseSession.h"
#include <unordered_set>

namespace Network
{
	class AYTcpServer : public IAYBaseServer
	{
	public:
		using base_pointer = boost::shared_ptr<IAYBaseSession>;

	public:
		AYTcpServer(asio::io_context& io_context);

		~AYTcpServer();

		void start(port_id port) override;

		void stop() override;

		void broadcast(const AYPacket& packet);


	private:
		void _registerSession(base_pointer session);
		void _doAccept();

		asio::ip::tcp::acceptor _acceptor;
		std::unordered_set<base_pointer> _sessions;
		std::mutex _sessionMutex;
	};
}