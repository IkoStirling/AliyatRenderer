#include <iostream>
#include "Core/Network/Network.hpp"


/*
	ְ��
		1��������������Ӧ����˽ӿ�
		2�����³���
		3����Ⱦ����
		4��ת��֡�����������
*/
using namespace Network;

int main()
{
	std::cout << "client" << std::endl;

	boost::asio::io_context io_context;
	auto tcps = AYTcpSession::create(io_context);
	tcps->start([](const STNetworkMessage& msg) {
		std::cout << reinterpret_cast<const char*>(msg.data.data(), msg.data.size()) << std::endl;
		});
	tcp::v4();
	return 0;
}