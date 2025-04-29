#include "AYTcpSession.h"

namespace Network {
	AYTcpSession::pointer AYTcpSession::create(asio::io_context& io_context)
	{
		return pointer(new AYTcpSession(io_context));
	}

	tcp::socket& AYTcpSession::getSocket()
	{
		return _socket;
	}

	boost::uuids::uuid AYTcpSession::getSessionID() const
	{
		return _sessionID;
	}

	void AYTcpSession::start(MessageHandler handler)
	{
		_handler = handler;

		//��ʼ�������ѭ��
		_startIdleTimeout();
		_doReadHeader();
	}

	void AYTcpSession::send(AYMessageType type, std::vector<uint8_t>& data, ResponseHandler onResponse)
	{
		//��Ϣ���г������ƣ���������
		if (_writeQueue.size() > MAX_QUEUE_SIZE)
			return;

		STNetworkMessage msg;
		msg.id = boost::uuids::random_generator()();
		msg.type = type;
		msg.data = data;

		auto buffer = _serializeMessage(msg);

		{
			std::lock_guard<std::mutex> lock(_writeMutex);
			_writeQueue.push_back(buffer);
		}

		if (onResponse)
		{
			std::lock_guard<std::mutex> lock(_callbackMutex);
			_responseCallbacks[msg.id] = onResponse;
		}

		if (!_isWriting)
		{
			_doWrite();
		}
	}

	void AYTcpSession::close() 
	{
		std::lock_guard<std::mutex> lock(_writeMutex);
		if (_socket.is_open())
		{
			boost::system::error_code ec;
			_socket.close(ec);
			_writeQueue.clear();
			_isWriting = false;
		}
	}

	std::string AYTcpSession::remoteAddress() const
	{
		try {
			return _socket.remote_endpoint().address().to_string();
		}
		catch (const std::exception&) {
			return "";
		}
	}

	void AYTcpSession::_doReadHeader()
	{
		_startTimeout();
		_resetIdleTimeout();
		auto self = shared_from_this();
		asio::async_read(_socket, _streamBuffer,
			asio::transfer_exactly(sizeof(STTcpHeader)),
			[this, self](const boost::system::error_code& err, size_t) {
				_timeoutTimer.cancel();
				if (!err)
				{
					std::istream is(&_streamBuffer);
					STTcpHeader header;
					is.read(reinterpret_cast<char*>(&header), sizeof(STTcpHeader));

					header.messageType = ntohl(header.messageType);
					header.bodyLength = ntohl(header.bodyLength);

					if (header.bodyLength > MAX_PACKET_SIZE) {
						close();
						return;
					}

					// bodyLength��������
					_headerBuffer = header;

					// ������ body
					_doReadBody();
				}
				else
				{
					close();
				}
			});
	}

	void AYTcpSession::_doReadBody()
	{
		_startTimeout();
		_resetIdleTimeout();
		auto self = shared_from_this();
		asio::async_read(_socket, _streamBuffer,
			asio::transfer_exactly(_headerBuffer.bodyLength),
			[this, self](const boost::system::error_code& err, size_t) {
				_timeoutTimer.cancel();
				if (!err)
				{
					std::istream is(&_streamBuffer);
					std::vector<uint8_t> bodyData(_headerBuffer.bodyLength);
					//ת����char*ָ�룬��ָ�����ȵĻ������������뵽��ָ��λ��
					is.read(reinterpret_cast<char*>(bodyData.data()), _headerBuffer.bodyLength);

					STNetworkMessage msg;
					msg.id = _headerBuffer.messageID;
					msg.type = static_cast<AYMessageType>(_headerBuffer.messageType);
					msg.data = std::move(bodyData);

					{
						//����ṩ������Ļص������ʹ�õ�������Ļص�
						std::lock_guard<std::mutex> lock(_callbackMutex);
						auto it = _responseCallbacks.find(msg.id);
						if (it != _responseCallbacks.end()) {
							it->second(msg);
							_responseCallbacks.erase(it);
							_doReadHeader();
							return;
						}
					}

					if (_handler)
						_handler(msg);

					// ������һ�ζ�ȡ header
					_doReadHeader();
				}
				else
				{
					close();
				}
			});
	}

	void AYTcpSession::_doWrite()
	{
		_startTimeout();
		_resetIdleTimeout();
		std::lock_guard<std::mutex> lock(_writeMutex);
		if (_writeQueue.empty() || _isWriting)
			return;

		_isWriting = true;

		auto self = shared_from_this();
		asio::async_write(_socket,
			asio::buffer(_writeQueue.front()),
			[this, self](const boost::system::error_code& err, size_t) {
				_timeoutTimer.cancel();
				std::lock_guard<std::mutex> lock(_writeMutex);
				if (!err)
				{
					_writeQueue.pop_front();
					_isWriting = false;
					if (!_writeQueue.empty()) 
						_doWrite();
				}
				else
				{
					close();
				}
			});
	}

	std::vector<uint8_t> AYTcpSession::_serializeMessage(const STNetworkMessage& msg)
	{
		std::vector<uint8_t> buffer;
		buffer.reserve(sizeof(STTcpHeader) + msg.data.size());

		STTcpHeader header;
		header.messageID = msg.id;
		header.messageType = htonl(static_cast<uint32_t>(msg.type));
		header.bodyLength = htonl(static_cast<uint32_t>(msg.data.size()));

		// ���� header
		buffer.insert(buffer.end(),
			reinterpret_cast<const uint8_t*>(&header),
			reinterpret_cast<const uint8_t*>(&header) + sizeof(STTcpHeader));

		// ���� body
		buffer.insert(buffer.end(), msg.data.begin(), msg.data.end());

		return buffer;
	}

	AYTcpSession::STTcpHeader AYTcpSession::_deserializeHeader(const uint8_t* data)
	{
		STTcpHeader header;
		std::memcpy(&header, data, sizeof(STTcpHeader));
		header.messageType = ntohl(header.messageType);
		header.bodyLength = ntohl(header.bodyLength);
		return header;
	}

	void AYTcpSession::_startTimeout(int second)
	{
		_timeoutTimer.expires_after(std::chrono::seconds(second)); // Ĭ��30�볬ʱ
		auto self = shared_from_this();
		_timeoutTimer.async_wait([this, self](const boost::system::error_code& ec) {
			if(!ec)
				close();
			});
	}

	void AYTcpSession::_startIdleTimeout(int second)
	{
		_idleSeconds = second;
		_idleTimer.expires_after(std::chrono::seconds(_idleSeconds)); // Ĭ��30�볬ʱ
		auto self = shared_from_this();
		_idleTimer.async_wait([this, self](const boost::system::error_code& ec) {
			if (!ec)
			{
				std::cout << "[Idle Timeout] Session " << remoteAddress() << " idle too long, closing." << std::endl;
				close();
			}
			});
	}

	void AYTcpSession::_resetIdleTimeout()
	{
		_idleTimer.cancel();  // ȡ��֮ǰ�Ķ�ʱ������ȫ����
		_startIdleTimeout(_idleSeconds);  // �������� 60 �� idle ��ʱ��ʱ
	}

	AYTcpSession::AYTcpSession(asio::io_context& io_context) :
		_socket(io_context),
		_timeoutTimer(io_context),
		_idleTimer(io_context),
		_isWriting(false)
	{
		_sessionID = boost::uuids::random_generator()();
	}
}