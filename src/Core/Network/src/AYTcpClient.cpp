#include "AYTcpClient.h"
#include "AYNetworkManager.h"
#include "AYNetworkHandler.h"

Network::AYTcpClient::AYTcpClient()
{
	auto handler = AYNetworkHandler(nullptr);
	_session = AYTcpSession::create(
		AYNetworkManager::getInstance()._io_context,
		handler
	);
}

void Network::AYTcpClient::connectServer(const std::string& ip_str, port_id port)
{
	_session->connect(ip_str, port);
}

void Network::AYTcpClient::send(const AYPacket& packet)
{
	_session->send(packet);
}

void Network::AYTcpClient::close()
{
	_session->close();
}