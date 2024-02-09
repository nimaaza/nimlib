#include <sys/socket.h>
#include <utility>

#include "connection.h"

Connection::Connection(std::unique_ptr<HttpRouter> router, std::unique_ptr<TcpSocket> socket)
    : router{ std::move(router) },
    socket{ std::move(socket) },
    request_stream{},
    state{ STARTING },
    last_state_change{}
{}

Connection::~Connection() {}

void Connection::read()
{
    static const int SIZE = 10240;
    std::array<char, SIZE> buff{};
    int bytes_count;

    bytes_count = socket->tcp_read(buff, 0);

    set_state(WRITING);
}

void Connection::write()
{
    std::array<char, 4> s{ 'd', 'o', 'n', 'e' };
    socket->tcp_send(s);
    set_state(DONE);
}

void Connection::set_state(ConnectionState s)
{
    if (s == PENDING) request_stream.clear();
    state = s;
}

std::pair<ConnectionState, long> Connection::get_state() const { return { state, last_state_change }; }

int Connection::get_tcp_socket_descriptor() const { return socket->get_tcp_socket_descriptor(); }
