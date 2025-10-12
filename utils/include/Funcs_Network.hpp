#pragma once
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace Utils
{
	using boost::asio::ip::tcp;
	namespace asio = boost::asio;

	template<class T>
	void HandleRequest(T& socket, const std::string& server, const std::string path)
	{
		std::string request = "GET " + path + " HTTP/1.1\r\n";
		request += "Host: " + server + "\r\n";
		request += "Accept: */*\r\n";
		request += "Connection: close\r\n\r\n";

		asio::write(socket, asio::buffer(request));
		char buffer[1024];
		boost::system::error_code error;

		while (size_t len = socket.read_some(asio::buffer(buffer), error))
		{
			std::cout.write(buffer, len);
		}
	}

	void HttpsRequest(const std::string& server, const std::string path)
	{
		asio::io_context io_context;
		asio::ssl::context ssl_context(asio::ssl::context::sslv23);
		ssl_context.set_options(asio::ssl::context::default_workarounds
			| asio::ssl::context::no_sslv2
			| asio::ssl::context::no_sslv3);
		ssl_context.use_certificate_chain_file("");  // 不需要，Schannel 自动管理
		ssl_context.use_private_key_file("", asio::ssl::context::file_format::pem);  // 不需要

		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(server, "https");

		asio::ssl::stream<tcp::socket> socket(io_context, ssl_context);

		asio::connect(socket.lowest_layer(), endpoints);
		socket.handshake(asio::ssl::stream_base::client);
		HandleRequest(socket, server, path);
	}

	void HttpRequest(const std::string& server, const std::string path)
	{
		asio::io_context io_context;
		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(server, "http");
		tcp::socket socket(io_context);
		asio::connect(socket, endpoints);
		HandleRequest(socket, server, path);
	}
}
