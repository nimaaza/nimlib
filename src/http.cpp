#include "http.h"

#include <sstream>
#include <iostream>

namespace nimlib::Server::Protocols
{
	Http::Http(ConnectionInterface& connection)
		: connection{ connection }
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
		std::stringstream& input_from_tls{ streams.get_input_stream() };

		auto a = reinterpret_cast<StreamsProviderInterface*>(&connection);
		a->get_input_stream();
		if (auto http_request = parse_http_message(input_from_tls))
		{
			// success in http parsing
		}
		else
		{
			// there's something wrong with the request, it's a bad request?
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
		return false;
	}

};

namespace nimlib::Server::Protocols
{
	header_validator Http::content_length_validator = [](const std::string& value) -> bool { return true; };

	const std::unordered_map<std::string, header_validator> Http::header_validators{
		{"content-length", content_length_validator}
	};

	HttpRequest::HttpRequest(
		std::string method,
		std::string target,
		std::string version,
		std::unordered_map<std::string, std::string> headers,
		std::string body
	) :
		method{ std::move(method) },
		target{ std::move(target) },
		version{ std::move(version) },
		headers{ std::move(headers) },
		body{ std::move(body) }
	{}

	HttpRequest::HttpRequest(HttpRequest&& other) noexcept
		:
		method{ std::move(other.method) },
		target{ std::move(other.target) },
		version{ std::move(other.version) },
		headers{ std::move(other.headers) },
		body{ std::move(other.body) }
	{}

	std::optional<HttpRequest> Http::parse_http_message(std::stringstream& input_stream)
	{
		std::string method;
		std::string target;
		std::string version;
		std::unordered_map<std::string, std::string> headers;
		std::string body;
		std::string line;

		auto request_line_tokenizer = [&](const std::string& line) -> bool
			{
				std::stringstream first_line_stream{ line };
				first_line_stream >> method >> target >> version;
				return true; // validate_verb(method) && ...;
			};

		auto header_tokenizer = [&](const std::string& line) -> bool
			{
				auto colon_pos = line.find(':');
				if (colon_pos > line.size())
				{
					std::cout << "error in headers, no colon" << std::endl;
					return false;
				}

				auto header_end = colon_pos;
				while (white_space(line[header_end])) header_end--;
				auto header = line.substr(0, header_end);
				for (auto& c : header) c = std::tolower(c);

				auto value_pos = colon_pos + 1;
				while (white_space(line[value_pos])) value_pos++;
				auto value = line.substr(value_pos);

				headers[header] = value; // TODO: handle repeated headers
				return true;
			};

		std::getline(input_stream, line, '\r');
		if (input_stream.peek() == '\n')
		{
			input_stream.get();
		}
		else
		{
			return {};
		}

		request_line_tokenizer(line);

		bool empty_line_found = false;
		while (std::getline(input_stream, line, '\r'))
		{
			if (input_stream.peek() == '\n')
			{
				input_stream.get();
			}
			else
			{
				return {};
			}

			if (line.size() == 0)
			{
				empty_line_found = true;
				break;
			}

			header_tokenizer(line);
		}

		if (empty_line_found)
		{
			std::getline(input_stream, body, '\0'); // TODO: this can fail. A better solution when content-size is known.
			return HttpRequest(std::move(method), std::move(target), std::move(version), std::move(headers), std::move(body));
		}
		else
		{
			std::cout << "http request has no empty line" << std::endl;
			return {};
		}
	}

	bool Http::white_space(char c) { return c == ' ' || c == '\t'; }
}
