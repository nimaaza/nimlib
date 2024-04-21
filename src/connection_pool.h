#pragma once

#include "common/types.h"

#include <vector>

#include "common/types.h"

namespace nimlib::Server
{
    class ConnectionPool
    {
    private:
        ConnectionPool();

    public:

        ~ConnectionPool();

        ConnectionPool(const ConnectionPool&) = delete;
        ConnectionPool& operator=(const ConnectionPool&) = delete;
        ConnectionPool(ConnectionPool&&) = delete;
        ConnectionPool& operator=(ConnectionPool&&) = delete;

        void record_connection(socket_ptr s);
        connection_ptr find(connection_id id) const;
        const std::vector<connection_ptr>& get_all() const;
        void clean_up();

        static ConnectionPool& get_pool();

    private:
        std::vector<connection_ptr> connections{};
    };
};
