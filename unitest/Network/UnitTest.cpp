#include <iostream>
#include "boost/asio.hpp"
int main()
{

	boost::asio::io_context io_context;
	io_context.stop();
	return 0;
}