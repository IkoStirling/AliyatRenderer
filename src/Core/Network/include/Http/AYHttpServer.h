#pragma once
#include "Tcp/AYTcpServer.h"
#include "AYHttpSession.h"
#include "AYHttpRequest.h"
#include "AYHttpResponse.h"

namespace Network {
    class AYHttpServer : public IAYBaseServer {
    public:
        AYHttpServer(asio::io_context& io_context);
        ~AYHttpServer();

        void start(port_id port) override;
        void stop() override;

        // HTTP路由注册接口
        void addRoute(const std::string& path,
            std::function<void(AYHttpRequest&, AYHttpResponse&)> handler);

    private:
        void _handleNewSession(base_pointer session);

        std::shared_ptr<AYTcpServer> _tcpServer;
        std::unordered_map<std::string,
            std::function<void(AYHttpRequest&, AYHttpResponse&)>> _routes;
    };
}