#pragma once

#include <sstream>
#include <chrono>

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
    std::pair<ConnectionState, long> get_state() const;
    int get_tcp_socket_descriptor() const;

private:
    void handle_incoming_data();
    void set_state(ConnectionState);
    void reset_state();

private:
    std::stringstream request_stream;
    std::stringstream response_stream;
    std::unique_ptr<TcpSocket> socket;
    ConnectionState state;
    ParseResult parse_result;
    std::chrono::steady_clock::time_point last_state_change;
    int reset_count;
    Protocol protocol;
};
