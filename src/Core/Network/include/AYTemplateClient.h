#include "ECNetworkDependence.h"

namespace Network {
	class AYTemplateClient
	{
	public:
		void connectServer(const std::string& host, unsigned short port);
		void sendMessage(const STNetworkMessage& msg);
	};
}