#include "gtest/gtest.h"

#include "../src/main.h"

#include <iostream>
#include <array>
#include <limits>
#include <optional>

TEST(HttpMethodTests, ParseMethodName) {
  std::vector<std::string> allowed_method_strings{
      "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"
  };

  for (auto& method_string : allowed_method_strings) {
    HttpMethod method{method_string};
    EXPECT_TRUE(method.parsed());
    EXPECT_TRUE(method.get_method().has_value());
    EXPECT_TRUE(method.to_string().has_value());
    EXPECT_EQ(method.to_string().value(), method_string);
  }

  std::vector<std::string> unallowed_method_strings{
      "get", "head", "post", "put", "delete", "connect", "options", "trace",
      "Get", "Head", "Post", "Put", "Delete", "Connect", "Options", "Trace",
      "ge", "ead", "posst"
  };

  for (auto& method_string : unallowed_method_strings) {
    HttpMethod method{method_string};
    EXPECT_FALSE(method.parsed());
    EXPECT_EQ(method.get_method(), std::nullopt);
    EXPECT_EQ(method.to_string(), std::nullopt);
  }
}

TEST(RequestTargetTests, ParseRequestTarget) {
  std::string empty_uri;
  URI uri{empty_uri};
  EXPECT_FALSE(uri.parsed());
  EXPECT_FALSE(uri.to_string().has_value());
  EXPECT_FALSE(uri.get_scheme().has_value());
  EXPECT_FALSE(uri.get_authority().has_value());
  EXPECT_FALSE(uri.get_path().has_value());
  EXPECT_FALSE(uri.get_query().has_value());
  EXPECT_FALSE(uri.get_fragment().has_value());

  std::string star_uri_str = "*";
  URI star_uri{star_uri_str};
  EXPECT_TRUE(star_uri.parsed());
  EXPECT_EQ(star_uri.to_string().value(), "*");
  EXPECT_EQ(star_uri.get_scheme().value(), "");
  EXPECT_EQ(star_uri.get_authority().value(), "");
  EXPECT_EQ(star_uri.get_path().value(), "*");
  EXPECT_EQ(star_uri.get_query().value(), "");
  EXPECT_EQ(star_uri.get_fragment().value(), "");

  std::string correct_uri_str = "https://www.ics.uci.edu/pub/ietf/uri/?query=q#Related";
  URI correct_uri{correct_uri_str};
  EXPECT_TRUE(correct_uri.parsed());
  EXPECT_EQ(correct_uri.to_string().value(), "https://www.ics.uci.edu/pub/ietf/uri/?query=q#Related");
  EXPECT_EQ(correct_uri.get_scheme().value(), "https");
  EXPECT_EQ(correct_uri.get_authority().value(), "www.ics.uci.edu");
  EXPECT_EQ(correct_uri.get_path().value(), "/pub/ietf/uri/");
  EXPECT_EQ(correct_uri.get_query().value(), "query=q");
  EXPECT_EQ(correct_uri.get_fragment().value(), "Related");
}

TEST(HttpVersionTests, ParseHttpVersion) {
  std::string version_string = "HTTP/1.1";
  HttpVersion correct_http_version{version_string};

  EXPECT_EQ(correct_http_version.parsed(), true);
  EXPECT_EQ(correct_http_version.get_name().has_value(), true);
  EXPECT_EQ(correct_http_version.get_name().value(), "HTTP");
  EXPECT_EQ(correct_http_version.get_major().has_value(), true);
  EXPECT_EQ(correct_http_version.get_major().value(), 1);
  EXPECT_EQ(correct_http_version.get_minor().has_value(), true);
  EXPECT_EQ(correct_http_version.get_minor().value(), 1);
  EXPECT_EQ(correct_http_version.to_string().has_value(), true);
  EXPECT_EQ(correct_http_version.to_string().value(), "HTTP/1.1");

  std::vector<std::string> incorrect_http_versions{
      "http/1.1", "HTP/1.1", "Http/1.1", "http1.1", "HTTP1.1", "1.1", "HTTP/1,1", "HTTP/1", "HTTP/1."
  };
  for (auto& http_version_str : incorrect_http_versions) {
    HttpVersion incorrect_http_version{http_version_str};

    EXPECT_EQ(incorrect_http_version.parsed(), false);
    EXPECT_EQ(incorrect_http_version.get_name().has_value(), false);
    EXPECT_EQ(incorrect_http_version.get_major().has_value(), false);
    EXPECT_EQ(incorrect_http_version.get_minor().has_value(), false);
    EXPECT_EQ(incorrect_http_version.to_string().has_value(), false);
  }
}

