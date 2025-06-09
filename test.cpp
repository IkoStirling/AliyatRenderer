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
	std::cout << "����������1��������2�ͻ��ˣ�\n";
	while (std::cin >> c)
	{
		if (c == 1)
		{
			std::cout << "������\n";
			Network::AYNetworkManager::getInstance().startServer("TCP", 59383);
			Network::AYNetworkManager::getInstance().start();
			auto x =getchar();
			x = getchar();
			break;
		}
		else if (c == 2)
		{
			std::cout << "�ͻ���\n";

			Network::AYNetworkManager::getInstance().start();
			auto handler = Network::AYNetworkHandler();
			auto session = Network::AYTcpSession::create(Network::AYNetworkManager::getInstance()._io_context, handler);
			session->connect("127.0.0.1", 59383);

			std::cout << "��������:\n\t1)������Ϣ 2��������Ϣ 3���˳�\n";
			int x;
			while (std::cin >> x)
			{
				if (x == 1)
				{
					Network::AYPacket packet;
					packet.header.packetId = 0x1234;
					packet.header.checksum = 0xABCDEF12;
					std::cout << "���뷢����Ϣ:\n";
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
				std::cout << "��������:\n\t1)������Ϣ 2��������Ϣ 3���˳�\n";
			}
		}
	}

	Network::AYNetworkManager::getInstance().stop();
	getchar();
	return 0;
}