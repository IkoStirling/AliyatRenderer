#pragma once
#include "IAYBaseSession.h"


namespace Network {

#define MAX_QUEUE_SIZE 1024

	class AYTcpSession : public IAYBaseSession, public boost::enable_shared_from_this<AYTcpSession>
	{
	public:
		using pointer = boost::shared_ptr<AYTcpSession>;
	private:
#pragma pack(push,1)
		struct STTcpHeader
		{
			boost::uuids::uuid messageID;
			uint32_t messageType;
			uint32_t bodyLength;
		};
#pragma pack(pop)
	public:
		static pointer create(asio::io_context& io_context);
		tcp::socket& getSocket();
		boost::uuids::uuid getSessionID() const override;
		void start(MessageHandler handler) override;
		void send(AYMessageType type, std::vector<uint8_t>& data, ResponseHandler onResponse = nullptr) override;
		void close() override;
		std::string remoteAddress() const override;
	private:
		explicit AYTcpSession(asio::io_context& io_context);
		void _doReadHeader();
		void _doReadBody();
		//void _doRead(); 因为拆包处理，单一doRead函数分为了处理消息头和处理消息体
		void _doWrite();
		std::vector<uint8_t> _serializeMessage(const STNetworkMessage& msg);
		STTcpHeader _deserializeHeader(const uint8_t* data);
		void _startTimeout(int second = 30);
		void _startIdleTimeout(int second = 300);
		void _resetIdleTimeout();
	private:
		tcp::socket _socket;
		STTcpHeader _headerBuffer;
		asio::streambuf _streamBuffer;
		MessageHandler _handler;
		std::deque<std::vector<uint8_t>> _writeQueue;
		std::mutex _writeMutex;
		bool _isWriting;

		std::unordered_map<boost::uuids::uuid, ResponseHandler, boost::hash<boost::uuids::uuid>> _responseCallbacks;
		std::mutex _callbackMutex;

		asio::steady_timer _timeoutTimer; //操作超时计时器
		asio::steady_timer _idleTimer; //全局静默计时器
		int _idleSeconds;

		boost::uuids::uuid _sessionID;
	};
}