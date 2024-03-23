#pragma once

#include <vector>

#include "types.h"

namespace nimlib::Server
{
    class ConnectionPool
    {
    private:
        ConnectionPool(/* args */);
    public:

        ~ConnectionPool();

        ConnectionPool(const ConnectionPool&) = delete;
        ConnectionPool& operator=(const ConnectionPool&) = delete;
        ConnectionPool(ConnectionPool&&) = delete;
        ConnectionPool& operator=(ConnectionPool&&) = delete;

        void record_connection(connection_ptr c);
        void record_connection(socket_ptr s);
        void stop_connection(connection_id id);
        connection_ptr find(connection_id id);
        std::vector<connection_ptr>& get_all();

        static ConnectionPool& get_pool();

    private:
        std::vector<connection_ptr> connections{};
    };
};
