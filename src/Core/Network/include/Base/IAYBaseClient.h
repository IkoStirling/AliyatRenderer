#pragma once
#include "ECNetworkDependence.h"
namespace Network
{
	class IAYBaseClient
	{
	public:
		IAYBaseClient(boost::asio::io_context& io_context) : _io_context(io_context) {}
		virtual void connectServer(const std::string& ip_str, port_id port) = 0;
		virtual void send(const AYPacket& packet) = 0;

	protected:
		boost::asio::io_context& _io_context;
	};
}