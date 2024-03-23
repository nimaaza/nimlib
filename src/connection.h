#pragma once

#include <sstream>
#include <chrono>

#include "types.h"
#include "socket.h"
#include "state_manager.h"
//#include "metrics/measurements.h"

namespace nimlib::Server
{
	using nimlib::Server::Types::ConnectionInterface;
	using nimlib::Server::Types::ProtocolInterface;
    using nimlib::Server::Constants::ConnectionState;
    using nimlib::Server::Constants::ParseResult;

	class Connection : public ConnectionInterface
    {
    public:
        Connection(std::unique_ptr<Socket>, connection_id);
        ~Connection();

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        Connection(Connection&&) noexcept = delete;
        Connection& operator=(Connection&&) noexcept = delete;

        ConnectionState read() override;
        ConnectionState write() override;
        void halt() override;
        void set_protocol(std::shared_ptr<ProtocolInterface>) override;
        std::pair<ConnectionState, long> get_state() const override;
        const int get_id() const override;

    private:
        ConnectionState handle_incoming_data();

    private:
        const connection_id id;
        StateManager<ConnectionState> sm;
        std::stringstream request_stream;
        std::stringstream response_stream;
        std::unique_ptr<Socket> socket;
        ParseResult parse_result;
        std::shared_ptr<ProtocolInterface> protocol;
//        nimlib::Server::Metrics::Measurements::Duration<long> response_timer;
    };
};
