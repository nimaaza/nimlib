#include <gtest/gtest.h>

#include <unordered_map>
#include <optional>

#include "../../src/http/router.h"
#include "../../src/common/common.h"

using nimlib::Server::Handlers::Http::Router;
using nimlib::Server::Handlers::Http::Request;
using nimlib::Server::Handlers::Http::Response;
using nimlib::Server::Constants::HandlerState;
using route_handler = std::function<std::optional<HandlerState>(const Request&, Response&, std::unordered_map<std::string, std::string>&)>;
using params_t = std::unordered_map<std::string, std::string>;

TEST(HttpRouter, EmptyRouteRejected)
{
    Router router{};
    bool handler_invoked = false;

    route_handler handler = [&handler_invoked](const Request&, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            response.body = "touched_by_handler";
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("", handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "";
    auto routing_result = router.route(request, response);

    EXPECT_FALSE(handler_accepted);
    EXPECT_FALSE(routing_result);
    EXPECT_FALSE(handler_invoked);
    EXPECT_EQ(response.body, "");
}

TEST(HttpRouter, RootHandlerSettingAndInvokation)
{
    Router router{};
    bool root_handler_invoked = false;

    route_handler root_handler = [&root_handler_invoked](const Request&, Response& response, params_t&) -> std::optional<HandlerState>
        {
            root_handler_invoked = true;
            response.body = "touched_by_root_handler";
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("/", root_handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/";
    auto routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(root_handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(response.body, "touched_by_root_handler");
}

TEST(HttpRouter, SimpleHandlerSettingAndInvokation)
{
    Router router{};
    bool handler_invoked = false;

    route_handler handler = [&handler_invoked](const Request& request, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            response.body = "touched_by_this_handler";
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("/target_1/target_2", handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/target_1/target_2";
    auto routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(response.body, "touched_by_this_handler");
}

TEST(HttpRouter, ComplexHandlerSettingAndInvokation)
{
    Router router{};

    bool handler_1_1_invoked = false;
    bool handler_1_2_invoked = false;
    bool handler_1_2_1_invoked = false;

    route_handler handler_1_1 = [&handler_1_1_invoked](const Request& request, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_1_1_invoked = true;
            response.body = "touched_by_handler_1_1";
            return HandlerState::FINISHED_NO_WAIT;
        };

    route_handler handler_1_2 = [&handler_1_2_invoked](const Request& request, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_1_2_invoked = true;
            response.body = "touched_by_handler_1_2";
            return HandlerState::FINISHED_NO_WAIT;
        };

    route_handler handler_1_2_1 = [&handler_1_2_1_invoked](const Request& request, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_1_2_1_invoked = true;
            response.body = "touched_by_handler_1_2_1";
            return HandlerState::FINISHED_NO_WAIT;
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

    auto routing_result_1_1 = router.route(request_1_1, response_1_1);
    auto routing_result_1_2 = router.route(request_1_2, response_1_2);
    auto routing_result_1_2_1 = router.route(request_1_2_1, response_1_2_1);

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

    route_handler fallback_handler = [&](const Request&, Response& response, params_t&) -> std::optional<HandlerState>
        {
            fallback_handler_invoked += 1;
            response.body = "touched_by_fallback_handler";
            return HandlerState::FINISHED_NO_WAIT;
        };
    route_handler handler = [&](const Request&, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            response.body = "touched_by_handler";
            return HandlerState::FINISHED_NO_WAIT;
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

    auto routing_result_fallback_1 = router.route(request_fallback_handler_1, response_fallback_handler_1);
    auto routing_result_fallback_2 = router.route(request_fallback_handler_2, response_fallback_handler_2);
    auto routing_result = router.route(request_handler, response_handler);

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

    route_handler handler = [&handler_invoked](const Request& request, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            response.body = "touched_by_handler";
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("/target_1/target_2", handler);

    Request request{};
    Response response{};
    request.method = "POST";
    request.target = "/target_1/target_2";
    auto routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_FALSE(handler_invoked);
    EXPECT_FALSE(routing_result);
    EXPECT_EQ(response.body, "");
}

TEST(HttpRouter, NoFallbackRouting)
{
    Router router{};
    bool handler_invoked = false;

    route_handler handler = [&handler_invoked](const Request& request, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            response.body = "touched_by_handler";
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("/target_1/target_2", handler);

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/target_1/target_2/fallback";
    response.body = "";
    auto routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_FALSE(handler_invoked);
    EXPECT_FALSE(routing_result);
    EXPECT_EQ(response.body, "");
}

TEST(HttpRouter, RepeatedNameInTarget)
{
    Router router{};
    bool handler_1_invoked = false;
    bool handler_2_invoked = false;
    bool handler_3_invoked = false;

    route_handler handler_1 = [&handler_1_invoked](const Request&, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_1_invoked = true;
            response.body = "touched_by_handler_1";
            return HandlerState::FINISHED_NO_WAIT;
        };

    route_handler handler_2 = [&handler_2_invoked](const Request&, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_2_invoked = true;
            response.body = "touched_by_handler_2";
            return HandlerState::FINISHED_NO_WAIT;
        };

    route_handler handler_3 = [&handler_3_invoked](const Request&, Response& response, params_t&) -> std::optional<HandlerState>
        {
            handler_3_invoked = true;
            response.body = "touched_by_handler_3";
            return HandlerState::FINISHED_NO_WAIT;
        };

    bool handler_1_accepted = router.get("/target/path/to/target", handler_1);
    bool handler_2_accepted = router.get("/target/target", handler_2);
    bool handler_3_accepted = router.get("/path/to/repeated/target/target", handler_3);

    Request request_1{};
    Response response_1{};
    Request request_2{};
    Response response_2{};
    Request request_3{};
    Response response_3{};
    request_1.method = "GET";
    request_1.target = "/target/path/to/target";
    request_2.method = "GET";
    request_2.target = "/target/target";
    request_3.method = "GET";
    request_3.target = "/path/to/repeated/target/target";
    auto routing_result_1 = router.route(request_1, response_1);
    auto routing_result_2 = router.route(request_2, response_2);
    auto routing_result_3 = router.route(request_3, response_3);

    EXPECT_TRUE(handler_1_accepted);
    EXPECT_TRUE(handler_2_accepted);
    EXPECT_TRUE(handler_3_accepted);
    EXPECT_TRUE(routing_result_1);
    EXPECT_TRUE(routing_result_2);
    EXPECT_TRUE(routing_result_3);
    EXPECT_TRUE(handler_1_invoked);
    EXPECT_TRUE(handler_2_invoked);
    EXPECT_TRUE(handler_3_invoked);
    EXPECT_EQ(response_1.body, "touched_by_handler_1");
    EXPECT_EQ(response_2.body, "touched_by_handler_2");
    EXPECT_EQ(response_3.body, "touched_by_handler_3");
}

TEST(HttpRouter, RepeatedNameInTargetWithParams)
{
    Router router{};
    bool handler_1_invoked = false;
    bool handler_2_invoked = false;
    bool handler_3_invoked = false;
    std::string handler_1_param_id_1;
    std::string handler_1_param_id_2;
    std::string handler_2_param_id_1;
    std::string handler_3_param_id_1;
    std::string handler_3_param_id_2;

    route_handler handler_1 = [&](const Request&, Response& response, params_t& params) -> std::optional<HandlerState>
        {
            handler_1_invoked = true;
            handler_1_param_id_1 = params["id_1"];
            handler_1_param_id_2 = params["id_2"];
            response.body = "touched_by_handler_1";
            return HandlerState::FINISHED_NO_WAIT;
        };

    route_handler handler_2 = [&](const Request&, Response& response, params_t& params) -> std::optional<HandlerState>
        {
            handler_2_invoked = true;
            handler_2_param_id_1 = params["id_1"];
            response.body = "touched_by_handler_2";
            return HandlerState::FINISHED_NO_WAIT;
        };

    route_handler handler_3 = [&](const Request&, Response& response, params_t& params) -> std::optional<HandlerState>
        {
            handler_3_invoked = true;
            handler_3_param_id_1 = params["id_1"];
            handler_3_param_id_2 = params["id_2"];
            response.body = "touched_by_handler_3";
            return HandlerState::FINISHED_NO_WAIT;
        };

    bool handler_1_accepted = router.get("/target/<id_1>/path/to/target/<id_2>", handler_1);
    bool handler_2_accepted = router.get("/target/<id_1>/target", handler_2);
    bool handler_3_accepted = router.get("/path/to/<id_1>/repeated/target/<id_2>/target", handler_3);

    Request request_1{};
    Response response_1{};
    Request request_2{};
    Response response_2{};
    Request request_3{};
    Response response_3{};
    request_1.method = "GET";
    request_1.target = "/target/1/path/to/target/some_param_1";
    request_2.method = "GET";
    request_2.target = "/target/some_id/target";
    request_3.method = "GET";
    request_3.target = "/path/to/another_id/repeated/target/one_last_id/target";
    auto routing_result_1 = router.route(request_1, response_1);
    auto routing_result_2 = router.route(request_2, response_2);
    auto routing_result_3 = router.route(request_3, response_3);

    EXPECT_TRUE(handler_1_accepted);
    EXPECT_TRUE(handler_2_accepted);
    EXPECT_TRUE(handler_3_accepted);
    EXPECT_TRUE(routing_result_1);
    EXPECT_TRUE(routing_result_2);
    EXPECT_TRUE(routing_result_3);
    EXPECT_TRUE(handler_1_invoked);
    EXPECT_TRUE(handler_2_invoked);
    EXPECT_TRUE(handler_3_invoked);
    EXPECT_EQ(handler_1_param_id_1, "1");
    EXPECT_EQ(handler_1_param_id_2, "some_param_1");
    EXPECT_EQ(handler_2_param_id_1, "some_id");
    EXPECT_EQ(handler_3_param_id_1, "another_id");
    EXPECT_EQ(handler_3_param_id_2, "one_last_id");
    EXPECT_EQ(response_1.body, "touched_by_handler_1");
    EXPECT_EQ(response_2.body, "touched_by_handler_2");
    EXPECT_EQ(response_3.body, "touched_by_handler_3");
}

TEST(HttpRouter, ParameterExtraction_StartOfUrl)
{
    Router router{};
    std::string param_value;
    bool handler_invoked = false;
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> std::optional<HandlerState>
        {
            param_value = params["action"];
            response.body = params["action"];
            handler_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };

    bool handler_accepted = router.get("/<action>/users", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "/delete/users";
    auto routing_result = router.route(request, response);

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
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> std::optional<HandlerState>
        {
            param_value = params["id"];
            response.body = params["id"];
            handler_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("/target/users/<id>/update", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "/target/users/123/update";
    auto routing_result = router.route(request, response);

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
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> std::optional<HandlerState>
        {
            param_value = params["username"];
            response.body = params["username"];
            handler_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("/target/users/<username>", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "/target/users/some_username";
    auto routing_result = router.route(request, response);

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
    route_handler handler = [&](const Request&, Response& response, params_t& params) -> std::optional<HandlerState>
        {
            param_value_1 = params["action"];
            param_value_2 = params["id"];
            param_value_3 = params["delete_type"];
            response.body = params["action"] + "_" + params["id"] + "_" + params["delete_type"];
            handler_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };
    bool handler_accepted = router.get("/<action>/users/<id>/method/<delete_type>", handler);

    Request request;
    Response response;
    request.method = "GET";
    request.target = "/delete/users/123/method/hard_delete";
    auto routing_result = router.route(request, response);

    EXPECT_TRUE(handler_accepted);
    EXPECT_TRUE(handler_invoked);
    EXPECT_TRUE(routing_result);
    EXPECT_EQ(param_value_1, "delete");
    EXPECT_EQ(param_value_2, "123");
    EXPECT_EQ(param_value_3, "hard_delete");
    EXPECT_EQ(response.body, "delete_123_hard_delete");
}

TEST(HttpRouter, AddSubroutes)
{
    Router main_router{};
    Router sub_routes{};

    bool get_handler_1_1_1_invoked = false;
    bool get_handler_1_1_2_invoked = false;
    bool get_handler_1_2_1_invoked = false;
    bool post_handler_1_1_1_invoked = false;

    auto get_handler_1_1_1 = [&](const Request&, Response&, params_t&) -> std::optional<HandlerState>
        {
            get_handler_1_1_1_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };
    auto get_handler_1_1_2 = [&](const Request&, Response&, params_t&) -> std::optional<HandlerState>
        {
            get_handler_1_1_2_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };
    auto get_handler_1_2_1 = [&](const Request&, Response&, params_t&) -> std::optional<HandlerState>
        {
            get_handler_1_2_1_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };
    auto post_handler_1_1_1 = [&](const Request&, Response&, params_t&) -> std::optional<HandlerState>
        {
            post_handler_1_1_1_invoked = true;
            return HandlerState::FINISHED_NO_WAIT;
        };

    sub_routes.get("/target_1_1/target_1_1_1", get_handler_1_1_1);
    sub_routes.get("/target_1_1/target_1_1_2", get_handler_1_1_2);
    sub_routes.get("/target_1_2/target_1_2_1", get_handler_1_2_1);
    sub_routes.post("/target_1_1/target_1_1_1", post_handler_1_1_1);
    main_router.sub_route("/target", std::move(sub_routes));

    Request get_request_1_1_1;
    Response get_response_1_1_1;
    Request get_request_1_1_2;
    Response get_response_1_1_2;
    Request get_request_1_2_1;
    Response get_response_1_2_1;
    Request post_request_1_1_1;
    Response post_response_1_1_1;
    get_request_1_1_1.method = "GET";
    get_request_1_1_1.target = "/target/target_1_1/target_1_1_1";
    get_request_1_1_2.method = "GET";
    get_request_1_1_2.target = "/target/target_1_1/target_1_1_2";
    get_request_1_2_1.method = "GET";
    get_request_1_2_1.target = "/target/target_1_2/target_1_2_1";
    post_request_1_1_1.method = "POST";
    post_request_1_1_1.target = "/target/target_1_1/target_1_1_1";

    auto get_routing_result_1_1_1 = main_router.route(get_request_1_1_1, get_response_1_1_1);
    auto get_routing_result_1_1_2 = main_router.route(get_request_1_1_2, get_response_1_1_2);
    auto get_routing_result_1_2_1 = main_router.route(get_request_1_2_1, get_response_1_2_1);
    auto post_routing_result_1_1_1 = main_router.route(post_request_1_1_1, post_response_1_1_1);

    EXPECT_TRUE(get_routing_result_1_1_1);
    EXPECT_TRUE(get_routing_result_1_1_2);
    EXPECT_TRUE(get_routing_result_1_2_1);
    EXPECT_TRUE(post_routing_result_1_1_1);
    EXPECT_TRUE(get_handler_1_1_1_invoked);
    EXPECT_TRUE(get_handler_1_1_2_invoked);
    EXPECT_TRUE(get_handler_1_2_1_invoked);
    EXPECT_TRUE(post_handler_1_1_1_invoked);
}

TEST(HttpRouter, AddSubroutesWithParamsInSubRoute)
{
    Router main_router{};
    Router sub_routes{};

    bool handler_invoked = false;
    std::string captured_param = "";

    auto handler = [&](const Request&, Response&, params_t& params) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            captured_param = params["param"];
            return HandlerState::FINISHED_NO_WAIT;
        };

    sub_routes.get("/sub_route/target/<param>", handler);
    main_router.sub_route("/top_route", std::move(sub_routes));

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/top_route/sub_route/target/some_value";

    auto routing_result = main_router.route(request, response);

    EXPECT_TRUE(routing_result);
    EXPECT_TRUE(handler_invoked);
    EXPECT_EQ(captured_param, "some_value");
}

TEST(HttpRouter, AddSubroutesWithParamsInTopRoute)
{
    Router main_router{};
    Router sub_routes{};

    bool handler_invoked = false;
    std::string captured_param = "";

    auto handler = [&](const Request&, Response&, params_t& params) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            captured_param = params["param"];
            return HandlerState::FINISHED_NO_WAIT;
        };

    sub_routes.get("/sub_route/target", handler);
    main_router.sub_route("/top_route/<param>", std::move(sub_routes));

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/top_route/some_value/sub_route/target";

    auto routing_result = main_router.route(request, response);

    EXPECT_TRUE(routing_result);
    EXPECT_TRUE(handler_invoked);
    EXPECT_EQ(captured_param, "some_value");
}

TEST(HttpRouter, AddSubroutesWithParamsInTopAndSubRoute)
{
    Router main_router{};
    Router sub_routes{};

    bool handler_invoked = false;
    std::string captured_param_1 = "";
    std::string captured_param_2 = "";
    std::string captured_param_3 = "";

    auto handler = [&](const Request&, Response&, params_t& params) -> std::optional<HandlerState>
        {
            handler_invoked = true;
            captured_param_1 = params["top_route_param"];
            captured_param_2 = params["sub_route_param"];
            captured_param_3 = params["action"];
            return HandlerState::FINISHED_NO_WAIT;
        };

    sub_routes.get("/sub_route/target/<sub_route_param>/<action>/end", handler);
    main_router.sub_route("/top_route/<top_route_param>/path/to/", std::move(sub_routes));

    Request request{};
    Response response{};
    request.method = "GET";
    request.target = "/top_route/top_route_value/path/to/sub_route/target/sub_route_value/route_action/end";

    auto routing_result = main_router.route(request, response);

    EXPECT_TRUE(routing_result);
    EXPECT_TRUE(handler_invoked);
    EXPECT_EQ(captured_param_1, "top_route_value");
    EXPECT_EQ(captured_param_2, "sub_route_value");
    EXPECT_EQ(captured_param_3, "route_action");
}
