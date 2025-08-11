#pragma once
#include "Base/IAYBaseSession.h"


namespace Network
{
	class AYTcpSession;
	class AYNetworkHandler;

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
		void connect(const std::string& ip_str, port_id port);

		void getSessionID() {};

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