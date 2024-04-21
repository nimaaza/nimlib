#include <gtest/gtest.h>

#include <sstream>
#include <vector>

#include "../../src/http/http_parser.h"

using nimlib::Server::Protocols::parse_http_request;

TEST(HttpRequestParserTests, IncorrectStartLine)
{
    // Recipients of an invalid request-line SHOULD respond with either a
    // 400 (Bad Request) error or a 301 (Moved Permanently) redirect with
    // the request-target properly encoded

    // No \r\n at the end of the request line
    std::stringstream input_stream_1{ "GET /some/target/ HTTP/1.1" };
    std::stringstream input_stream_1_{ "GET /some/target/ HTTP/1.1"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // No \n at the end of the request line
    std::stringstream input_stream_2{ "GET /some/target/ HTTP/1.1\r" };
    std::stringstream input_stream_2_{ "GET /some/target/ HTTP/1.1\r"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // No \r at the end of the request line
    std::stringstream input_stream_3{ "GET /some/target/ HTTP/1.1\n" };
    std::stringstream input_stream_3_{ "GET /some/target/ HTTP/1.1\n"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // Unexpected char in request line
    std::stringstream input_stream_4{ "GET x /some/target/ HTTP/1.1\r\n" };
    std::stringstream input_stream_4_{ "GET x /some/target/ HTTP/1.1\r\n"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // Incorrect method (undefined)
    std::stringstream input_stream_5{ "GEET /some/target/ HTTP/1.1\r\n" };
    std::stringstream input_stream_5_{ "GEET /some/target/ HTTP/1.1\r\n"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // Incorrect method (not in capital letters)
    std::stringstream input_stream_6{ "get /some/target/ HTTP/1.1\r\n" };
    std::stringstream input_stream_6_{ "get /some/target/ HTTP/1.1\r\n"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // Incorrectly formatted version (anything other HTTP/1.1)
    std::stringstream input_stream_7{ "GET /some/target/ HTTP1.1\r\n" };
    std::stringstream input_stream_7_{ "GET /some/target/ HTTP1.1\r\n"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // Missing method
    std::stringstream input_stream_8{ "/some/target/ HTTP/1.1\r\n" };
    std::stringstream input_stream_8_{ "/some/target/ HTTP/1.1\r\n"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // Missing target
    std::stringstream input_stream_9{ "GET HTTP/1.1\r\n" };
    std::stringstream input_stream_9_{ "GET  HTTP/1.1\r\n"
                                       "Host: www.hostname.com\r\n"
                                       "\r\n"
                                       "message body" };

    // Missing version
    std::stringstream input_stream_10{ "GET /some/target/\r\n" };
    std::stringstream input_stream_10_{ "GET /some/target/\r\n"
                                        "Host: www.hostname.com\r\n"
                                        "\r\n"
                                        "message body" };

    // No whitespace is allowed in the three components of start line
    std::stringstream input_stream_11{ "G ET /some/target/ HTTP/1.1\r\n" };
    std::stringstream input_stream_11_{ "GET /some /target/ HTTP/1.1\r\n"
                                        "Host: www.hostname.com\r\n"
                                        "\r\n"
                                        "message body" };
    std::stringstream input_stream_12{ "G ET /some/target/ HTTP/1 .1\r\n" };
    std::stringstream input_stream_12_{ "GET /some/target/ HTTP /1.1\r\n"
                                        "Host: www.hostname.com\r\n"
                                        "\r\n"
                                        "message body" };


    EXPECT_FALSE(parse_http_request(input_stream_1));
    EXPECT_FALSE(parse_http_request(input_stream_1_));
    EXPECT_FALSE(parse_http_request(input_stream_2));
    EXPECT_FALSE(parse_http_request(input_stream_2_));
    EXPECT_FALSE(parse_http_request(input_stream_3));
    EXPECT_FALSE(parse_http_request(input_stream_3_));
    EXPECT_FALSE(parse_http_request(input_stream_4));
    EXPECT_FALSE(parse_http_request(input_stream_4_));
    EXPECT_FALSE(parse_http_request(input_stream_5));
    EXPECT_FALSE(parse_http_request(input_stream_5_));
    EXPECT_FALSE(parse_http_request(input_stream_6));
    EXPECT_FALSE(parse_http_request(input_stream_6_));
    EXPECT_FALSE(parse_http_request(input_stream_7));
    EXPECT_FALSE(parse_http_request(input_stream_7_));
    EXPECT_FALSE(parse_http_request(input_stream_8));
    EXPECT_FALSE(parse_http_request(input_stream_8_));
    EXPECT_FALSE(parse_http_request(input_stream_9));
    EXPECT_FALSE(parse_http_request(input_stream_9_));
    EXPECT_FALSE(parse_http_request(input_stream_10));
    EXPECT_FALSE(parse_http_request(input_stream_10_));
    EXPECT_FALSE(parse_http_request(input_stream_11));
    EXPECT_FALSE(parse_http_request(input_stream_11_));
    EXPECT_FALSE(parse_http_request(input_stream_12));
    EXPECT_FALSE(parse_http_request(input_stream_12_));
}

TEST(HttpRequestParserTests, IncorrectHeaders)
{
    // Header line not ending properly
    {
        // Without \r\n sequence
        std::stringstream input_stream_1{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_1));

        std::stringstream input_stream_2{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive"
            "\r\n" };
        EXPECT_FALSE(parse_http_request(input_stream_2));

        std::stringstream input_stream_3{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive" };
        EXPECT_FALSE(parse_http_request(input_stream_3));

        // Without \n
        std::stringstream input_stream_4{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\r"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_4));

        std::stringstream input_stream_5{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\r"
            "\r\n" };
        EXPECT_FALSE(parse_http_request(input_stream_5));

        std::stringstream input_stream_6{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\r" };
        EXPECT_FALSE(parse_http_request(input_stream_6));

        // Without \r
        std::stringstream input_stream_7{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\n"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_7));

        std::stringstream input_stream_8{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\n"
            "\r\n" };
        EXPECT_FALSE(parse_http_request(input_stream_8));

        std::stringstream input_stream_9{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\n" };
        EXPECT_FALSE(parse_http_request(input_stream_9));
    }

    // No colon in some header line
    {
        std::stringstream input_stream_10{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection keep-alive\r\n"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_10));

        std::stringstream input_stream_11{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection keep-alive\r"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_11));

        std::stringstream input_stream_12{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection keep-alive\n"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_12));

        std::stringstream input_stream_13{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Connection keep-alive\r\n"
            "\r\n" };
        EXPECT_FALSE(parse_http_request(input_stream_13));
    }

    // There must be no whitespace between header name and the colon
    {
        std::stringstream input_stream_14{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host : www.hostname.com\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_14));

        std::stringstream input_stream_15{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host\t: www.hostname.com\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_15));
    }

    // A sender MUST NOT send whitespace between the start-line and the first header field.
    {
        std::stringstream input_stream_16{
            "GET /some/target/ HTTP/1.1\r\n"
            " "
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };
        EXPECT_FALSE(parse_http_request(input_stream_16));
    }

    // A sender MUST NOT send a Content-Length header field in any message
    // that contains a Transfer-Encoding header field.
    {
        std::stringstream input_stream_16{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Content-Length: 12\r\n"
            "Connection: keep-alive\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "message body" };
        std::stringstream input_stream_17{
            "GET /some/target/ HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Host: www.hostname.com\r\n"
            "Content-Length: 12\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };
        std::stringstream input_stream_18{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Content-Length: 12\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };
        std::stringstream input_stream_19{
            "GET /some/target/ HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Host: www.hostname.com\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };

        EXPECT_FALSE(parse_http_request(input_stream_16));
        EXPECT_FALSE(parse_http_request(input_stream_17));
        EXPECT_TRUE(parse_http_request(input_stream_18));
        EXPECT_TRUE(parse_http_request(input_stream_19));
    }

    // Content-Length must be an integer
    {
        std::stringstream input_stream_20{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Content-Length: 12a\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };
        std::stringstream input_stream_21{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Content-Length: 45a2\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };
        std::stringstream input_stream_22{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Content-Length: 12\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "message body" };

        EXPECT_FALSE(parse_http_request(input_stream_20));
        EXPECT_FALSE(parse_http_request(input_stream_21));
        EXPECT_TRUE(parse_http_request(input_stream_22));
    }

    // Content-Length must not be repeated in headers
    {
        std::stringstream input_stream_23{
            "GET /some/target/ HTTP/1.1\r\n"
            "Host: www.hostname.com\r\n"
            "Content-Length: 12\r\n"
            "Connection: keep-alive\r\n"
            "Content-Length: 14\r\n"
            "\r\n"
            "message body" };

        EXPECT_FALSE(parse_http_request(input_stream_23));
    }
}

TEST(HttpRequestParserTests, RepeatedHeaderField)
{
    std::stringstream input_stream{
        "GET /some/target/ HTTP/1.1\r\n"
        "Host: www.hostname.com\r\n"
        "Header1: value-1\r\n"
        "Connection: keep-alive\r\n"
        "Header1: value-2\r\n"
        "\r\n" };
    auto parse_result = parse_http_request(input_stream);
    std::vector<std::string> header_1{ "value-1", "value-2" };

    EXPECT_TRUE(parse_result);
    EXPECT_EQ(parse_result.value().headers.find("header1")->second, header_1);
    EXPECT_EQ(parse_result.value().headers.find("host")->second, std::vector<std::string> { "www.hostname.com" });
    EXPECT_EQ(parse_result.value().headers.find("connection")->second, std::vector<std::string> { "keep-alive" });
}

TEST(HttpRequestParserTests, WhiteSpaceRemovedFromHeaderValues)
{
    std::stringstream input_stream{
        "GET /some/target/ HTTP/1.1\r\n"
        "Host:  www.hostname.com\r\n"
        "Header1: value-1 \r\n"
        "Connection: keep-alive  \r\n"
        "Header1:  value-2  \r\n"
        "\r\n" };
    auto parse_result = parse_http_request(input_stream);
    std::vector<std::string> header_1{ "value-1", "value-2" };

    EXPECT_TRUE(parse_result);
    EXPECT_EQ(parse_result.value().headers.find("header1")->second, header_1);
    EXPECT_EQ(parse_result.value().headers.find("host")->second, std::vector<std::string> { "www.hostname.com" });
    EXPECT_EQ(parse_result.value().headers.find("connection")->second, std::vector<std::string> { "keep-alive" });
}

TEST(HttpRequestParserTests, HeaderFieldValueSplit)
{
    std::stringstream input_stream{
        "GET /some/target/ HTTP/1.1\r\n"
        "Host: www.hostname.com\r\n"
        "Header: value-1, value-2, \tvalue-3\r\n"
        "Connection: keep-alive\r\n"
        "Transfer-Encoding: gzip;q=0.5, chunked\r\n"
        "\r\n"
        "message_body" };
    auto parse_result = parse_http_request(input_stream);

    EXPECT_TRUE(parse_result);
    EXPECT_EQ(parse_result.value().headers.size(), 4);
    EXPECT_EQ(parse_result.value().headers.find("header")->second.size(), 3);
    EXPECT_EQ(parse_result.value().headers.find("header")->second[0], "value-1");
    EXPECT_EQ(parse_result.value().headers.find("header")->second[1], "value-2");
    EXPECT_EQ(parse_result.value().headers.find("header")->second[2], "value-3");
    EXPECT_EQ(parse_result.value().headers.find("transfer-encoding")->second.size(), 2);
    EXPECT_EQ(parse_result.value().headers.find("transfer-encoding")->second[0], "gzip;q=0.5");
    EXPECT_EQ(parse_result.value().headers.find("transfer-encoding")->second[1], "chunked");
    EXPECT_EQ(parse_result.value().headers.find("host")->second[0], "www.hostname.com");
    EXPECT_EQ(parse_result.value().headers.find("connection")->second[0], "keep-alive");
}
