#pragma once

#include "../common/types.h"

#include <unordered_map>
#include <vector>
#include <optional>

namespace nimlib::Server::Handlers::Http
{
    struct Request
    {
        Request(
            const std::string method,
            const std::string target,
            const std::string version,
            const headers_t headers,
            const std::string body
        );
        Request() = default;
        ~Request() = default;

        Request(const Request&) = delete;
        Request& operator=(const Request&) = delete;
        Request(Request&& other) noexcept = default;
        Request& operator=(Request&&) noexcept = default;

        std::string method;
        std::string target;
        std::string version;
        headers_t headers;
        std::string body;
    };

    struct Response
    {
        Response() = default;
        ~Response() = default;

        Response(const Response&) = delete;
        Response& operator=(const Response&) = delete;
        Response(Response&& other) noexcept = default;
        Response& operator=(Response&&) noexcept = default;

        std::string version{ "HTTP/1.1" };
        short status;
        std::string reason;
        headers_t headers;
        std::string body;
    };

    std::optional<Request> parse_request(std::stringstream& input_stream);
    std::optional<std::string> parse_response(const Response& http_response);
    bool white_space(char c);
    bool validate_method(const std::string& method);
    bool validate_target(const std::string& target);
    bool validate_version(const std::string& version);
    int body_length(const headers_t& headers);
};
