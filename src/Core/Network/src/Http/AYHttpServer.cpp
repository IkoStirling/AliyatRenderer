#include "Http/AYHttpServer.h"

Network::AYHttpServer::AYHttpServer(asio::io_context& io_context)
    : _tcpServer(std::make_shared<AYTcpServer>(io_context)) {
    _tcpServer->setSessionHandler([this](base_pointer session) {
        _handleNewSession(session);
        });
}

Network::AYHttpServer::~AYHttpServer()
{
}

void Network::AYHttpServer::start(port_id port) {
    _tcpServer->start(port);
}

void Network::AYHttpServer::_handleNewSession(base_pointer session) {
    auto httpSession = std::make_shared<AYHttpSession>(
        _tcpServer->getIoContext(),
        std::dynamic_pointer_cast<AYTcpSession>(session));
    httpSession->start();
}

void Network::AYHttpServer::addRoute(
    const std::string& path,
    std::function<void(AYHttpRequest&, AYHttpResponse&)> handler) {
    _routes[path] = handler;
}

// AYHttpSession.cpp
void Network::AYHttpSession::_handleData(const char* data, size_t length) {
    _buffer.append(data, length);

    // 检查是否收到完整的HTTP请求（根据空行判断）
    size_t endOfHeaders = _buffer.find("\r\n\r\n");
    if (endOfHeaders != std::string::npos) {
        _currentRequest.parse(_buffer.substr(0, endOfHeaders + 4));
        _buffer.erase(0, endOfHeaders + 4);
        _processHttpRequest();
    }
}

void Network::AYHttpSession::_processHttpRequest() {
    AYHttpResponse response;

    // 查找匹配的路由
    auto it = _server->getRoutes().find(_currentRequest.path);
    if (it != _server->getRoutes().end()) {
        it->second(_currentRequest, response);
    }
    else {
        response.statusCode = 404;
        response.statusText = "Not Found";
        response.body = "404 Not Found";
    }

    _tcpSession->send(AYPacket(response.toString().data(), response.toString().size()));
}