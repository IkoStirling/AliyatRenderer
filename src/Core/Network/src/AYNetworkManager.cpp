#include "AYNetworkManager.h"
#include "AYTcpSession.h"

namespace Network
{
	AYNetworkManager& AYNetworkManager::getInstance()
	{
		static AYNetworkManager mInstance;
		return mInstance;
	}

	void AYNetworkManager::startTcpServer(port_id port, int thread_num)
	{
		//ep代表对端
		tcp::endpoint ep(tcp::v4(), port);
		_acceptor.open(ep.protocol());
		_acceptor.set_option(tcp::acceptor::reuse_address(true));
		_acceptor.bind(ep);
		_acceptor.listen();

		_startTcpAccept();

		for (int i = 0; i < thread_num; ++i)
			_netThreads.emplace_back([this]() { _io_context.run(); });
	}

	void AYNetworkManager::_startTcpAccept()
	{
		auto newSession = AYTcpSession::create(_io_context);

		_acceptor.async_accept(newSession->getSocket(),
			[this, newSession](const boost::system::error_code& error) {
				if (!error)
				{
					registerSession(newSession);
					newSession->start([this](const STNetworkMessage& msg) {
						_handleMessage(msg);
						});
				}
				_startTcpAccept();  // 等待下一个
			});
	}

	void AYNetworkManager::stop()
	{
		_io_context.stop();
		for (auto& t : _netThreads)
			t.join();
	}

	void AYNetworkManager::registerSession(base_pointer session)
	{
		std::lock_guard<std::mutex> lock(_sessionMutex);
		_sessions[session->getSessionID()] = boost::static_pointer_cast<IAYBaseSession>(session);
	}

	AYNetworkManager::AYNetworkManager():
		_acceptor(_io_context)
	{

	}
}