#pragma once
#include "ECNetworkDependence.h"
#include "IAYBaseSession.h"

namespace Network
{
	class IAYBaseServer
	{
	public:
		using base_pointer = boost::shared_ptr<IAYBaseSession>;

	public:
		IAYBaseServer(asio::io_context& io_context) : _io_context(io_context) {}
		virtual ~IAYBaseServer() = default;
		virtual void start(port_id port) = 0;
		virtual void stop() = 0;
		virtual void broadcast(const AYPacket& packet) = 0;
		virtual void send(base_pointer session, const AYPacket& packet) = 0;

	protected:
		asio::io_context& _io_context;
	};
}