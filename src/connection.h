#pragma once

#include <sstream>

#include "tcp_socket.h"

// =======================================================================
class HttpRouter {};
class HttpParser {};
class HttpObject {};
class HttpRequest {};
class HttpResponse {};
// =======================================================================

enum ConnectionState { STARTING, READING, HANDLING, WRITING, PENDING, DONE };

class Connection
{
public:
    Connection(std::unique_ptr<HttpRouter>, std::unique_ptr<TcpSocket>);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&) noexcept = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    void read();
    void write();
    void halt();
    void set_state(ConnectionState);
    std::pair<ConnectionState, long> get_state() const;
    int get_tcp_socket_descriptor() const;

private:
    void parse();
    void route();
    void write_error();
    void write_response();

private:
    std::stringstream request_stream;
    HttpParser parser;
    HttpObject http_object;
    HttpRequest request;
    HttpResponse response;
    std::unique_ptr<HttpRouter> router;
    std::unique_ptr<TcpSocket> socket;
    ConnectionState state;
    long last_state_change;
};