TEST(RequestStartLineTests, WhenStartLineIsIncorrect) {
  // Invalid HTTP verb
  std::string start_line = "GEET /index.html HTTP/1.1";
  RequestStartLine start_line_1{start_line};
  EXPECT_FALSE(start_line_1.parsed());

  // Non-existing HTTP verb
  start_line = "/index.html HTTP/1.1";
  RequestStartLine start_line_2{start_line};
  EXPECT_FALSE(start_line_2.parsed());

  // Invalid request target
  //  start_line = "GET ??? HTTP/1.1";
  //  RequestStartLine start_line_3{start_line};
  //  EXPECT_FALSE(start_line_3.parsed());

  // Non-existing request target
  start_line = "GET HTTP/1.1";
  RequestStartLine start_line_4{start_line};
  EXPECT_FALSE(start_line_4.parsed());

  // Invalid HTTP version
  start_line = "GET /index.html HTP/1.1";
  RequestStartLine start_line_5{start_line};
  EXPECT_FALSE(start_line_5.parsed());

  // Non-existing HTTP version
  start_line = "GET /index.html";
  RequestStartLine start_line_6{start_line};
  EXPECT_FALSE(start_line_6.parsed());

  start_line = "GET  /index.html HTTP/1.1";
  RequestStartLine start_line_7{start_line};
  EXPECT_FALSE(start_line_7.parsed());

  // Valid request start line
  start_line = "GET /index.html HTTP/1.1";
  RequestStartLine start_line_8{start_line};
  EXPECT_TRUE(start_line_8.parsed());
  EXPECT_TRUE(start_line_8.get_method().has_value());
  EXPECT_TRUE(start_line_8.get_request_target().has_value());
  EXPECT_TRUE(start_line_8.get_version().has_value());
  EXPECT_EQ(start_line_8.get_method().value().to_string().value(), "GET");
  EXPECT_EQ(start_line_8.get_request_target().value().to_string().value(), "/index.html");
  EXPECT_EQ(start_line_8.get_version().value().to_string().value(), "HTTP/1.1");
}

TEST(RequestStartLineTests, WhenStartLineIsCorrect) {
  // Valid request start line
  std::string start_line_str = "GET /index.html HTTP/1.1";
  RequestStartLine start_line{start_line_str};

  EXPECT_TRUE(start_line.parsed());
  EXPECT_TRUE(start_line.get_method().has_value());
  EXPECT_TRUE(start_line.get_request_target().has_value());
  EXPECT_TRUE(start_line.get_version().has_value());

  EXPECT_EQ(start_line.get_method().value().to_string().value(), "GET");
  EXPECT_EQ(start_line.get_request_target().value().to_string().value(), "/index.html");
  EXPECT_EQ(start_line.get_version().value().to_string().value(), "HTTP/1.1");
}

