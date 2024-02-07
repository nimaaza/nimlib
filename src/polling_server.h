#pragma once

#include "connection.h"
#include "tcp_socket.h"

#include <unordered_map>
#include <queue>
#include <string>
#include <memory>
#include <algorithm>
#include <poll.h>

class PollingServer
{
public:
    PollingServer(const std::string&);
    ~PollingServer();

    void run();

private:
    pollfd create_socket_descriptor(int);
    void queue_connections(short, std::vector<pollfd>&, std::vector<ConnectionState>&, std::queue<std::unique_ptr<Connection>>&);
    void handle_reads();
    void handle_writes();

private:
    const std::string& port;
    std::unique_ptr<TcpSocket> server_socket;
    std::unordered_map<int, std::unique_ptr<Connection>> connections;
    std::queue <std::unique_ptr<Connection>> read_queue;
    std::queue <std::unique_ptr<Connection>> write_queue;
};
