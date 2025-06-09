#pragma once
#include "ECNetworkDependence.h"
#include "IAYBaseSession.h"

namespace Network
{
	class AYProtocolRouter;
	class AYTcpServer;

	class AYNetworkManager
	{
	public:
		using base_pointer = boost::shared_ptr<IAYBaseSession>;
	public:
		static AYNetworkManager& getInstance();

	public:
		void start();
		void stop();

		void startServer(const std::string& protocol, port_id port);

		asio::io_context _io_context;
	private:
		std::unique_ptr<AYProtocolRouter> _router;

	//Common ----------------------------------------------------------------
	public:

	private:
		AYNetworkManager();
	private:
		std::vector<std::thread> _netThreads;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> _workGuard;
		int thread_num = 1;
	};
}