TEST(HttpHeaderTests, ParseHeaderFields) {
  HttpHeader http_header_1{"Accept-Language: en-US,en;q=0.5"};
  EXPECT_TRUE(http_header_1.parsed());
  EXPECT_TRUE(http_header_1.to_string().has_value());
  EXPECT_TRUE(http_header_1.get_field_name().has_value());
  EXPECT_TRUE(http_header_1.get_field_value().has_value());
  EXPECT_EQ(http_header_1.get_field_name().value(), "accept-language");
  EXPECT_EQ(http_header_1.get_field_value().value(), "en-US,en;q=0.5");
  EXPECT_EQ(http_header_1.to_string().value(), "accept-language: en-US,en;q=0.5");

  HttpHeader http_header_2{"Accept-Encoding: gzip, deflate, br"};
  EXPECT_TRUE(http_header_2.parsed());
  EXPECT_TRUE(http_header_2.to_string().has_value());
  EXPECT_EQ(http_header_2.to_string().value(), "accept-encoding: gzip, deflate, br");

  HttpHeader http_header_3{"Accept-Encoding:gzip, deflate, br"};
  EXPECT_TRUE(http_header_3.parsed());
  EXPECT_TRUE(http_header_3.to_string().has_value());
  EXPECT_EQ(http_header_3.to_string().value(), "accept-encoding: gzip, deflate, br");

  HttpHeader http_header_4{"Connection:keep-alive"};
  EXPECT_TRUE(http_header_4.parsed());
  EXPECT_TRUE(http_header_4.to_string().has_value());
  EXPECT_EQ(http_header_4.to_string().value(), "connection: keep-alive");

  HttpHeader http_header_5{"Host: en.cppreference.com\t "};
  EXPECT_TRUE(http_header_5.parsed());
  EXPECT_TRUE(http_header_5.to_string().has_value());
  EXPECT_EQ(http_header_5.to_string().value(), "host: en.cppreference.com");

  HttpHeader http_header_6{"server: Video Stats Server"};
  EXPECT_TRUE(http_header_6.parsed());
  EXPECT_TRUE(http_header_6.to_string().has_value());
  EXPECT_EQ(http_header_6.to_string().value(), "server: Video Stats Server");

  HttpHeader http_header_7{"Accept/Encoding: gzip, deflate, br"};
  EXPECT_FALSE(http_header_7.parsed());
  EXPECT_FALSE(http_header_7.to_string().has_value());

  HttpHeader http_header_8{"Accept/Encoding : gzip, deflate, br"};
  EXPECT_FALSE(http_header_8.parsed());
  EXPECT_FALSE(http_header_8.to_string().has_value());

  HttpHeader http_header_9{"Host\t\t:en.cppreference.com"};
  EXPECT_FALSE(http_header_9.parsed());
  EXPECT_FALSE(http_header_9.to_string().has_value());

  HttpHeader http_header_10{" "};
  EXPECT_FALSE(http_header_10.parsed());
  EXPECT_FALSE(http_header_10.to_string().has_value());

  HttpHeader http_header_11{"Host en.cppreference.com"};
  EXPECT_FALSE(http_header_11.parsed());
  EXPECT_FALSE(http_header_11.to_string().has_value());

  HttpHeader http_header_12{"abc: def: ghi"};
  EXPECT_TRUE(http_header_12.parsed());
  EXPECT_TRUE(http_header_12.to_string().has_value());
  EXPECT_TRUE(http_header_12.get_field_name().has_value());
  EXPECT_TRUE(http_header_12.get_field_value().has_value());
  EXPECT_EQ(http_header_12.get_field_name().value(), "abc");
  EXPECT_EQ(http_header_12.get_field_value().value(), "def: ghi");
  EXPECT_EQ(http_header_12.to_string().value(), "abc: def: ghi");
}

TEST(HttpRequestMessage, ParseHttpRequest_WithoutHeaderWithoutBody) {
  std::string message = "GET / HTTP/1.1\r\n";
  Request request{message};

  EXPECT_TRUE(request.parsed());
  EXPECT_TRUE(request.get_headers().empty());
  EXPECT_FALSE(request.get_body().has_value());
}

TEST(HttpRequestMessage, ParseHttpRequest_WithHeaderWithoutBody) {
  std::string message = "GET / HTTP/1.1\r\n"
                        "Host: www.example.com\r\n"
                        "User-Agent: Mozilla/5.0\r\n"
                        "Accept: text/html,application/xhtml+xml\r\n"
                        "Accept-Language: en-US,en;q=0.5\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Connection: keep-alive\r\n";
  Request request{message};

  EXPECT_TRUE(request.parsed());
  EXPECT_EQ(request.get_headers().size(), 6);

  EXPECT_TRUE(request.get_headers()[0].to_string().has_value());
  EXPECT_TRUE(request.get_headers()[0].get_field_name().has_value());
  EXPECT_TRUE(request.get_headers()[0].get_field_value().has_value());
  EXPECT_EQ(request.get_headers()[0].to_string().value(), "host: www.example.com");
  EXPECT_EQ(request.get_headers()[0].get_field_name().value(), "host");
  EXPECT_EQ(request.get_headers()[0].get_field_value().value(), "www.example.com");

  EXPECT_TRUE(request.get_headers()[2].to_string().has_value());
  EXPECT_TRUE(request.get_headers()[2].get_field_name().has_value());
  EXPECT_TRUE(request.get_headers()[2].get_field_value().has_value());
  EXPECT_EQ(request.get_headers()[2].to_string().value(), "accept: text/html,application/xhtml+xml");
  EXPECT_EQ(request.get_headers()[2].get_field_name().value(), "accept");
  EXPECT_EQ(request.get_headers()[2].get_field_value().value(), "text/html,application/xhtml+xml");

  EXPECT_TRUE(request.get_headers()[5].to_string().has_value());
  EXPECT_TRUE(request.get_headers()[5].get_field_name().has_value());
  EXPECT_TRUE(request.get_headers()[5].get_field_value().has_value());
  EXPECT_EQ(request.get_headers()[5].to_string().value(), "connection: keep-alive");
  EXPECT_EQ(request.get_headers()[5].get_field_name().value(), "connection");
  EXPECT_EQ(request.get_headers()[5].get_field_value().value(), "keep-alive");

  EXPECT_FALSE(request.get_body().has_value());
}

