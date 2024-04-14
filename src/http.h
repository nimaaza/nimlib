#pragma once

#include "common/types.h"

#include <unordered_map>
#include <optional>

using nimlib::Server::Types::ConnectionInterface;
using nimlib::Server::Types::StreamsProviderInterface;

namespace nimlib::Server::Protocols
{
	using nimlib::Server::Types::ProtocolInterface;
	using ParseResult = nimlib::Server::Constants::ParseResult;

	struct HttpRequest
	{
		HttpRequest(
			std::string method,
			std::string target,
			std::string version,
			std::unordered_map<std::string, std::string> headers,
			std::string body
		);
		~HttpRequest() = default;

		HttpRequest(const HttpRequest&) = delete;
		HttpRequest& operator=(const HttpRequest&) = delete;
		HttpRequest(HttpRequest&& other) noexcept;
		HttpRequest& operator=(HttpRequest&&) noexcept = delete;

		const std::string method;
		const std::string target;
		const std::string version;
		const std::unordered_map<std::string, std::string> headers;
		const std::string body;
	};

	class Http : public ProtocolInterface
	{
	public:
		Http(ConnectionInterface& connection);
		~Http();

		void notify(ConnectionInterface& connection, StreamsProviderInterface& streams) override;
		void notify(ProtocolInterface& protocol, ConnectionInterface& connection, StreamsProviderInterface& streams) override;
		bool wants_more_bytes() override;
		bool wants_to_write() override;
		bool wants_to_live() override;

	private:
		std::optional<HttpRequest> parse_http_message(std::stringstream& input_stream);
		bool white_space(char c);

	private:
		ConnectionInterface& connection;

		static header_validator content_length_validator;
		static const std::unordered_map<std::string, header_validator> header_validators;
	};
};
