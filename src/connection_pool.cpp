#include "connection_pool.h"
#include "connection.h"
#include "tls/tls_layer.h"
#include "http/http.h"
#include "common/common.h"

#include <memory>
#include <cassert>

using nimlib::Server::Connection;
using nimlib::Server::ConnectionState;

namespace nimlib::Server
{
    ConnectionPool::ConnectionPool()
    {
        connections.resize(65'000);
    }

    ConnectionPool::~ConnectionPool() = default;

    void ConnectionPool::record_connection(socket_ptr s)
    {
        assert(connections[s->get_tcp_socket_descriptor()] == nullptr);
        connection_id id = s->get_tcp_socket_descriptor();
        auto connection = std::make_shared<Connection>(std::move(s), id);
        auto http_protocol = std::make_shared<nimlib::Server::Protocols::Http>(*connection);
        auto protocol = std::make_shared<nimlib::Server::Protocols::TlsLayer>(
            *connection,
            *connection,
            http_protocol
        );
        connection->set_protocol(protocol);
        connections[id] = connection;
    }

    connection_ptr ConnectionPool::find(connection_id id) const
    {
        return connections[id];
    }

    const std::vector<connection_ptr>& ConnectionPool::get_all() const
    {
        return connections;
    }

    void ConnectionPool::clean_up()
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

    ConnectionPool& ConnectionPool::get_pool()
    {
        static ConnectionPool connection_pool{};
        return connection_pool;
    };
}