TEST(HttpRequestMessage, ParseHttpRequest_WithoutHeaderWithBody) {
  std::string message = "POST /test/demo_form.php HTTP/1.1\r\n"
                        "\r\n"
                        "name1=value1&name2=value2";

  Request request{message};
  auto& request_line = reinterpret_cast<const RequestStartLine&>(request.get_start_line());

  EXPECT_TRUE(request_line.parsed());
  EXPECT_TRUE(request_line.get_method().has_value());
  EXPECT_TRUE(request_line.get_request_target().has_value());
  EXPECT_TRUE(request_line.get_version().has_value());
  EXPECT_TRUE(request_line.get_method().value().parsed());
  EXPECT_TRUE(request_line.get_request_target().value().parsed());
  EXPECT_TRUE(request_line.get_version().value().parsed());
  EXPECT_EQ(request_line.get_method().value().to_string().value(), "POST");
  EXPECT_EQ(request_line.get_request_target().value().to_string().value(), "/test/demo_form.php");
  EXPECT_EQ(request_line.get_version().value().to_string().value(), "HTTP/1.1");

  EXPECT_EQ(request.get_headers().size(), 0);

  EXPECT_TRUE(request.get_body().has_value());
  EXPECT_EQ(request.get_body().value(), "name1=value1&name2=value2");
}

TEST(HttpRequestMessage, ParseHttpRequest_WithHeaderWithBody) {
  std::string message = "POST /test/demo_form.php HTTP/1.1\r\n"
                        "Host: w3schools.com\r\n"
                        "\r\n"
                        "name1=value1&name2=value2";

  Request request{message};
  auto& request_line = reinterpret_cast<const RequestStartLine&>(request.get_start_line());

  EXPECT_TRUE(request.parsed());
  EXPECT_EQ(request.get_headers().size(), 1);

  EXPECT_TRUE(request_line.parsed());
  EXPECT_TRUE(request_line.get_method().has_value());
  EXPECT_TRUE(request_line.get_request_target().has_value());
  EXPECT_TRUE(request_line.get_version().has_value());
  EXPECT_TRUE(request_line.get_method().value().parsed());
  EXPECT_TRUE(request_line.get_request_target().value().parsed());
  EXPECT_TRUE(request_line.get_version().value().parsed());
  EXPECT_EQ(request_line.get_method().value().to_string().value(), "POST");
  EXPECT_EQ(request_line.get_request_target().value().to_string().value(), "/test/demo_form.php");
  EXPECT_EQ(request_line.get_version().value().to_string().value(), "HTTP/1.1");

  EXPECT_EQ(request.get_headers().size(), 1);
  EXPECT_TRUE(request.get_headers()[0].parsed());
  EXPECT_TRUE(request.get_headers()[0].to_string().has_value());
  EXPECT_TRUE(request.get_headers()[0].get_field_name().has_value());
  EXPECT_TRUE(request.get_headers()[0].get_field_value().has_value());
  EXPECT_EQ(request.get_headers()[0].to_string().value(), "host: w3schools.com");
  EXPECT_EQ(request.get_headers()[0].get_field_name().value(), "host");
  EXPECT_EQ(request.get_headers()[0].get_field_value().value(), "w3schools.com");

  EXPECT_TRUE(request.get_body().has_value());
  EXPECT_EQ(request.get_body().value(), "name1=value1&name2=value2");
}

