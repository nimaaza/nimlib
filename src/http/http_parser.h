#pragma once

#include "../common/types.h"

#include <unordered_map>
#include <vector>
#include <optional>

namespace nimlib::Server::Protocols
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

    std::optional<HttpRequest> parse_http_message(std::stringstream& input_stream);
    bool white_space(char c);
    bool validate_method(const std::string& method);
    bool validate_target(const std::string& target);
    bool validate_version(const std::string& version);
    int body_length(const headers_t& headers);
};
