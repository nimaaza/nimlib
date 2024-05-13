#pragma once

#include "parser.h"
#include "router.h"
#include "../utils/state_manager.h"
#include "../common/types.h"

using nimlib::Server::Constants::HandlerState;
using nimlib::Server::Utils::StateManager;
using nimlib::Server::Types::Connection;
using nimlib::Server::Types::StreamsProvider;

namespace nimlib::Server::Handlers::Http
{
    using nimlib::Server::Types::Handler;
    using nimlib::Server::Constants::HandlerState;
    class Request;

    class HttpHandler : public Handler
    {
    public:
        HttpHandler();
        ~HttpHandler() = default;

        void notify(Connection& connection, StreamsProvider& streams) override;
        void notify(Handler& handler, Connection& connection, StreamsProvider& streams) override;
        bool wants_more_bytes() override;
        bool wants_to_write() override;
        bool wants_to_live() override;

    private:
        std::optional<Request> http_request{ std::nullopt };
        std::optional<Response> htt_response{ std::nullopt };
        Router router{};
        StateManager<HandlerState> handler_state
        {
            HandlerState::READY_TO_HANDLE,
            HandlerState::HANDLER_ERROR,
            HttpHandler::transition_map,
            max_reset_counts,
            time_outs
        };
        inline static const std::unordered_map<HandlerState, std::vector<HandlerState>> transition_map
        {
            {HandlerState::READY_TO_HANDLE, {HandlerState::H_HANDLING}},
            {HandlerState::H_HANDLING, {HandlerState::WRITE_AND_WAIT, HandlerState::WRITE_AND_DIE, HandlerState::INCOMPLETE}},
            {HandlerState::WRITE_AND_DIE, {}},
            {HandlerState::WRITE_AND_WAIT, {HandlerState::H_HANDLING}},
            {HandlerState::INCOMPLETE, {HandlerState::H_HANDLING}},
        };
        inline static const std::unordered_map<HandlerState, int> max_reset_counts{};
        inline static const std::unordered_map<HandlerState, long> time_outs{};
    };
};
