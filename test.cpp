#include "AYEngineCore.h"
#include "AYTimerManager.h"
#include "AYTcpSession.h"
#include "AYNetworkManager.h"
#include <iostream>
#include <thread>

int main()
{
	std::cout << "now begin;\r\n";

	/*AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();*/

	int c;
	std::cout << "输入整数（1服务器，2客户端）\n";
	while (std::cin >> c)
	{
		if (c == 1)
		{
			std::cout << "服务器\n";
			Network::AYNetworkManager::getInstance().startServer("TCP", 59383);
			Network::AYNetworkManager::getInstance().start();
			auto x =getchar();
			x = getchar();
			break;
		}
		else if (c == 2)
		{
			std::cout << "客户端\n";

			Network::AYNetworkManager::getInstance().start();
			auto handler = Network::AYNetworkHandler();
			auto session = Network::AYTcpSession::create(Network::AYNetworkManager::getInstance()._io_context, handler);
			session->connect("127.0.0.1", 59383);

			std::cout << "输入整数:\n\t1)发送信息 2）接收信息 3）退出\n";
			int x;
			while (std::cin >> x)
			{
				if (x == 1)
				{
					Network::AYPacket packet;
					packet.header.packetId = 0x1234;
					packet.header.checksum = 0xABCDEF12;
					std::cout << "输入发送信息:\n";
					std::string str;
					std::cin >> str;
					packet.header.payloadSize = str.size();
					packet.payload.assign(str.begin(),str.end());
					session->send(packet);
				}
				else if (x == 2)
				{

				}
				else if (x == 3)
				{
					session->close();
					break;
				}
				std::cout << "输入整数:\n\t1)发送信息 2）接收信息 3）退出\n";
			}
		}
	}

	Network::AYNetworkManager::getInstance().stop();
	getchar();
	return 0;
}