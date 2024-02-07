#include "connection.h"

Connection::Connection(std::unique_ptr<HttpRouter> router, std::unique_ptr<TcpSocket> socket)
    : router{ std::move(router) },
    socket{ std::move(socket) },
    state{ STARTING }
{}

Connection::~Connection() {}

void Connection::read()
{
    std::array<char, 10240> buff{};
    auto i = socket->tcp_read(buff, 0);
    state = WRITING;
}

void Connection::write()
{
    std::array<char, 4> s{ 'd', 'o', 'n', 'e' };
    socket->tcp_send(s);
    state = DONE;
}

ConnectionState Connection::get_state() const { return state; }

int Connection::get_tcp_socket_descriptor() const { return socket->get_tcp_socket_descriptor(); }