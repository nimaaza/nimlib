#include <gtest/gtest.h>

#include <unordered_map>

#include "../../src/http/router.h"

using nimlib::Server::Handlers::Http::Router;
using nimlib::Server::Handlers::Http::Request;
using nimlib::Server::Handlers::Http::Response;
using route_handler = std::function<void(const Request&, Response&, std::unordered_map<std::string, std::string>&)>;
using params_t = std::unordered_map<std::string, std::string>;

TEST(HttpRouter, EmptyRouteRejected)
{
    Router router{};
    bool handler_invoked = false;

    route_handler handler = [&handler_invoked](const Request&, Response& response, params_t&) -> void
    {
        handler_invoked = true;
        response.body = "touched_by_handler";
    };
    bool handler_accepted = router.get("", handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "";
    bool routing_result = router.route(request, response);

    EXPECT_FALSE(handler_accepted);
    EXPECT_FALSE(routing_result);
    EXPECT_FALSE(handler_invoked);
    EXPECT_EQ(response.body, "");
}

TEST(HttpRouter, RootHandlerSettingAndInvokation)
{
    Router router{};
    bool root_handler_invoked = false;

    route_handler root_handler = [&root_handler_invoked](const Request&, Response& response, params_t&) -> void
    {
        root_handler_invoked = true;
        response.body = "touched_by_root_handler";
    };
    bool handler_accepted = router.get("/", root_handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(root_handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(response.body, "touched_by_root_handler");
}

TEST(HttpRouter, SimpleHandlerSettingAndInvokation)
{
    Router router{};
    bool handler_invoked = false;

    route_handler handler = [&handler_invoked](const Request& request, Response& response, params_t&) -> void
    {
        handler_invoked = true;
        response.body = "touched_by_handler";
    };
    bool handler_accepted = router.get("/target_1/target_2", handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/target_1/target_2";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(response.body, "touched_by_handler");
}

TEST(HttpRouter, ComplexHandlerSettingAndInvokation)
{
    Router router{};

    bool handler_1_1_invoked = false;
    bool handler_1_2_invoked = false;
    bool handler_1_2_1_invoked = false;

    route_handler handler_1_1 = [&handler_1_1_invoked](const Request& request, Response& response, params_t&) -> void
    {
        handler_1_1_invoked = true;
        response.body = "touched_by_handler_1_1";
    };

    route_handler handler_1_2 = [&handler_1_2_invoked](const Request& request, Response& response, params_t&) -> void
    {
        handler_1_2_invoked = true;
        response.body = "touched_by_handler_1_2";
    };

    route_handler
        handler_1_2_1 = [&handler_1_2_1_invoked](const Request& request, Response& response, params_t&) -> void
    {
        handler_1_2_1_invoked = true;
        response.body = "touched_by_handler_1_2_1";
    };

    bool handler_accepted_1_1 = router.get("/target_1/target_1_1", handler_1_1);
    bool handler_accepted_1_2 = router.get("/target_1/target_1_2", handler_1_2);
    bool handler_accepted_1_2_1 = router.get("/target_1/target_1_2/target_1_2_1", handler_1_2_1);

    Request request_1_1{};
    Request request_1_2{};
    Request request_1_2_1{};
    Response response_1_1{};
    Response response_1_2{};
    Response response_1_2_1{};
    request_1_1.method = "GET";
    request_1_2.method = "GET";
    request_1_2_1.method = "GET";
    request_1_1.target = "/target_1/target_1_1";
    request_1_2.target = "/target_1/target_1_2";
    request_1_2_1.target = "/target_1/target_1_2/target_1_2_1";

    bool routing_result_1_1 = router.route(request_1_1, response_1_1);
    bool routing_result_1_2 = router.route(request_1_2, response_1_2);
    bool routing_result_1_2_1 = router.route(request_1_2_1, response_1_2_1);

    EXPECT_TRUE(handler_accepted_1_1);
    EXPECT_TRUE(handler_accepted_1_2);
    EXPECT_TRUE(handler_accepted_1_2_1);
    EXPECT_TRUE(handler_1_1_invoked);
    EXPECT_TRUE(handler_1_2_invoked);
    EXPECT_TRUE(handler_1_2_1_invoked);
    EXPECT_TRUE(routing_result_1_1);
    EXPECT_TRUE(routing_result_1_2);
    EXPECT_TRUE(routing_result_1_2_1);
    EXPECT_EQ(response_1_1.body, "touched_by_handler_1_1");
    EXPECT_EQ(response_1_2.body, "touched_by_handler_1_2");
    EXPECT_EQ(response_1_2_1.body, "touched_by_handler_1_2_1");
}

TEST(HttpRouter, FallbackHandler)
{
    Router router{};
    int fallback_handler_invoked = 0;
    bool handler_invoked = false;

    route_handler fallback_handler = [&](const Request&, Response& response, params_t&) -> void
    {
        fallback_handler_invoked += 1;
        response.body = "touched_by_fallback_handler";
    };
    route_handler handler = [&](const Request&, Response& response, params_t&) -> void
    {
        handler_invoked = true;
        response.body = "touched_by_handler";
    };

    router.fallback(fallback_handler);
    bool handler_accepted = router.get("/target_1/target_1_1/target_1_1_1", handler);

    Request request_fallback_handler_1{};
    Request request_fallback_handler_2{};
    Request request_handler{};
    Response response_fallback_handler_1{};
    Response response_fallback_handler_2{};
    Response response_handler{};
    request_fallback_handler_1.method = "GET";
    request_fallback_handler_1.target = "/target_1/target_undefined/target_1_1_1";
    request_fallback_handler_2.method = "GET";
    request_fallback_handler_2.target = "/target_1/another_undefined_target";
    request_handler.method = "GET";
    request_handler.target = "/target_1/target_1_1/target_1_1_1";

    bool routing_result_fallback_1 = router.route(request_fallback_handler_1, response_fallback_handler_1);
    bool routing_result_fallback_2 = router.route(request_fallback_handler_2, response_fallback_handler_2);
    bool routing_result = router.route(request_handler, response_handler);

    EXPECT_TRUE(handler_accepted);
    EXPECT_EQ(fallback_handler_invoked, 2);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result_fallback_1);
    EXPECT_TRUE(routing_result_fallback_2);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(response_fallback_handler_1.body, "touched_by_fallback_handler");
    EXPECT_EQ(response_fallback_handler_2.body, "touched_by_fallback_handler");
    EXPECT_EQ(response_handler.body, "touched_by_handler");
}

TEST(HttpRouter, NoRoutingForMethod)
{
    Router router{};
    bool handler_invoked = false;

    route_handler handler = [&handler_invoked](const Request& request, Response& response, params_t&) -> void
    {
        handler_invoked = true;
        response.body = "touched_by_handler";
    };
    bool handler_accepted = router.get("/target_1/target_2", handler);

    Request request{};
    Response response{};
    request.method = "POST";
    request.target = "/target_1/target_2";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_FALSE(handler_invoked);
    EXPECT_FALSE(routing_result);
    EXPECT_EQ(response.body, "");
}

TEST(HttpRouter, NoFallbackRouting)
{
    Router router{};
    bool handler_invoked = false;

    route_handler handler = [&handler_invoked](const Request& request, Response& response, params_t&) -> void
    {
        handler_invoked = true;
        response.body = "touched_by_handler";
    };
    bool handler_accepted = router.get("/target_1/target_2", handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/target_1/target_2/fallback";
    response.body = "";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_FALSE(handler_invoked);
    EXPECT_FALSE(routing_result);
    EXPECT_EQ(response.body, "");
}

TEST(HttpRouter, ParameterExtraction_StartOfUrl)
{
    Router router{};
    std::string param_value;
    bool handler_invoked = false;
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> void
    {
        param_value = params["action"];
        response.body = params["action"];
        handler_invoked = true;
    };

    bool handler_accepted = router.get("<action>/users", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "delete/users";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(param_value, "delete");
    EXPECT_EQ(response.body, "delete");
}

TEST(HttpRouter, ParameterExtraction_MiddleOfUrl)
{
    Router router{};
    std::string param_value;
    bool handler_invoked = false;
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> void
    {
        param_value = params["id"];
        response.body = params["id"];
        handler_invoked = true;
    };
    bool handler_accepted = router.get("/target/users/<id>/update", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "/target/users/123/update";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(param_value, "123");
    EXPECT_EQ(response.body, "123");
}

TEST(HttpRouter, ParameterExtraction_EndOfUrl)
{
    Router router{};
    std::string param_value;
    bool handler_invoked = false;
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> void
    {
        param_value = params["username"];
        response.body = params["username"];
        handler_invoked = true;
    };
    bool handler_accepted = router.get("/target/users/<username>", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "/target/users/some_username";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(param_value, "some_username");
    EXPECT_EQ(response.body, "some_username");
}

TEST(HttpRouter, ParameterExtraction_MultipleParameters)
{
    Router router{};
    std::string param_value_1;
    std::string param_value_2;
    std::string param_value_3;
    bool handler_invoked = false;
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> void
    {
        param_value_1 = params["action"];
        param_value_2 = params["id"];
        param_value_3 = params["delete_type"];
        response.body = params["action"] + "_" + params["id"] + "_" + params["delete_type"];
        handler_invoked = true;
    };
    bool handler_accepted = router.get("<action>/users/<id>/method/<delete_type>", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "delete/users/123/method/hard_delete";
    bool routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(param_value_1, "delete");
    EXPECT_EQ(param_value_2, "123");
    EXPECT_EQ(param_value_3, "hard_delete");
    EXPECT_EQ(response.body, "delete_123_hard_delete");
}
