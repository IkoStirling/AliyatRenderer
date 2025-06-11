#include "AYTcpSession.h"
#include "AYTcpPacketAssembler.h"
#include "AYNetworkHandler.h"

namespace Network {

	AYTcpSession::pointer AYTcpSession::create(asio::io_context& io_context, AYNetworkHandler& handler)
	{
		return pointer(new AYTcpSession(io_context, handler));
	}

	AYTcpSession::AYTcpSession(boost::asio::io_context& io_context, AYNetworkHandler& handler):
		_socket(io_context),
		_handler(handler)
	{

	}

	void AYTcpSession::start()
	{
		_assembler = std::make_unique<AYTcpPacketAssembler>([this](const AYPacket& packet) {
			_handler.onPacketReceived(this, packet);
			});
		_startRead();
	}

	void AYTcpSession::close()
	{
		boost::system::error_code ec;

		// 安全关闭socket
		_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		_socket.close(ec);

		// 清理资源
		_assembler.reset();

		// 清空发送队列
		std::lock_guard<std::mutex> lock(_sendMutex);
		std::queue<ByteBuffer> empty;
		std::swap(_sendQueue, empty);
	}

	void AYTcpSession::send(const AYPacket& packet)
	{
		ByteBuffer buffer;
		// 写入包头
		buffer << packet.header;

		// 写入有效载荷
		if (!packet.payload.empty()) {
			buffer.append(packet.payload.data(), packet.payload.size());
		}

		{
			std::lock_guard<std::mutex> lock(_sendMutex);
			_sendQueue.push(std::move(buffer));
		}

		if (!_isWriting)
			_doWrite();
	}

	void AYTcpSession::connect(const std::string& ip_str, port_id port) {
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

	tcp::socket& AYTcpSession::getSocket()
	{
		return _socket;
	}

	void AYTcpSession::_startRead()
	{
		auto self(shared_from_this());
		_socket.async_read_some(
			boost::asio::buffer(_receiveBuffer, RECV_BUFFER_SIZE),
			[this, self](const boost::system::error_code& ec, size_t bytes_transferred)
			{
				if (!ec)
				{
					std::cout << "读取到一些数据\r\n";
					_assembler->feedData(_receiveBuffer.data(), bytes_transferred);
					_startRead();
				}
				else
				{
					close();
					_handler.onSessionClosed(this);
				}
			}
		);
	}

	void AYTcpSession::_doWrite()
	{
		std::lock_guard<std::mutex> lock(_sendMutex);
		if (_sendQueue.empty()) {
			_isWriting = false;
			return;
		}

		_isWriting = true;
		auto& buffer = _sendQueue.front();

		auto self(shared_from_this());
		boost::asio::async_write(_socket,
			boost::asio::buffer(buffer.data(), buffer.size()),
			[this, self](boost::system::error_code ec, size_t /*length*/) {
				std::lock_guard<std::mutex> lock(_sendMutex);
				if (!ec) {
					std::cout << "发送成功\r\n";
					_sendQueue.pop();
					if (!_sendQueue.empty()) {
						_doWrite();  // 继续发送下一个
					}
					else {
						_isWriting = false;
					}
				}
				else {
					// 错误处理
					_handler.onSessionClosed(this);
					_isWriting = false;
				}
			});
	}
}