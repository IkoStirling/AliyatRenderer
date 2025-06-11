#pragma once
#include "AYEngineCore.h"
#include "AYTimerManager.h"
#include "AYTcpSession.h"
#include "AYNetworkManager.h"
#include "AYNetworkHandler.h"
#include "AYTcpClient.h"
#include <iostream>
#include <thread>

int main()
{
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
			int y;
			while (std::cin >> y)
			{
				if (y == 999)
					break;
				std::cout << "输入广播信息:\n";
				std::string str;
				std::getline(std::cin, str);
				Network::AYPacket packet;
				packet.header.packetId = 0x1234;
				packet.payload.assign(str.begin(), str.end());
				packet.updateHeader();
				Network::AYNetworkManager::getInstance().broadcast("TCP", packet);
			}
			x = getchar();
			break;
		}
		else if (c == 2)
		{
			std::cout << "客户端\n";

			Network::AYNetworkManager::getInstance().start();
			auto client = new Network::AYTcpClient();
			client->connectServer("127.0.0.1", 59383);

			std::cout << "输入整数:\n\t1)发送信息 2）重新连接 3）退出\n";
			int x;
			while (std::cin >> x)
			{
				if (x == 1)
				{
					Network::AYPacket packet;
					packet.header.packetId = 0x1234;
					
					std::cout << "输入发送信息:\n";
					std::string str;
					std::getline(std::cin, str);
					packet.payload.assign(str.begin(),str.end());
					packet.updateHeader();
					auto& header = packet.header;
					std::cout << "\t header.checksum: " << header.checksum <<
						"\n\t header.packetId: " << header.packetId << "\n\t header.payloadSize: " << header.payloadSize << std::endl;
					std::cout << "发送数据大小：" << sizeof(packet.header) + packet.payload.size() << std::endl;
					Network::printBytes(&packet.header, sizeof(Network::STPacketHeader));
					Network::printBytes(packet.payload.data(), packet.payload.size());
					client->send(packet);
				}
				else if (x == 2)
				{
					client->connectServer("127.0.0.1", 59383);
				}
				else if (x == 3)
				{
					client->close();
					break;
				}
				std::cout << "输入整数:\n\t1)发送信息 2）重新连接 3）退出\n";
			}
		}
	}

	Network::AYNetworkManager::getInstance().stop();
	getchar();
	return 0;
}