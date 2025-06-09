#include "AYNetworkManager.h"
#include "AYProtocolRouter.h"

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
			std::cout << "io_context begin \n";
			_io_context.run();
			std::cout << "io_context end \n";
				});
	}

	void AYNetworkManager::startServer(const std::string& protocol, port_id port)
	{
		_router->startServer(protocol, port);
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