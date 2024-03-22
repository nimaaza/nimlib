#pragma once

#include <span>
#include <memory>
#include <string_view>

struct Socket
{
    virtual ~Socket() = default;

    virtual int read(std::span<uint8_t>) = 0;
    virtual int send(std::span<uint8_t>) = 0;
    virtual int send(std::string_view) = 0;
};

///////////////////////////////////////////////
#include "tcp_socket.h"

#include <sys/socket.h>

using nimlib::Server::Sockets::TcpSocketInterface;
using nimlib::Server::Sockets::TcpSocket;


class TcpSocketAdapter : public Socket
{
public:
    TcpSocketAdapter(std::unique_ptr<TcpSocketInterface> s) : s{ std::move(s) } {}

    TcpSocketAdapter(const std::string& port)
    {
        auto tcp_socket = std::make_unique<TcpSocket>(port);
        this->s = std::move(tcp_socket);
    }

    TcpSocketAdapter(const TcpSocketAdapter&) = delete;
    TcpSocketAdapter& operator=(const TcpSocketAdapter&) = delete;
    TcpSocketAdapter(TcpSocketAdapter&&) noexcept = delete;
    TcpSocketAdapter& operator=(TcpSocketAdapter&&) noexcept = delete;

    int read(std::span<uint8_t> buf) override { return s->tcp_read(buf, MSG_DONTWAIT); }

    int send(std::span<uint8_t> buf) override { return s->tcp_send(buf); }

    int send(std::string_view buf) override { return s->tcp_send(buf); }

private:
    std::unique_ptr<TcpSocketInterface> s;
};
///////////////////////////////////////////////
