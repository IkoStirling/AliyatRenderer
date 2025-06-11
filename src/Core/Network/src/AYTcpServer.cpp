#include "AYTcpServer.h"
#include "AYTcpSession.h"
#include "AYNetworkHandler.h"

Network::AYTcpServer::AYTcpServer(asio::io_context& io_context):
	IAYBaseServer(io_context),
	_acceptor(io_context)
{
}

Network::AYTcpServer::~AYTcpServer()
{
}

void Network::AYTcpServer::start(port_id port)
{
	tcp::endpoint ep(tcp::v4(), port);
	_acceptor.open(ep.protocol());
	_acceptor.set_option(tcp::acceptor::reuse_address(true));
	_acceptor.bind(ep);
	_acceptor.listen();

	_doAccept();
}

void Network::AYTcpServer::stop()
{
	_acceptor.close();

	for (auto& session : _sessions) {
		if (session)
			session->close();
	}
}

void Network::AYTcpServer::_doAccept()
{
	std::cout << "�ȴ�������\r\n";
	auto handler = AYNetworkHandler(this);
	auto newSession = AYTcpSession::create(_io_context, handler);

	_acceptor.async_accept(newSession->getSocket(),
		[this, newSession](const boost::system::error_code& error) {
			if (!error)
			{
				std::cout << "������+1\r\n";
				_registerSession(newSession);
				newSession->start();
			}
			else
			{
				std::cerr << "Accept error: " << error.message() << std::endl;
			}
			_doAccept();  // �ȴ���һ��
		});
}

void Network::AYTcpServer::broadcast(const AYPacket& packet)
{
	for (auto& session : _sessions) {
		if(session)
			session->send(packet); 
	}
}

void Network::AYTcpServer::send(base_pointer session, const AYPacket& packet)
{
	std::lock_guard<std::mutex> lock(_sessionMutex);
	auto it = _sessions.find(session);
	if (it != _sessions.end())
		if (session)
			session->send(packet);

}

void Network::AYTcpServer::_registerSession(base_pointer session)
{
	std::lock_guard<std::mutex> lock(_sessionMutex);
	_sessions.insert(boost::static_pointer_cast<IAYBaseSession>(session));
}

void Network::AYTcpServer::_removeSession(base_pointer session)
{
	std::lock_guard<std::mutex> lock(_sessionMutex);
	_sessions.erase(boost::static_pointer_cast<IAYBaseSession>(session));
}