TEST(HttpRequestMessage, ParseHttpRequest_IncorrectMessages) {
  // According to RFC7230, the request start line must contain \r\n
  // even if it's the only component of the message.
  std::string message = "GET / HTTP/1.1";
  Request request_1{message};

  EXPECT_FALSE(request_1.parsed());
  EXPECT_FALSE(request_1.get_start_line().parsed());
  EXPECT_FALSE(request_1.get_body().has_value());
  EXPECT_FALSE(request_1.to_string().has_value());
  EXPECT_TRUE(request_1.get_headers().empty());

  // Incorrect start line (request target not specified)
  message = "POST HTTP/1.1\r\n"
            "Host: w3schools.com\r\n"
            "\r\n"
            "name1=value1&name2=value2";
  Request request_2{message};

  EXPECT_FALSE(request_2.parsed());
  EXPECT_FALSE(request_2.get_start_line().parsed());
  EXPECT_FALSE(request_2.get_body().has_value());
  EXPECT_FALSE(request_2.to_string().has_value());
  EXPECT_TRUE(request_2.get_headers().empty());

  // Incorrect start line (wrong delimiter at the end)
  message = "POST /test/demo_form.php HTTP/1.1\n"
            "Host: w3schools.com\r\n"
            "\r\n"
            "name1=value1&name2=value2";
  Request request_3{message};

  EXPECT_FALSE(request_3.parsed());
  EXPECT_FALSE(request_3.get_start_line().parsed());
  EXPECT_FALSE(request_3.get_body().has_value());
  EXPECT_FALSE(request_3.to_string().has_value());
  EXPECT_TRUE(request_3.get_headers().empty());

  // Incorrect headers (space after field name)
  message = "POST /test/demo_form.php HTTP/1.1\r\n"
            "Host : w3schools.com\r\n"
            "\r\n"
            "name1=value1&name2=value2";
  Request request_4{message};

  EXPECT_FALSE(request_4.parsed());
  EXPECT_TRUE(request_4.get_start_line().parsed());  // Start line is valid and gets parsed
  EXPECT_FALSE(request_4.get_body().has_value());
  EXPECT_FALSE(request_4.to_string().has_value());
  EXPECT_TRUE(request_4.get_headers().empty());

  // Incorrect message body delimiter (missing one \r\n)
  message = "POST /test/demo_form.php HTTP/1.1\r\n"
            "Host: w3schools.com\r\n"
            "User-Agent: Mozilla/5.0\r\n"
            "name1=value1&name2=value2";
  Request request_5{message};

  EXPECT_FALSE(request_5.parsed());
  EXPECT_TRUE(request_5.get_start_line().parsed());  // Start line is valid and gets parsed
  EXPECT_FALSE(request_5.get_body().has_value());
  EXPECT_FALSE(request_5.to_string().has_value());
  EXPECT_EQ(request_5.get_headers().size(), 2); // The first two headers are still parsed

  // Incorrect header (missing one \r\n)
  message = "POST /test/demo_form.php HTTP/1.1\r\n"
            "Host: w3schools.com\r\n"
            "User-Agent: Mozilla/5.0"
            "name1=value1&name2=value2";
  Request request_6{message};

  EXPECT_FALSE(request_6.parsed());
  EXPECT_TRUE(request_6.get_start_line().parsed());  // Start line is valid and gets parsed
  EXPECT_FALSE(request_6.get_body().has_value());
  EXPECT_FALSE(request_6.to_string().has_value());
  EXPECT_EQ(request_6.get_headers().size(), 1); // The first header is still parsed

  // Incorrect message body delimiter (missing one \r\n)
  message = "POST /test/demo_form.php HTTP/1.1\r\n"
            "Host: w3schools.com\r\n"
            "User-Agent: Mozilla/5.0\r\n"
            " \r\n"
            "name1=value1&name2=value2";
  Request request_7{message};

  EXPECT_FALSE(request_7.parsed());
  EXPECT_TRUE(request_7.get_start_line().parsed());  // Start line is valid and gets parsed
  EXPECT_FALSE(request_7.get_body().has_value());
  EXPECT_FALSE(request_7.to_string().has_value());
  EXPECT_EQ(request_7.get_headers().size(), 2); // The first header is still parsed

  // Incorrect message body delimiter (missing one \r\n)
  message = "POST /test/demo_form.php HTTP/1.1\r\n"
            "Host: w3schools.com\r\n"
            " \r\n"
            "User-Agent: Mozilla/5.0\r\n"
            "name1=value1&name2=value2";
  Request request_8{message};

  EXPECT_FALSE(request_8.parsed());
  EXPECT_TRUE(request_8.get_start_line().parsed());  // Start line is valid and gets parsed
  EXPECT_FALSE(request_8.get_body().has_value());
  EXPECT_FALSE(request_8.to_string().has_value());
  EXPECT_EQ(request_8.get_headers().size(), 1); // The first header is still parsed

  // Incorrect message body delimiter (missing one \r\n)
  message = "POST /test/demo_form.php HTTP/1.1\r\n"
            "Host: w3schools.com\r\n"
            " "
            "User-Agent: Mozilla/5.0\r\n"
            "name1=value1&name2=value2";
  Request request_9{message};

  EXPECT_FALSE(request_9.parsed());
  EXPECT_TRUE(request_9.get_start_line().parsed());  // Start line is valid and gets parsed
  EXPECT_FALSE(request_9.get_body().has_value());
  EXPECT_FALSE(request_9.to_string().has_value());
  EXPECT_EQ(request_9.get_headers().size(), 1); // The first header is still parsed
}

