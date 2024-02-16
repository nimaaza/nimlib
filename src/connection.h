#pragma once

#include <sstream>

#include "common.h"
#include "tcp_socket.h"
#include "protocol.h"

class Connection
{
public:
    Connection(std::unique_ptr<TcpSocket>);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&) noexcept = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    void read();
    void write();
    void halt();
    void set_state(ConnectionState);
    void reset_state();
    std::pair<ConnectionState, long> get_state() const;
    int get_tcp_socket_descriptor() const;

private:
    void handle_incoming_data();

private:
    std::stringstream request_stream;
    std::stringstream response_stream;
    Protocol protocol;
    std::unique_ptr<TcpSocket> socket;
    ConnectionState state;
    ParseResult parse_result;
    long last_state_change;
    int reset_count;
};
