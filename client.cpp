#include <iostream>
#include "Core/Network/Network.hpp"


/*
	职责：
		1）发出请求与响应服务端接口
		2）更新场景
		3）渲染场景
		4）转发帧画面至服务端
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