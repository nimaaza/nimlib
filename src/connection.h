#pragma once

#include <sstream>
#include <chrono>

#include "common/types.h"
#include "utils/state_manager.h"
//#include "metrics/measurements.h"

namespace nimlib::Server
{
    using nimlib::Server::Types::StreamsProviderInterface;
    using nimlib::Server::Types::ConnectionInterface;
    using nimlib::Server::Types::TcpSocketInterface;
    using nimlib::Server::Types::ProtocolInterface;
    using nimlib::Server::Constants::ServerDirective;
    using nimlib::Server::Constants::ConnectionState;

    class Connection : public ConnectionInterface, public StreamsProviderInterface
    {
    public:
        Connection(std::unique_ptr<TcpSocketInterface>, connection_id, size_t buffer_size = 10240);
        ~Connection();

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        Connection(Connection&&) noexcept = delete;
        Connection& operator=(Connection&&) noexcept = delete;

        void notify(ServerDirective directive) override;
        void notify(ProtocolInterface& protocol) override;
        void set_protocol(std::shared_ptr<ProtocolInterface>) override;
        void halt() override;
        ConnectionState get_state() override;
        const int get_id() const override;

        std::stringstream& get_input_stream() override;
        std::stringstream& get_output_stream() override;

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
        std::unique_ptr<TcpSocketInterface> socket;
        std::shared_ptr<ProtocolInterface> protocol;
        // nimlib::Server::Metrics::Measurements::Duration<long> response_timer;

        static const std::unordered_map<ConnectionState, std::vector<ConnectionState>> states_transition_map;
        static const std::unordered_map<ConnectionState, long> state_time_outs;
    };
};
