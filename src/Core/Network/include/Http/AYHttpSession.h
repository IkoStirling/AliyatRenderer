#pragma once
#include "Base/IAYBaseSession.h"
#include "AYHttpRequest.h"
#include "AYHttpResponse.h"

namespace Network {
    class AYHttpSession : public IAYBaseSession {
    public:
        AYHttpSession(asio::io_context& io_context,
            std::shared_ptr<AYTcpSession> tcpSession);

        void start() override;
        void send(const AYPacket& packet) override;

    private:
        void _handleData(const char* data, size_t length);
        void _processHttpRequest();

        std::shared_ptr<AYTcpSession> _tcpSession;
        AYHttpRequest _currentRequest;
        std::string _buffer;
    };
}