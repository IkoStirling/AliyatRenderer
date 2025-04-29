#pragma once
#include "ECNetworkDependence.h"
#include "IAYBaseSession.h"

namespace Network
{
	class AYNetworkManager
	{
	public:
		using port_id = unsigned short;
		using base_pointer = boost::shared_ptr<IAYBaseSession>;
	public:
		static AYNetworkManager& getInstance();

	//TCP Session -----------------------------------------------------------
	public:
		void startTcpServer(port_id port, int thread_num);
		void connectTcpServer(const std::string& host, port_id port){}

	private:
		void _startTcpAccept();

	//Common ----------------------------------------------------------------
	public:
		void stop();

		void registerSession(base_pointer session);
		void removeSession(base_pointer session){}

		void broadcast(AYMessageType type, const std::vector<uint8_t>& data){}
		void sendTo(const boost::uuids::uuid& sessionID, AYMessageType type, const std::vector<uint8_t>& data){}
	private:
		AYNetworkManager();
		void _handleMessage(const STNetworkMessage& msg){}
		void _run(){}
	private:
		asio::io_context _io_context;
		tcp::acceptor _acceptor;

		std::unordered_map<boost::uuids::uuid, base_pointer, boost::hash<boost::uuids::uuid>> _sessions;
		std::mutex _sessionMutex;

		std::vector<std::thread> _netThreads;
		int _sessionCounter;

	};
}
