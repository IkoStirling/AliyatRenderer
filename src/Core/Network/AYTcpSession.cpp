#include "AYTcpSession.h"

namespace Network {
	AYTcpSession::AYTcpSession(tcp::socket in_socket)
	{
		_socket = in_socket;
	}

}