#pragma once
#include "ECNetworkDependence.h"

namespace Network
{
	class AYTcpSession : public boost::enable_shared_from_this<AYTcpSession>
	{
	public:
		using pointer = boost::shared_ptr<AYTcpSession>;

		static pointer create(asio::io_context& io_context)
		{
			return pointer(new AYTcpSession(io_context));
		}

		tcp::socket& socket()
		{
			return _socket;
		}

		//一旦开始监听，该tcp会话就会不停在异步读写间循环，这种一次只能处理一个消息
		void start(MessageHandler handler) 
		{
			_handler = handler;

			//从这个代表对端的socket中读取数据头，并存储到数据头缓冲区，在处理时持有该会话指针，防止运行中被删除
			asio::async_read(_socket,
				asio::buffer(&_headerBuf, sizeof(STMessageHeader)),
				boost::bind(&AYTcpSession::_handleReadHeader,
					shared_from_this(),
					asio::placeholders::error));
		}

		void send(const STNetworkMessage& message)
		{
			std::lock_guard<std::mutex> lock(_writeMutex);
			_writeQueue.push(message);

			if (!_isWriting)
			{
				_dowrite();
			}
		}

		void close()
		{
			_socket.close();
		}


	private:
		AYTcpSession(asio::io_context& io_context) :
			_socket(io_context),
			_isWriting(false)
		{ }

		void _handleReadHeader(const boost::system::error_code& err)
		{
			if (!err && _headerBuf.size <= MAX_PACKET_SIZE)
			{
				_readBuf.resize(_headerBuf.size);

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

		void _handleReadBody(const boost::system::error_code& err)
		{
			if (!err)
			{
				STNetworkMessage msg;
				msg.type = static_cast<AYMessageType>(_headerBuf.type);
				msg.data = std::move(_readBuf);

				if (_handler)
				{
					_handler(msg);
				}

				asio::async_read(_socket,
					asio::buffer(&_headerBuf, sizeof(STMessageHeader)),
					boost::bind(&AYTcpSession::_handleReadHeader,
						shared_from_this(),
						asio::placeholders::error));
			}
			else
			{
				close();
			}
		}
		void _dowrite()
		{
			std::lock_guard<std::mutex> lock(_writeMutex);
			if (_writeQueue.empty())
			{
				_isWriting = false;
				return;
			}

			_isWriting = true;
			const auto& msg = _writeQueue.front();
			
			STMessageHeader header;
			header.type = static_cast<uint32_t>(msg.type);
			header.size = static_cast<uint32_t>(msg.data.size());

			std::vector<asio::const_buffer> buffers;
			buffers.push_back(asio::buffer(&header, sizeof(STMessageHeader)));
			buffers.push_back(asio::buffer(msg.data));

			asio::async_write(_socket,
				buffers,
				[this](const boost::system::error_code& err, size_t) {
					std::lock_guard<std::mutex> lock(_writeMutex);
					if (!err)
					{
						_writeQueue.pop();
						_dowrite();
					}
					else
					{
						close();
					}
				});
		}


		tcp::socket _socket;
		MessageHandler _handler;

		struct STMessageHeader
		{
			uint32_t type;
			uint32_t size;
		};

		STMessageHeader _headerBuf;
		std::vector<uint8_t> _readBuf;

		std::mutex _writeMutex;
		std::queue<STNetworkMessage> _writeQueue;
		bool _isWriting;

	};
}