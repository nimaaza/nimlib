#include "http.h"

#include "http_parser.h"

#include <sstream>

namespace nimlib::Server::Protocols
{
	Http::Http(ConnectionInterface& connection)
		: connection{ connection }, http_request{ std::nullopt }
	{}

	Http::~Http() = default;

	void Http::notify(ConnectionInterface& connection, StreamsProviderInterface& streams)
	{
		std::stringstream& out{ streams.get_output_stream() };
		out << "done";
		connection.notify(*this);
	}

	void Http::notify(
		ProtocolInterface& protocol,
		ConnectionInterface& connection,
		StreamsProviderInterface& streams
	)
	{
		if (!http_request)
		{
			// This is a newly accepted request which has not been parsed.
			std::stringstream& input_from_tls{ streams.get_input_stream() };
			http_request = std::move(parse_http_message(input_from_tls));
			if (http_request)
			{
				// TODO: HTTP request has been parsed successfully. Moving on.
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

		std::stringstream& output_to_tls{ streams.get_output_stream() };
		output_to_tls << "done_all";
		protocol.notify(*this, connection, streams);
	}

	bool Http::wants_more_bytes()
	{
		return false;
	}

	bool Http::wants_to_write()
	{
		return true;
	}

	bool Http::wants_to_live()
	{
		if (http_request)
		{
			const auto& headers = http_request.value().headers;
			auto it = headers.find("connection");
			if (it != headers.end())
			{
				const auto& values = it->second;
				return std::find(values.begin(), values.end(), "keep-alive") != values.end();
			}
			else
			{
				return false;
			}
		}
		else
		{
			return  false;
		}
	}
};
