#pragma once
#include "IAYBaseSession.h"


namespace Network 
{
	class AYTcpSession;
	class AYNetworkHandler
	{
	public:
		void onPacketReceived(AYTcpSession* session, const AYPacket& packet) {
			std::string rstr(packet.payload.begin(), packet.payload.end());
			std::cout << rstr << std::endl;
		}
		void onConnectError(const std::string& error_msg) { std::cout << error_msg; }
		void onSessionClosed(AYTcpSession* session)	{}
	};
	class AYTcpPacketAssembler;

	class AYTcpSession : public IAYBaseSession, public boost::enable_shared_from_this<AYTcpSession>
	{
	public:
		using pointer = boost::shared_ptr<AYTcpSession>;
	private:
		static constexpr int RETRY_TIMES = 3;
		static constexpr size_t RECV_BUFFER_SIZE = 8192;

	public:
		static pointer create(asio::io_context& io_context, AYNetworkHandler& handler);

		AYTcpSession(boost::asio::io_context& io_context, AYNetworkHandler& handler);

		void start() override;
		void close() override;
		void send(const AYPacket& packet) override;

		void connect(const std::string& ip_str, port_id port) {
			boost::system::error_code ec;
			auto ip = asio::ip::make_address(ip_str, ec);

			if (ec) {
				_handler.onConnectError("Invalid IP format: " + ec.message());
				return;
			}

			asio::ip::tcp::endpoint endpoint(ip, port);

			_socket.async_connect(endpoint,
				[this, self = shared_from_this()](const boost::system::error_code& ec) {
					if (!ec) {
						std::cout << "目标服务器连接成功\r\n";
						this->start(); // 连接成功后启动读写
					}
					else {
						_handler.onConnectError(ec.message()); // 需要添加错误回调
					}
				}
			);
		}

		boost::asio::ip::tcp::socket& getSocket();

	private:
		void _startRead();
		void _doWrite();

	private:
		boost::asio::ip::tcp::socket _socket;
		AYNetworkHandler& _handler;	//callback

		std::array<uint8_t, RECV_BUFFER_SIZE> _receiveBuffer;
		
		std::unique_ptr<AYTcpPacketAssembler> _assembler;

		std::queue<ByteBuffer> _sendQueue;
		std::mutex _sendMutex;

		bool _isWriting = false;
	};
}