#pragma once

#include <span>
#include <netdb.h>

#include "common/types.h"

namespace nimlib::Server::Sockets
{
	using nimlib::Server::Types::TcpSocketInterface;

    struct TcpSocket : public TcpSocketInterface
    {
        TcpSocket(int tcp_socket, const std::string& port = "");
        TcpSocket(const std::string& port);
        ~TcpSocket();

        TcpSocket(const TcpSocket&) = delete;
        TcpSocket& operator=(const TcpSocket&) = delete;
        TcpSocket(TcpSocket&&) noexcept;
        TcpSocket& operator=(TcpSocket&&) noexcept;

        // int tcp_connect(const std::string& addr, const std::string& port) override;
        int tcp_bind() override;
        int tcp_listen() override;
        std::unique_ptr<TcpSocketInterface> tcp_accept() override;
        void tcp_get_host_name(const sockaddr& socket_address, std::string& host_name) override;
        int tcp_read(std::span<uint8_t> buffer, int flags) override;
        int tcp_send(std::span<uint8_t> buffer) override;
        int tcp_send(std::string_view buffer) override;
        void tcp_close() override;  // TODO: may never be used
        const int get_tcp_socket_descriptor() const override;
		const std::string& get_port() const override;

		// TODO: the following operations on sockets might become necessary
        // connectx(2)
        // disconnectx(2)
        // getsockopt(2)
        // shutdown(2)
        // socketpair(2),
        // getprotoent(3)
        // inet(4)
        // inet6(4)
        // unix(4)
        // compat(5)

    protected:
        // TODO: try to avoid the raw pointer here
        addrinfo* bind_address;
        // TODO: this will eventually come from config
        static const int MAX_CONNECTIONS{ 10 };
    };
};
