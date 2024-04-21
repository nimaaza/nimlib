#pragma once

#include "../common/types.h"

#include <unordered_map>
#include <vector>
#include <optional>

namespace nimlib::Server::Handlers
{
    struct HttpRequest
    {
        HttpRequest(
            const std::string method,
            const std::string target,
            const std::string version,
            const headers_t headers,
            const std::string body
        );
        ~HttpRequest() = default;

        HttpRequest(const HttpRequest&) = delete;
        HttpRequest& operator=(const HttpRequest&) = delete;
        HttpRequest(HttpRequest&& other) noexcept = default;
        HttpRequest& operator=(HttpRequest&&) noexcept = default;

        std::string method;
        std::string target;
        std::string version;
        headers_t headers;
        std::string body;
    };

    struct HttpResponse
    {
        HttpResponse() = default;
        ~HttpResponse() = default;

        HttpResponse(const HttpResponse&) = delete;
        HttpResponse& operator=(const HttpResponse&) = delete;
        HttpResponse(HttpResponse&& other) noexcept = default;
        HttpResponse& operator=(HttpResponse&&) noexcept = default;

        std::string version{ "HTTP/1.1" };
        short status;
        std::string reason;
        headers_t headers;
        std::string body;
    };

    std::optional<HttpRequest> parse_http_request(std::stringstream& input_stream);
    std::optional<std::string> parse_http_response(const HttpResponse& http_response);
    bool white_space(char c);
    bool validate_method(const std::string& method);
    bool validate_target(const std::string& target);
    bool validate_version(const std::string& version);
    int body_length(const headers_t& headers);
};
