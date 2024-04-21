#pragma once

#include "http_parser.h"

namespace nimlib::Server::Protocols
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
    };
};
