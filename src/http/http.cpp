#include "http.h"
#include "../metrics/metrics_store.h"

#include <sstream>

namespace nimlib::Server::Handlers::Http
{
	HttpHandler::HttpHandler()
	{
		auto metrics_handler = [](const Request& request, Response& response, params_t params) -> void
			{
				auto& metrics_store = nimlib::Server::Metrics::MetricsStore<long>::get_instance();
				auto report = metrics_store.generate_stats_report();

				response.status = 200;
				response.reason = "OK";
				response.headers["content-type"].push_back("text/plain");
				response.body = report;
			};

		auto fallback_router = [](const Request& request, Response& response, params_t params)
			{
				response.status = 404;
				response.reason = "Not found";
				response.headers["content-type"].push_back("text/html; charset=UTF-8");
				response.body = "not found";
			};

		router.get("/metrics", metrics_handler);
		router.fallback(fallback_router);
	}

	void HttpHandler::notify(Connection& connection, StreamsProvider& streams)
	{
		std::stringstream& out{ streams.sink() };
		out << "done";
		connection.notify(*this);
	}

	void HttpHandler::notify(Handler& handler, Connection& connection, StreamsProvider& streams)
	{
		if (handler_state.set_state(HandlerState::H_HANDLING) == HandlerState::HANDLER_ERROR)
		{
			return;
		}

		if (!http_request)
		{
			// This is a newly accepted request which has not been parsed.
			std::stringstream& input_from_tls{ streams.source() };
			http_request = std::move(parse_request(input_from_tls));
			if (http_request)
			{
				Response response;
				router.route(http_request.value(), response);
				auto http_response = parse_response(response);
				if (http_response)
				{
					std::stringstream& output_to_tls{ streams.sink() };
					output_to_tls << http_response.value();

					const auto& headers = http_request.value().headers;
					auto it = headers.find("connection");
					if (it != headers.end())
					{
						const auto& values = it->second;
						if (std::find(values.begin(), values.end(), "keep-alive") != values.end())
						{
							handler_state.set_state(HandlerState::WRITE_AND_WAIT);
						}
						else
						{
							handler_state.set_state(HandlerState::WRITE_AND_DIE);
						}
					}
					else
					{
						handler_state.set_state(HandlerState::WRITE_AND_DIE);
					}

					handler.notify(*this, connection, streams);
				}
			}
			else
			{
				// TODO: HTTP request is not valid. Maybe send an error response and close connection.
			}
		}
		else
		{
			// This is an existing connection that has more data to be processed.
			// TODO
		}
	}

	bool HttpHandler::wants_more_bytes()
	{
		return handler_state.get_state() == HandlerState::INCOMPLETE;
	}

	bool HttpHandler::wants_to_write()
	{
		auto state = handler_state.get_state();
		return state == HandlerState::WRITE_AND_DIE || state == HandlerState::WRITE_AND_WAIT;
	}

	bool HttpHandler::wants_to_live()
	{
		return handler_state.get_state() == HandlerState::WRITE_AND_WAIT;
	}
};
