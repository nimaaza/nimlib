#include "connection.h"

#include <sys/socket.h>

Connection::Connection(std::unique_ptr<HttpRouter> router, std::unique_ptr<TcpSocket> socket)
    : router{ std::move(router) },
    socket{ std::move(socket) },
    request_stream{},
    state{ STARTING }
{}

Connection::~Connection() {}

void Connection::read()
{
    static const int SIZE = 10;

    int flag = 0;
    int bytes_count;
    std::array<char, SIZE> buff{};

    do
    {
        bytes_count = socket->tcp_read(buff, flag);
        for (int i = 0; i < buff.size(); i++)
        {
            request_stream << buff[i];
            buff[i] = '\0';
            flag = MSG_DONTWAIT;
        }
    } while (bytes_count > 0);

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

ConnectionState Connection::get_state() const { return state; }

int Connection::get_tcp_socket_descriptor() const { return socket->get_tcp_socket_descriptor(); }