TEST(SplitOnceTests, SplitsAsRequired) {
  // Tests for single-char delimiter
  std::string test_string = "random string to split with respect to space char";
  EXPECT_EQ(split_once(test_string, " ", 0), "random");
  EXPECT_EQ(split_once(test_string, " ", 1), "string");
  EXPECT_EQ(split_once(test_string, " ", 4), "with");
  EXPECT_EQ(split_once(test_string, " ", 8), "char");
  EXPECT_EQ(split_once(test_string, " ", 400), "");

  // Tests for single-char delimiter (when delimiter is repeated)
  test_string = "random string  to split    with respect to space char";
  EXPECT_EQ(split_once(test_string, " ", 0), "random");
  EXPECT_EQ(split_once(test_string, " ", 1), "string");
  EXPECT_EQ(split_once(test_string, " ", 2), "to");
  EXPECT_EQ(split_once(test_string, " ", 4), "with");
  EXPECT_EQ(split_once(test_string, " ", 8), "char");
  EXPECT_EQ(split_once(test_string, " ", 400), "");

  // Tests for the case when the string begins with the delimiter
  test_string = " a test string beginning with space character";
  EXPECT_EQ(split_once(test_string, " ", 0), "");
  EXPECT_EQ(split_once(test_string, " ", 1), "a");
  EXPECT_EQ(split_once(test_string, " ", 2), "test");
  EXPECT_EQ(split_once(test_string, " ", 7), "character");

  test_string = "  a test string  beginning with space    character";
  EXPECT_EQ(split_once(test_string, " ", 0), "");
  EXPECT_EQ(split_once(test_string, " ", 1), "a");
  EXPECT_EQ(split_once(test_string, " ", 2), "test");
  EXPECT_EQ(split_once(test_string, " ", 7), "character");

  // Tests for multi-char delimiter
  test_string = "random\r\nstring\r\nto\r\nsplit\r\nwith\r\nmulti-char\r\ndelimiter";
  EXPECT_EQ(split_once(test_string, "\r\n", 0), "random");
  EXPECT_EQ(split_once(test_string, "\r\n", 1), "string");
  EXPECT_EQ(split_once(test_string, "\r\n", 3), "split");
  EXPECT_EQ(split_once(test_string, "\r\n", 6), "delimiter");
  EXPECT_EQ(split_once(test_string, "\r\n", 400), "");

  test_string = "\r\nrandom\r\nstring\r\nto\r\nsplit\r\nwith\r\nmulti-char\r\ndelimiter";
  EXPECT_EQ(split_once(test_string, "\r\n", 0), "");
  EXPECT_EQ(split_once(test_string, "\r\n", 1), "random");
  EXPECT_EQ(split_once(test_string, "\r\n", 2), "string");
  EXPECT_EQ(split_once(test_string, "\r\n", 4), "split");
  EXPECT_EQ(split_once(test_string, "\r\n", 7), "delimiter");
  EXPECT_EQ(split_once(test_string, "\r\n", 400), "");

  // Tests for multi-char delimiter (when there's a sequence of delimiter in string)
  test_string = "random\r\n\r\nstring\r\n\r\nto\r\nsplit\r\n\r\n\r\n\r\nwith\r\nmulti-char\r\ndelimiter";
  EXPECT_EQ(split_once(test_string, "\r\n", 0), "random");
  EXPECT_EQ(split_once(test_string, "\r\n", 1), "string");
  EXPECT_EQ(split_once(test_string, "\r\n", 3), "split");
  EXPECT_EQ(split_once(test_string, "\r\n", 4), "with");
  EXPECT_EQ(split_once(test_string, "\r\n", 6), "delimiter");
  EXPECT_EQ(split_once(test_string, "\r\n", 400), "");

  // Tests for multi-char repeating delimiter
  test_string = "random\r\n\r\nstring\r\nto\r\n\r\nsplit\r\n\r\nwith\r\n\r\nrespect\r\r\nto\r\n\r\nspace\r\n\r\nchar";
  EXPECT_EQ(split_once(test_string, "\r\n\r\n", 0), "random");
  EXPECT_EQ(split_once(test_string, "\r\n\r\n", 1), "string\r\nto");
  EXPECT_EQ(split_once(test_string, "\r\n\r\n", 4), "respect\r\r\nto");
  EXPECT_EQ(split_once(test_string, "\r\n\r\n", 6), "char");
  EXPECT_EQ(split_once(test_string, "\r\n\r\n", 400), "");

  // Tests for non-existing delimiter
  test_string = "random string to split with respect to space char";
  EXPECT_EQ(split_once(test_string, "  ", 0), "");
  EXPECT_EQ(split_once(test_string, "\n", 1), "");
  EXPECT_EQ(split_once(test_string, "\r\n", 4), "");
  EXPECT_EQ(split_once(test_string, "  ", 400), "");
  EXPECT_EQ(split_once(test_string, "abc", 400), "");

  // Tests for empty string
  test_string = "";
  EXPECT_EQ(split_once(test_string, " ", 0), "");
  EXPECT_EQ(split_once(test_string, "\r", 1), "");
  EXPECT_EQ(split_once(test_string, "\r\n", 3), "");
  EXPECT_EQ(split_once(test_string, "abc", 0), "");
}

