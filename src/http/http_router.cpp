#include "http_router.h"

#include "fstream"
#include "iostream"

namespace nimlib::Server::Protocols
{
    HttpRouter::HttpRouter() = default;

    HttpRouter::~HttpRouter() = default;

    HttpResponse HttpRouter::route(HttpRequest& http_request)
    {
        if (http_request.target == "/files/img.jpg")
        {
            std::string contents;
            std::ifstream in("./img.jpg", std::ios::in | std::ios::binary);
            if (in)
            {
                in.seekg(0, std::ios::end);
                contents.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&contents[0], contents.size());
                in.close();
            }

            HttpResponse http_response{};
            http_response.status = 200;
            http_response.reason = "OK";
            http_response.headers["content-type"].push_back("image/jpeg");
            http_response.body = contents;
            return http_response;
        }
        else
        {
            HttpResponse http_response{};
            http_response.status = 404;
            http_response.reason = "Not found";
            http_response.headers["content-type"].push_back("text/html; charset=UTF-8");
            http_response.body = "not found";
            return http_response;
        }
    }
}
