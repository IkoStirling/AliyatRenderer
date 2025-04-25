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

	void AYTcpSession::start(MessageHandler handler)
	{
		_handler = handler;

		//��ʼ�������ѭ��
		_recvBuf.resize(sizeof(STTcpHeader));
		asio::async_read(_socket,
			asio::buffer(&_headerBuf, sizeof(STTcpHeader)),
			boost::bind(&AYTcpSession::_handleReadHeader,
				shared_from_this(),
				asio::placeholders::error));
	}

	void AYTcpSession::send(const STNetworkMessage& msg)
	{
		std::lock_guard<std::mutex> lock(_writeMutex);

		//��Ϣ���г������ƣ���������
		if (_writeQueue.size() <= MAX_QUEUE_SIZE) {
			_writeQueue.push_back(msg);
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
		}
	}

	std::wstring AYTcpSession::remoteAddress() const
	{
		try {
			return std::wstring(_socket.remote_endpoint().address().to_string().begin(),
				_socket.remote_endpoint().address().to_string().end());
		}
		catch (const std::exception&) {
			return L"";
		}
	}

	void AYTcpSession::_handleReadHeader(const boost::system::error_code& err)
	{
		if (!err)
		{
			auto headerPtr = reinterpret_cast<STTcpHeader*>(_recvBuf.data());
			if (headerPtr->bodyLength > MAX_PACKET_SIZE)
			{
				close();
				return;
			}

			_readBuf.resize(sizeof(STTcpHeader) + _headerBuf.bodyLength);

			asio::async_read(_socket,
				asio::buffer(_readBuf),
				boost::bind(&AYTcpSession::_handleReadBody,
					shared_from_this(),
					asio::placeholders::error));
		}
		else
		{
			close();
		}
	}

	void AYTcpSession::_handleReadBody(const boost::system::error_code& err)
	{
		if (!err)
		{
			auto headerPtr = reinterpret_cast<STTcpHeader*>(_recvBuf.data());

			STNetworkMessage msg;
			msg.id = headerPtr->messageID;
			msg.type = static_cast<AYMessageType>(headerPtr->messageType);
			msg.data.assign(_recvBuf.begin() + sizeof(STTcpHeader),
				_recvBuf.end());

			if (_handler)
			{
				_handler(msg);
			}

			//�������걾����Ϣ��������һ�ֲ������
			asio::async_read(_socket,
				asio::buffer(&_headerBuf, sizeof(STTcpHeader)),
				boost::bind(&AYTcpSession::_handleReadHeader,
					shared_from_this(),
					asio::placeholders::error));
		}
		else
		{
			close();
		}
	}

	void AYTcpSession::_doWrite()
	{
		std::lock_guard<std::mutex> lock(_writeMutex);
		if (_writeQueue.empty())
		{
			_isWriting = false;
			return;
		}

		_isWriting = true;
		const auto& msg = _writeQueue.front();

		STTcpHeader header;
		header.messageID = msg.id;
		header.messageType = static_cast<uint32_t>(msg.type);
		header.bodyLength = static_cast<uint32_t>(msg.data.size());

		std::vector<asio::const_buffer> buffers;
		buffers.push_back(asio::buffer(&header, sizeof(STTcpHeader)));
		buffers.push_back(asio::buffer(msg.data));

		auto self = shared_from_this();
		asio::async_write(_socket,
			buffers,
			[self](const boost::system::error_code& err, size_t) {
				std::lock_guard<std::mutex> lock(self->_writeMutex);
				if (!err)
				{
					self->_writeQueue.pop_front();
					self->_doWrite();
				}
				else
				{
					self->close();
				}
			});
	}

	AYTcpSession::AYTcpSession(asio::io_context& io_context) :
		_socket(io_context),
		_isWriting(false)
	{

	}
}