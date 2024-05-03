#pragma once

#include "http_parser.h"
#include "http_router.h"
#include "../utils/state_manager.h"
#include "../common/types.h"

using nimlib::Server::Constants::HandlerState;
using nimlib::Server::Utils::StateManager;
using nimlib::Server::Types::Connection;
using nimlib::Server::Types::StreamsProvider;

namespace nimlib::Server::Handlers
{
    using nimlib::Server::Types::Handler;
    using nimlib::Server::Constants::HandlerState;
    class HttpRequest;

    class Http : public Handler
    {
    public:
        Http() = default;
        ~Http() = default;

        void notify(Connection& connection, StreamsProvider& streams) override;
        void notify(Handler& handler, Connection& connection, StreamsProvider& streams) override;
        bool wants_more_bytes() override;
        bool wants_to_write() override;
        bool wants_to_live() override;

    private:
        std::optional<HttpRequest> http_request{ std::nullopt };
        std::optional<HttpResponse> htt_response{ std::nullopt };
        HttpRouter router{};
        StateManager<HandlerState> handler_state
        {
            HandlerState::READY_TO_HANDLE,
            HandlerState::HANDLER_ERROR,
            Http::transition_map,
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
