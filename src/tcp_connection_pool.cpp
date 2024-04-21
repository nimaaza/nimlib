#include "tcp_connection_pool.h"
#include "tcp_connection.h"
#include "tls/tls_layer.h"
#include "http/http.h"
#include "common/common.h"

#include <memory>
#include <cassert>

using nimlib::Server::TcpConnection;
using nimlib::Server::ConnectionState;

namespace nimlib::Server
{
    TcpConnectionPool::TcpConnectionPool()
    {
        connections.resize(65'000);
    }

    TcpConnectionPool::~TcpConnectionPool() = default;

    void TcpConnectionPool::record_connection(socket_ptr s)
    {
        assert(connections[s->get_tcp_socket_descriptor()] == nullptr);
        connection_id id = s->get_tcp_socket_descriptor();
        auto connection = std::make_shared<TcpConnection>(std::move(s), id);
        auto http_handler = std::make_shared<nimlib::Server::Handlers::Http>(*connection);
        auto tls_handler = std::make_shared<nimlib::Server::Handlers::TlsLayer>(
            *connection,
            *connection,
            http_handler
        );
        connection->set_handler(tls_handler);
        connections[id] = connection;
    }

    connection_ptr TcpConnectionPool::find(connection_id id) const
    {
        return connections[id];
    }

    const std::vector<connection_ptr>& TcpConnectionPool::get_all() const
    {
        return connections;
    }

    void TcpConnectionPool::clean_up()
    {
        for (auto& connection : connections)
        {
            if (connection)
            {
                auto connection_state = connection->get_state();
                if (connection_state == ConnectionState::CON_ERROR || connection_state == ConnectionState::DONE)
                {
                    connection.reset();
                    connection = nullptr;
                }
            }
        }
    }

    TcpConnectionPool& TcpConnectionPool::get_pool()
    {
        static TcpConnectionPool connection_pool{};
        return connection_pool;
    };
}
