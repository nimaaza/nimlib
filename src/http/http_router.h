#pragma once

#include "http_parser.h"

namespace nimlib::Server::Handlers
{
    class HttpRouter
    {
    public:
        HttpRouter();
        ~HttpRouter();

        HttpRouter(const HttpRouter&) = delete;
        HttpRouter& operator=(const HttpRouter&) = delete;
        HttpRouter(HttpRouter&&) noexcept = delete;
        HttpRouter& operator=(HttpRouter&&) noexcept = delete;

        HttpResponse route(HttpRequest& http_request);
        // std::string read_next_bytes(int size, int seq);
    };
};
