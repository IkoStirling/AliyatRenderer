#pragma once
#include "ECNetworkDependence.h"

namespace Network
{
	class IAYBaseServer;

	class AYProtocolRouter
	{
	public:
		AYProtocolRouter(asio::io_context& io_context);
		~AYProtocolRouter();

		void startServer(const std::string& protocol, port_id port);

	private:
		asio::io_context& _io_context;
		std::unordered_map<std::string, std::shared_ptr<IAYBaseServer>> _servers;

	};
}