TEST(SplitOnceAfter, SplitsAsRequired_SingleCharDelimiter) {
  // Test for non-repeated delimiter
  size_t pos;
  std::string test_string = "a simple test string to split";

  std::string result_1;
  pos = split_once_after(test_string, result_1, " ", 0);
  EXPECT_EQ(result_1, "a");
  EXPECT_EQ(pos, 2);

  std::string result_2;
  pos = split_once_after(test_string, result_2, " ", pos);
  EXPECT_EQ(result_2, "simple");
  EXPECT_EQ(pos, 9);

  std::string result_3;
  pos = split_once_after(test_string, result_3, " ", 24);
  EXPECT_EQ(result_3, "split");
  EXPECT_EQ(pos, std::string::npos);

  // Tests for repeated delimiter
  // Expected behaviour when the delimiter is repeated is to return
  // empty string (the empty string represents the non-existing
  // string between the delimiters). Repeated delimiters are not to
  // be treated as one delimiter.
  pos = 0;
  test_string = "a  simple test  string"; // Contains repeated space chars

  std::string result_4;
  pos = split_once_after(test_string, result_4, " ", 0);
  EXPECT_EQ(result_4, "a");
  EXPECT_EQ(pos, 2);

  std::string result_5;
  pos = split_once_after(test_string, result_5, " ", pos);
  EXPECT_EQ(result_5, "");
  EXPECT_EQ(pos, 3);

  std::string result_6;
  pos = split_once_after(test_string, result_6, " ", pos);
  EXPECT_EQ(result_6, "simple");
  EXPECT_EQ(pos, 10);

  std::string result_7;
  pos = split_once_after(test_string, result_7, " ", pos);
  EXPECT_EQ(result_7, "test");
  EXPECT_EQ(pos, 15);

  std::string result_8;
  pos = split_once_after(test_string, result_8, " ", pos);
  EXPECT_EQ(result_8, "");
  EXPECT_EQ(pos, 16);

  std::string result_9;
  pos = split_once_after(test_string, result_9, " ", pos);
  EXPECT_EQ(result_9, "string");
  EXPECT_EQ(pos, std::string::npos);

  // Tests for when the delimiter is repeated more than twice
  pos = 0;
  test_string = "three   spaces";

  std::string result_10;
  pos = split_once_after(test_string, result_10, " ", pos);
  EXPECT_EQ(result_10, "three");
  EXPECT_EQ(pos, 6);

  std::string result_11 = "must be set to empty string";
  pos = split_once_after(test_string, result_11, " ", pos);
  EXPECT_EQ(result_11, "");
  EXPECT_EQ(pos, 7);

  std::string result_12 = "must be set to empty string";
  pos = split_once_after(test_string, result_12, " ", pos);
  EXPECT_EQ(result_12, "");
  EXPECT_EQ(pos, 8);

  std::string result_13;
  pos = split_once_after(test_string, result_13, " ", pos);
  EXPECT_EQ(result_13, "spaces");
  EXPECT_EQ(pos, std::string::npos);

  // Tests for when the string starts with the delimiter
  pos = 0;
  test_string = " a simple test string to split beginning with delimiter";

  std::string result_14;
  pos = split_once_after(test_string, result_14, " ", pos);
  EXPECT_EQ(result_14, "");
  EXPECT_EQ(pos, 1);

  std::string result_15;
  pos = split_once_after(test_string, result_15, " ", 10);
  EXPECT_EQ(result_15, "test");
  EXPECT_EQ(pos, 15);

  // Tests for when pos is larger than string length
  test_string = "a test string";
  pos = test_string.length() + 10;
  std::string result_16 = "must not change";
  pos = split_once_after(test_string, result_16, " ", pos);
  EXPECT_EQ(result_16, "must not change");
  EXPECT_EQ(pos, std::string::npos);

  // Tests for when there is no delimiter in the string (whole string should be returned)
  test_string = "no_delimiter";
  std::string result_17;
  pos = 0;
  pos = split_once_after(test_string, result_17, " ", pos);
  EXPECT_EQ(result_17, "no_delimiter");
  EXPECT_EQ(pos, std::string::npos);

  pos = 2;
  pos = split_once_after(test_string, result_17, " ", pos);
  EXPECT_EQ(result_17, "_delimiter");
  EXPECT_EQ(pos, std::string::npos);
}

