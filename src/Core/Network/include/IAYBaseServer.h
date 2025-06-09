#pragma once
#include "ECNetworkDependence.h"

namespace Network
{
	class IAYBaseServer
	{
	public:
		IAYBaseServer(asio::io_context& io_context) : _io_context(io_context) {}
		virtual ~IAYBaseServer() = default;
		virtual void start(port_id port) = 0;
		virtual void stop() = 0;

	protected:
		asio::io_context& _io_context;
	};
}