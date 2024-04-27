#pragma once

#include "common/types.h"

#include <vector>

namespace nimlib::Server
{
    class TcpConnectionPool
    {
    private:
        TcpConnectionPool();

    public:

        ~TcpConnectionPool();

        TcpConnectionPool(const TcpConnectionPool&) = delete;
        TcpConnectionPool& operator=(const TcpConnectionPool&) = delete;
        TcpConnectionPool(TcpConnectionPool&&) = delete;
        TcpConnectionPool& operator=(TcpConnectionPool&&) = delete;

        void record_connection(socket_ptr s);
        connection_ptr find(connection_id id) const;
        const std::vector<connection_ptr>& get_all() const;
        void clean_up();

        static TcpConnectionPool& get_pool();

    private:
        std::vector<connection_ptr> connections{};
        const int CONNECTIONS = 65'000;
    };
};
