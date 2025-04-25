#pragma once
#include "IAYBaseSession.h"


namespace Network {

#define MAX_QUEUE_SIZE 1024

	class AYTcpSession : public IAYBaseSession, public boost::enable_shared_from_this<AYTcpSession>
	{
	public:
		using pointer = boost::shared_ptr<AYTcpSession>;
	public:
		static pointer create(asio::io_context& io_context);
		tcp::socket& getSocket();
		void start(MessageHandler handler) override;
		void send(const STNetworkMessage& msg) override;
		void close() override;
		std::wstring remoteAddress() const override;
	private:
		explicit AYTcpSession(asio::io_context& io_context);
		void _handleReadHeader(const boost::system::error_code& err);
		void _handleReadBody(const boost::system::error_code& err);
		//void _doRead(); 因为拆包处理，单一doRead函数分为了处理消息头和处理消息体
		void _doWrite();
	private:
#pragma pack(push,1)
		struct STTcpHeader
		{
			boost::uuids::uuid messageID;
			uint32_t messageType;
			uint32_t bodyLength;
		};
#pragma pack(pop)
	private:
		tcp::socket _socket;
		STTcpHeader _headerBuf;
		std::vector<uint8_t> _readBuf;
		std::vector<uint8_t> _recvBuf;
		asio::streambuf _streamBuffer;
		MessageHandler _handler;
		std::deque<STNetworkMessage> _writeQueue;
		std::mutex _writeMutex;
		bool _isWriting;

	};
}