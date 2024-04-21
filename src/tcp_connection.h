#pragma once

#include <sstream>
#include <chrono>

#include "common/types.h"
#include "utils/state_manager.h"
//#include "metrics/measurements.h"

namespace nimlib::Server
{
    using nimlib::Server::Types::StreamsProvider;
    using nimlib::Server::Types::Connection;
    using nimlib::Server::Types::Socket;
    using nimlib::Server::Types::Handler;
    using nimlib::Server::Constants::ServerDirective;
    using nimlib::Server::Constants::ConnectionState;

    class TcpConnection : public Connection, public StreamsProvider
    {
    public:
        TcpConnection(std::unique_ptr<Socket>, connection_id, size_t buffer_size = 10240);
        ~TcpConnection();

        TcpConnection(const TcpConnection&) = delete;
        TcpConnection& operator=(const TcpConnection&) = delete;
        TcpConnection(TcpConnection&&) noexcept = delete;
        TcpConnection& operator=(TcpConnection&&) noexcept = delete;

        void notify(ServerDirective directive) override;
        void notify(Handler& handler) override;
        void set_handler(std::shared_ptr<Handler>) override;
        void halt() override;
        ConnectionState get_state() override;
        const int get_id() const override;

        std::stringstream& source() override;
        std::stringstream& sink() override;

    private:
        ConnectionState read();
        ConnectionState write();

    private:
        const connection_id id;
        size_t buffer_size;
        bool keep_alive{ false };
        StateManager<ConnectionState> connection_state{
            ConnectionState::STARTING,
            ConnectionState::CON_ERROR,
            states_transition_map,
            nimlib::Server::Constants::MAX_RESET_COUNT,
            state_time_outs
        };
        std::stringstream input_stream{};
        std::stringstream output_stream{};
        std::unique_ptr<Socket> socket;
        std::shared_ptr<Handler> handler;
        // nimlib::Server::Metrics::Measurements::Duration<long> response_timer;

        static const std::unordered_map<ConnectionState, std::vector<ConnectionState>> states_transition_map;
        static const std::unordered_map<ConnectionState, long> state_time_outs;
    };
};