TEST(SplitOnceAfter, SplitsAsRequired_MultiCharDelimiter) {
  // Tests for non-repeated delimiter
  size_t pos;
  std::string test_string = "a simple test\r\nstring\r\nto split";

  std::string result_1;
  pos = split_once_after(test_string, result_1, "\r\n", 0);
  EXPECT_EQ(result_1, "a simple test");
  EXPECT_EQ(pos, 15);

  std::string result_2;
  pos = split_once_after(test_string, result_2, "\r\n", pos); // pos == 15 from previous test
  EXPECT_EQ(result_2, "string");
  EXPECT_EQ(pos, 23);

  std::string result_3;
  pos = split_once_after(test_string, result_3, "\r\n", pos); // pos == 23 from previous test
  EXPECT_EQ(result_3, "to split");
  EXPECT_EQ(pos, std::string::npos);

  // Tests for repeated delimiter
  pos = 0;
  std::string test_string_complex = "a simple test\r\nstring\r\nwith multi-char delimiter\r\n\r\nto split";

  std::string result_4;
  pos = split_once_after(test_string_complex, result_4, "\r\n", 0);
  EXPECT_EQ(result_4, "a simple test");
  EXPECT_EQ(pos, 15);

  std::string result_5;
  pos = split_once_after(test_string_complex, result_5, "\r\n", pos);
  EXPECT_EQ(result_5, "string");
  EXPECT_EQ(pos, 23);

  std::string result_6;
  pos = split_once_after(test_string_complex, result_6, "\r\n", pos);
  EXPECT_EQ(result_6, "with multi-char delimiter");
  EXPECT_EQ(pos, 50);

  // The destination should be set to empty string to represent the empty string
  // between adjacent delimiters.
  std::string result_7 = "should be set to empty string";
  pos = split_once_after(test_string_complex, result_7, "\r\n", pos);
  EXPECT_EQ(result_7, "");
  EXPECT_EQ(pos, 52);

  std::string result_8;
  pos = split_once_after(test_string_complex, result_8, "\r\n", pos);
  EXPECT_EQ(result_8, "to split");
  EXPECT_EQ(pos, std::string::npos);
}

TEST(HttpWhiteSpaceChar, ReturnsTrueForSpecifiedWhiteSpaceChars) {
  std::vector<uint8_t> ws{' ', '\t'}; // RFC 7230 page 82

  for (uint8_t c = 0; c < std::numeric_limits<uint8_t>::max(); c++) {
    if (std::find(ws.begin(), ws.end(), c) != ws.end()) {
      EXPECT_EQ(is_http_white_space(c), true);
    } else {
      EXPECT_EQ(is_http_white_space(c), false);
    }
  }
}

TEST(HttpToken, ReturnsTrueForHttpToken) {
  EXPECT_EQ(is_http_token("hello"), true);
  EXPECT_EQ(is_http_token("hello+bye"), true);
  EXPECT_EQ(is_http_token("content-type"), true);
  EXPECT_EQ(is_http_token("123456abcd"), true);
  EXPECT_EQ(is_http_token(""), false);
  EXPECT_EQ(is_http_token("hello\t"), false);
  EXPECT_EQ(is_http_token("hello bye"), false);
  EXPECT_EQ(is_http_token("hello\nbye"), false);
  // TODO needs more testing for corner cases
}

