#include "http_parser.h"

#include "../utils/helpers.h"

#include "iostream"

namespace nimlib::Server::Protocols
{
    header_validator content_length_validator =
        [](const std::string& value, const std::unordered_map<std::string, std::vector<std::string>>& headers) -> bool
        {
            // A sender MUST NOT send a Content-Length header field in any message
            // that contains a Transfer-Encoding header field.
            if (auto it = headers.find("transfer-encoding"); it != headers.end()) return false;

            // If a message is received without Transfer-Encoding and with
            // either multiple Content-Length header fields having differing
            // field-values or a single Content-Length header field having an
            // invalid value, then the message framing is invalid and the
            // recipient MUST treat it as an unrecoverable error.
            if (auto it = headers.find("content-length"); it != headers.end()) return false;

            // Content-Length must be an integer and only an integer.
            std::stringstream content_length_s{ value };
            int content_length_i;
            content_length_s >> content_length_i;
            return content_length_s.eof();
        };

    header_validator transfer_encoding_validator =
        [](const std::string& value, const std::unordered_map<std::string, std::vector<std::string>>& headers) -> bool
        {
            // A sender MUST NOT send a Content-Length header field in any message
            // that contains a Transfer-Encoding header field.
            if (auto it = headers.find("content-length"); it != headers.end()) return false;
            return true;
        };

    const std::unordered_map<std::string, header_validator> header_validators
        {
            {"content-length", content_length_validator},
            {"transfer-encoding", transfer_encoding_validator}
        };

    HttpRequest::HttpRequest(
        std::string method,
        std::string target,
        std::string version,
        headers_t headers,
        std::string body
    ) :
        method{ std::move(method) },
        target{ std::move(target) },
        version{ std::move(version) },
        headers{ std::move(headers) },
        body{ std::move(body) }
    {}

    std::optional<HttpRequest> parse_http_message(std::stringstream& input_stream)
    {
        std::string method;
        std::string target;
        std::string version;
        headers_t headers;
        std::string body;
        std::string line;

        auto request_line_tokenizer = [&](const std::string& line) -> bool
        {
            std::stringstream first_line_stream{ line };
            first_line_stream >> method >> target >> version;
            return first_line_stream.eof() && validate_method(method) && validate_target(target) && validate_version(version);
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
            auto header = line.substr(0, header_end);
            // There should be no white space in header name
            // Also, header names are case-insensitive
            for (auto& c : header)
            {
                if (white_space(c)) return false;
                c = std::tolower(c);
            }

            auto value_pos_begin = colon_pos + 1;
            auto value_pos_end = line.size() - 1;
            while (white_space(line[value_pos_begin])) value_pos_begin++;
            while (white_space(line[value_pos_end])) value_pos_end--;
            auto value = line.substr(value_pos_begin, value_pos_end - value_pos_begin + 1);

            if (auto it = header_validators.find(header); it != header_validators.end())
            {
                if (!it->second(value, headers)) return false;
            }

            std::vector<std::string_view> values{};
            split(value, ",", values);
            for (auto v : values)
            {
                std::string value_string{ v };
                headers[header].push_back(value_string);
            }

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

        if (!request_line_tokenizer(line)) return {};

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

            if (!header_tokenizer(line)) return {};
        }

        if (empty_line_found)
        {
            // Read message body only if headers indicate the existence of it.
            if (body_length(headers) > 0) std::getline(input_stream, body, '\0');
            return std::move(HttpRequest(std::move(method), std::move(target), std::move(version), std::move(headers), std::move(body)));
        }
        else
        {
            std::cout << "http request has no empty line" << std::endl;
            return {};
        }
    }

    bool white_space(char c) { return c == ' ' || c == '\t'; }

    bool validate_method(const std::string& method)
    {
        return method == "GET"
            || method == "POST"
            || method == "PUT"
            || method == "DELETE"
            || method == "PATCH"
            || method == "HEAD"
            || method == "OPTIONS"
            || method == "TRACE"
            || method == "CONNECT";
    }

    bool validate_target(const std::string& target) { return true; /* TODO */ }

    bool validate_version(const std::string& version) { return version == "HTTP/1.1"; }

    int body_length(const headers_t& headers) { return 1; }
}
