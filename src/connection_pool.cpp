#include "connection_pool.h"
#include "connection.h"

#include <memory>
#include <cassert>

using nimlib::Server::Connection;

namespace nimlib::Server
{
    ConnectionPool::ConnectionPool(/* args */)
    {
        connections.resize(65'000);
    }

    ConnectionPool::~ConnectionPool() {}

    void ConnectionPool::record_connection(connection_ptr c)
    {
        assert(connections[c->get_id()] == nullptr);
        connections[c->get_id()] = c;  // TODO: maybe I can use at here.
    }

    void ConnectionPool::record_connection(socket_ptr s)
    {
        assert(connections[s->get_tcp_socket_descriptor()] == nullptr);
        connection_id id = s->get_tcp_socket_descriptor();
        auto connection = std::make_shared<Connection>(std::move(s), id);
        connections[id] = connection;
    }

    void ConnectionPool::stop_connection(connection_id id)
    {
        if (connections[id])
        {
            connections[id]->halt();
        }
    }

    connection_ptr ConnectionPool::find(connection_id id)
    {
        return connections[id];
    }

    std::vector<connection_ptr>& ConnectionPool::get_all()
    {
        return connections;
    }

    ConnectionPool& ConnectionPool::get_pool()
    {
        static ConnectionPool connection_pool{};
        return connection_pool;
    };
}
