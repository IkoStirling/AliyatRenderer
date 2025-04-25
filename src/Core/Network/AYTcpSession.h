#pragma once
#include "IAYBaseSession.h"

namespace Network {
	class AYTcpSession : public IAYBaseSession, public boost::enable_shared_from_this<AYTcpSession>
	{
	public:
		using pointer = boost::shared_ptr<AYTcpSession>;
	public:
		static pointer create(tcp::socket socket);
		tcp::socket& getSocket();
		void start() override;
		void send(const STNetworkMessage& msg) override;
		void close() override;
		std::wstring remoteAddress() const override;
	private:
		explicit AYTcpSession(tcp::socket in_socket);

		tcp::socket _socket;
		void _doWrite();
		void _doRead();
		boost::asio::streambuf _readBuffer;
		std::deque<STNetworkMessage> _writeQueue;
	};
}