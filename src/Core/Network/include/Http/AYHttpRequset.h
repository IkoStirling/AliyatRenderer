#pragma once
#include <string>
#include <unordered_map>

namespace Network {
    class AYHttpRequest {
    public:
        std::string method;
        std::string path;
        std::string version;
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        void parse(const std::string& rawRequest);
    };
}

// AYHttpResponse.h
#pragma once
#include <string>
#include <unordered_map>

namespace Network {
    class AYHttpResponse {
    public:
        int statusCode = 200;
        std::string statusText = "OK";
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        std::string toString() const;
    };
}