#include "AYTcpClient.h"
#include "AYNetworkManager.h"
#include "AYNetworkHandler.h"

Network::AYTcpClient::AYTcpClient(boost::asio::io_context& io_context) :
	IAYBaseClient(io_context)
{

}

void Network::AYTcpClient::connectServer(const std::string& ip_str, port_id port)
{
	if (_session)
		_session->connect(ip_str, port);
}

void Network::AYTcpClient::send(const AYPacket& packet)
{
	if (_session)
		_session->send(packet);
}

void Network::AYTcpClient::close()
{
	if (_session)
		_session->close();
}

void Network::AYTcpClient::start()
{
	auto handler = AYNetworkHandler(nullptr);
	_session = AYTcpSession::create(
		_io_context,
		handler
	);
}