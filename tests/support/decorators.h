#pragma once

#include <memory>
#include <concepts>

#include "../../src/common/types.h"
#include "../../src/tcp_socket.h"
#include "../../src/decorations/implementations.h"

namespace nimlib::Server::Decorators
{
	using nimlib::Server::Types::Socket;
	using nimlib::Server::Sockets::TcpSocketWithLogger;

	template <typename T>
	concept IsTcpSocket = std::derived_from<T, Socket>;

	template <IsTcpSocket T = TcpSocketWithLogger>
	std::unique_ptr<Socket> decorate(std::unique_ptr<Socket> socket)
	{
		return socket;
	}
}
