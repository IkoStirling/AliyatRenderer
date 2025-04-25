#pragma once
#include "ECNetworkDependence.h"
#include "IAYBaseSession.h"

namespace Network
{
	class AYNetworkManager
	{
	public:
		static AYNetworkManager& getInstance();

		void startTcpServer(unsigned short port);
		void connectTcpServer(const std::string& host, unsigned short port);

		void registerSession(std::shared_ptr<IAYBaseSession> session);
		void removeSession(std::shared_ptr<IAYBaseSession> session);

	private:
		asio::io_context _io_context;
		std::unordered_map<int, std::shared_ptr<IAYBaseSession>> _sessions;
		std::thread _netThread;
		int _sessionIdCounter;

		void _run();
	};
}