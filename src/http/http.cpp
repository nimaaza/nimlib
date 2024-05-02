#include "http.h"

#include <sstream>

namespace nimlib::Server::Handlers
{
	void Http::notify(Connection& connection, StreamsProvider& streams)
	{
		std::stringstream& out{ streams.sink() };
		out << "done";
		connection.notify(*this);
	}

	void Http::notify(Handler& handler, Connection& connection, StreamsProvider& streams)
	{
		if (handler_state.set_state(HandlerState::H_HANDLING) == HandlerState::HANDLER_ERROR)
		{
			return;
		}

		if (!http_request)
		{
			// This is a newly accepted request which has not been parsed.
			std::stringstream& input_from_tls{ streams.source() };
			http_request = std::move(parse_http_request(input_from_tls));
			if (http_request)
			{
				// TODO: HTTP request has been parsed successfully. Moving on.
				HttpRouter router{};
				auto http_response = parse_http_response(router.route(http_request.value()));
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

	bool Http::wants_more_bytes()
	{
		return handler_state.get_state() == HandlerState::INCOMPLETE;
	}

	bool Http::wants_to_write()
	{
		auto state = handler_state.get_state();
		return state == HandlerState::WRITE_AND_DIE || state == HandlerState::WRITE_AND_WAIT;
	}

	bool Http::wants_to_live()
	{
		return handler_state.get_state() == HandlerState::WRITE_AND_WAIT;
	}
};
