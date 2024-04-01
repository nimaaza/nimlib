#pragma once

#include <sstream>
#include <chrono>

#include "types.h"
#include "state_manager.h"
//#include "metrics/measurements.h"

namespace nimlib::Server
{
    using nimlib::Server::Types::ConnectionInterface;
    using nimlib::Server::Types::TcpSocketInterface;
    using nimlib::Server::Types::ProtocolInterface;
    using nimlib::Server::Constants::ConnectionState;

    class Connection : public ConnectionInterface
    {
    public:
        Connection(std::unique_ptr<TcpSocketInterface>, connection_id, size_t buffer_size = 10240);
        ~Connection();

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        Connection(Connection&&) noexcept = delete;
        Connection& operator=(Connection&&) noexcept = delete;

        ConnectionState read() override;
        ConnectionState write() override;
        void halt() override;
        // Connection& operator<<(uint8_t c) override;
        // Connection& operator<<(std::string& s) override;
        std::stringstream& get_input_stream() override;
        std::stringstream& get_output_stream() override;
        void notify(ProtocolInterface& protocol) override;
        void set_protocol(std::shared_ptr<ProtocolInterface>) override;
        std::pair<ConnectionState, long> get_state() const override;
        const int get_id() const override;

    private:
        const connection_id id;
        size_t buffer_size;
        bool keep_alive{ false };
        StateManager<ConnectionState> connection_state{
            ConnectionState::STARTING,
            ConnectionState::CON_ERROR,
            nimlib::Server::Constants::MAX_RESET_COUNT
        };
        std::stringstream request_stream{};
        std::stringstream response_stream{};
        std::unique_ptr<TcpSocketInterface> socket;
        std::shared_ptr<ProtocolInterface> protocol;
        // nimlib::Server::Metrics::Measurements::Duration<long> response_timer;
    };
};
