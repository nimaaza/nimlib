#include "http.h"
#include "../metrics/metrics_store.h"

#include <sstream>
#include <cassert>

namespace nimlib::Server::Handlers::Http
{
    HttpHandler::HttpHandler()
    {
        auto metrics_handler = [](const Request& request, Response& response, params_t params) -> std::optional<HandlerState>
            {
                auto& metrics_store = nimlib::Server::Metrics::MetricsStore<long>::get_instance();
                auto report = metrics_store.generate_stats_report();

                response.status = 200;
                response.reason = "OK";
                response.headers["content-type"].push_back("text/plain");
                response.body = report;

                return HandlerState::FINISHED_NO_WAIT;
            };

        auto fallback_router = [](const Request& request, Response& response, params_t params) -> std::optional<HandlerState>
            {
                response.status = 404;
                response.reason = "Not found";
                response.headers["content-type"].push_back("text/html; charset=UTF-8");
                response.body = "not found";

                return HandlerState::FINISHED_NO_WAIT;
            };

        router.get("/metrics", metrics_handler);
        router.get("/", fallback_router);
        router.serve_static_big("/files/bird", "/Users/nimaazarbayjani/Documents/CODE/nimlib/cmake-build-debug/img.jpg");
        router.serve_static_big("/files/big", "/Users/nimaazarbayjani/Movies/Tafrigh-1080.mp4");
        router.fallback(fallback_router);
    }

    void HttpHandler::notify(Connection& connection, StreamsProvider& streams)
    {
        if (!http_request) // There is new data to parse.
        {
            if (state_manager.set_state(HandlerState::H_HANDLING) == HandlerState::HANDLER_ERROR)
            {
                return;
            }

            auto& connection_source = streams.source();
            http_request = parse_request(connection_source);

            if (http_request) // HTTP request parsed successfully.
            {
                Response http_response;
                std::optional<HandlerState> routing_result = router.route(http_request.value(), http_response);

                if (routing_result && routing_result.value() != HandlerState::INCOMPLETE_INPUT)
                {
                    auto response_opt = parse_response(http_response);
                    if (response_opt)
                    {
                        auto& connection_sink = streams.sink();
                        connection_sink << response_opt.value();
                        state_manager.set_state(routing_result.value());
                    }
                }
                else
                {
                    // Create and send a bad request response.
                    state_manager.set_state(HandlerState::FINISHED_NO_WAIT);
                }
            }
            else
            {
                // Create and send a bad request response.
                state_manager.set_state(HandlerState::FINISHED_NO_WAIT);
            }
        }
        else // This is an existing request for further processing.
        {
            // This branch should only be taken after the handler state has
            // transitioned to INCOMPLETE_INPUT or RECALL. This means, there
            // should be a parsed HTTP request already available.
            auto state = state_manager.get_state();
            assert(http_request);

            Response http_response;
            std::optional<HandlerState> routing_result = router.route(http_request.value(), http_response);

            if (routing_result && routing_result.value() != HandlerState::HANDLER_ERROR)
            {
                auto& connection_sink = streams.sink();
                connection_sink << http_response.body;
                state_manager.set_state(routing_result.value());
            }
        }

        auto state = state_manager.get_state();
        if (state == HandlerState::FINISHED_NO_WAIT
            || state == HandlerState::FINISHED_WAIT
            || state == HandlerState::HANDLER_ERROR)
        {
            http_request = std::nullopt;
        }

        connection.notify(*this);
    }

    void HttpHandler::notify(Handler& handler, Connection& connection, StreamsProvider& streams)
    {
        if (!http_request) // There is new data to parse.
        {
            if (state_manager.set_state(HandlerState::H_HANDLING) == HandlerState::HANDLER_ERROR)
            {
                return;
            }

            auto& tls_source = streams.source();
            http_request = parse_request(tls_source);

            if (http_request) // HTTP request parsed successfully.
            {
                Response http_response;
                std::optional<HandlerState> routing_result = router.route(http_request.value(), http_response);

                if (routing_result && routing_result.value() != HandlerState::INCOMPLETE_INPUT)
                {
                    auto response_opt = parse_response(http_response);
                    if (response_opt)
                    {
                        auto& tls_sink = streams.sink();
                        tls_sink << response_opt.value();
                        state_manager.set_state(routing_result.value());
                    }
                }
                else
                {
                    // Create and send a bad request response.
                    state_manager.set_state(HandlerState::FINISHED_NO_WAIT);
                }
            }
            else
            {
                // Create and send a bad request response.
                state_manager.set_state(HandlerState::FINISHED_NO_WAIT);
            }
        }
        else // This is an existing request for further processing.
        {
            // This branch should only be taken after the handler state has
            // transitioned to INCOMPLETE_INPUT or RECALL. This means, there
            // should be a parsed HTTP request already available.
            auto state = state_manager.get_state();
            // assert(state == HandlerState::RECALL || state == HandlerState::INCOMPLETE_INPUT);
            assert(http_request);

            Response http_response;
            std::optional<HandlerState> routing_result = router.route(http_request.value(), http_response);

            if (routing_result && routing_result.value() != HandlerState::HANDLER_ERROR)
            {
                auto& tls_sink = streams.sink();
                tls_sink << http_response.body;
                state_manager.set_state(routing_result.value());
            }
        }

        auto state = state_manager.get_state();
        if (state == HandlerState::FINISHED_NO_WAIT
            || state == HandlerState::FINISHED_WAIT
            || state == HandlerState::HANDLER_ERROR)
        {
            http_request = std::nullopt;
        }

        handler.notify(*this, connection, streams);
    }

    bool HttpHandler::wants_more_bytes()
    {
        return state_manager.get_state() == HandlerState::INCOMPLETE_INPUT;
    }

    bool HttpHandler::wants_to_write()
    {
        return state_manager.get_state() == HandlerState::FINISHED_NO_WAIT;
    }

    bool HttpHandler::wants_to_live()
    {
        return state_manager.get_state() == HandlerState::FINISHED_WAIT;
    }

    bool HttpHandler::wants_to_be_calledback()
    {
        return state_manager.get_state() == HandlerState::RECALL;
    }

    HandlerState HttpHandler::get_state() { return state_manager.get_state(); }
};
