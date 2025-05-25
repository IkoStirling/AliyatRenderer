#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <mutex>

namespace Network
{
	using boost::asio::ip::tcp;
	namespace asio = boost::asio;

	const size_t MAX_PACKET_SIZE = 1024 * 1024 * 10; // 10MB

    enum class AYMessageType {
        Text,
        Binary,
        Command,
        VideoFrame
    };

    struct STNetworkMessage {
        AYMessageType type;
        boost::uuids::uuid id;
        std::vector<uint8_t> data;
    };

   
    using MessageHandler = std::function<void(const STNetworkMessage&)>;
    using ResponseHandler = std::function<void(const STNetworkMessage&)>;
}