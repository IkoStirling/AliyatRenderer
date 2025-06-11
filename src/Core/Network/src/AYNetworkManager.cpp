#include "AYNetworkManager.h"
#include "AYProtocolRouter.h"
#include "IAYBaseServer.h"

namespace Network
{
	AYNetworkManager& AYNetworkManager::getInstance()
	{
		static AYNetworkManager mInstance;
		return mInstance;
	}

	void AYNetworkManager::start()
	{
		for (int i = 0; i < thread_num; ++i)
			_netThreads.emplace_back([this]() {
				_io_context.run();
			});
	}

	void AYNetworkManager::startServer(const std::string& protocol, port_id port)
	{
		_router->startServer(protocol, port);
	}

	void AYNetworkManager::broadcast(const std::string& protocol, const AYPacket& packet)
	{
		auto server = _router->getServer(protocol);
		if (server)
			server->broadcast(packet);
	}

	void AYNetworkManager::stop()
	{
		std::cout << "AYNetworkManager stop \n";
		_workGuard.reset();
		_io_context.stop();
		for (auto& t : _netThreads)
			t.join();
	}

	AYNetworkManager::AYNetworkManager():
		_workGuard(boost::asio::make_work_guard(_io_context))
	{
		_router = std::make_unique<AYProtocolRouter>(_io_context);
	}
}