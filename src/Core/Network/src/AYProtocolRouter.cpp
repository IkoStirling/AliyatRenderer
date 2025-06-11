#include "AYProtocolRouter.h"
#include "AYTcpServer.h"

Network::AYProtocolRouter::AYProtocolRouter(asio::io_context& io_context):
	_io_context(io_context)
{

}

Network::AYProtocolRouter::~AYProtocolRouter()
{
}

void Network::AYProtocolRouter::startServer(const std::string& protocol, port_id port)
{
	if (protocol == "TCP")
	{
		std::cout << "TCP 服务已启用，正在监听客户端连接… \r\n";
		auto _tcpServer = std::make_shared<AYTcpServer>(_io_context);
		_servers[protocol] = _tcpServer;
		_tcpServer->start(port);
	}
	else if (protocol == "UDP")
	{

	}
	else
	{

	}
}

std::shared_ptr<Network::IAYBaseServer>  Network::AYProtocolRouter::getServer(const std::string& protocol)
{
	return _servers[protocol];
}