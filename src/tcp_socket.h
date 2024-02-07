#pragma once

#include <string>
#include <span>
#include <netdb.h>

struct TcpSocket
{
    TcpSocket(const std::string& port);
    TcpSocket(int tcp_socket);
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    TcpSocket(TcpSocket&&) noexcept;
    TcpSocket& operator=(TcpSocket&&) noexcept;


    void tcp_bind();
    void tcp_listen();
    std::unique_ptr<TcpSocket> tcp_accept();
    void tcp_get_host_name(const sockaddr& socket_address, std::string& host_name);
    int tcp_read(std::span<char> buffer, int flags);
    int tcp_send(std::span<char> buffer);
    void tcp_close();  // TODO: may never be used

    const int get_tcp_socket_descriptor() const;

    // TODO: the following operations on sockets might become necessary
    // connect(2)
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

private:
    // TODO: try to avoid the raw pointer here
    addrinfo* bind_address;
    int tcp_socket_descriptor;

    // TODO: this will eventually come from config
    static const int MAX_CONNECTIONS{ 10 };
};
