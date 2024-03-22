#pragma once

#include <memory>
#include <concepts>

#include "../../src/types.h"
#include "../../src/tcp_socket.h"
#include "../../src/tcp_socket_with_logger.h"

namespace nimlib::Server::Decorators
{
	using nimlib::Server::Types::TcpSocketInterface;
    using nimlib::Server::Sockets::TcpSocketWithLogger;

	template <typename T>
	concept IsTcpSocket = std::derived_from<T, TcpSocketInterface>;

	template <IsTcpSocket T = TcpSocketWithLogger>
	std::unique_ptr<TcpSocketInterface> decorate(std::unique_ptr<TcpSocketInterface> socket)
	{
		return socket;
	}
}
