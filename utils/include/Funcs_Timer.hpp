#pragma once
#include <boost/asio.hpp>
#include <functional>
#include <chrono>

namespace Utils
{
	namespace asio = boost::asio;

	void CreateTimer(asio::io_context& io_context, int seconds, std::function<void(const boost::system::error_code& ec)> exec)
	{
		try {
			auto timer = std::make_shared<boost::asio::steady_timer>(io_context, std::chrono::seconds(seconds));

			timer->async_wait([timer, exec](const boost::system::error_code& ec) {
				exec(ec);  // Ö´ÐÐ»Øµ÷
				});
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}
	}
}