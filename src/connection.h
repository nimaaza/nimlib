#pragma once

#include <sstream>
#include <chrono>

#include "common.h"
#include "socket.h"
#include "protocol.h"
#include "state_manager.h"
#include "metrics/measurements.h"

using connection_id = int;

class Connection
{
public:
    Connection(std::unique_ptr<Socket>, connection_id);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    ConnectionState read();
    ConnectionState write();
    void halt();
    void set_protocol(std::shared_ptr<ProtocolInterface>);
    std::pair<ConnectionState, long> get_state() const;
    const int get_id() const;

private:
    ConnectionState handle_incoming_data();

private:
    connection_id id;
    StateManager<ConnectionState> sm;
    std::stringstream request_stream;
    std::stringstream response_stream;
    std::unique_ptr<Socket> socket;
    ParseResult parse_result;
    std::shared_ptr<ProtocolInterface> protocol;
    nimlib::Server::Metrics::Measurements::Duration<long> response_timer;
};
