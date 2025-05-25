#include "ECNetworkDependence.h"

namespace Network {
	class AYTemplateServer
	{
	public:
		void start(unsigned short port);
		void broadcastMessage(const STNetworkMessage& msg);
	};
}