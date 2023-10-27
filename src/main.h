#ifndef NIMLIB_LIBRARY_H
#define NIMLIB_LIBRARY_H

#include <iostream>
#include <optional>

const std::string HTTP_DELIMITER = "\r\n";

class HttpHeader;
class HttpStartLine;

struct Message {
  virtual ~Message() = default;

 protected:
  virtual bool parse_start_line() = 0;
  virtual bool parse_headers_and_body() = 0;

 public:
  virtual const HttpStartLine& get_start_line() const = 0;
  virtual const std::vector<HttpHeader>& get_headers() const = 0;
  virtual std::optional<std::string> get_body() const = 0;
};

struct HttpCheck {
  virtual ~HttpCheck() = default;

  virtual bool parsed() const = 0;
  virtual bool parsed(std::errc& err) const = 0;
  virtual std::optional<std::string> to_string() const = 0;
};

struct HttpStartLine : HttpCheck {
  virtual ~HttpStartLine() = default;
};

struct ErrorCondition {};

struct HttpMethod : HttpCheck {
  enum HTTP_METHOD { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };

  HttpMethod(std::string& request_method);
  HttpMethod() = default;
  ~HttpMethod() = default;

  bool parsed() const override;
  bool parsed(std::errc& err) const override;
  std::optional<std::string> to_string() const override;

  std::optional<HTTP_METHOD> get_method() const;

 private:
  std::optional<HTTP_METHOD> parse_http_method(std::string& request_method) const;

 private:
  std::unordered_map<std::string, HTTP_METHOD> method_string_to_enum{
      {"GET", GET}, {"HEAD", HEAD},
      {"POST", POST}, {"PUT", PUT},
      {"DELETE", DELETE}, {"CONNECT", CONNECT},
      {"OPTIONS", OPTIONS}, {"TRACE", TRACE}
  };

  std::optional<HTTP_METHOD> method;
  std::optional<std::string> method_string;
};

struct URI : HttpCheck {
  URI(std::string& request_target);
  URI() = default;
  ~URI() = default;

  bool parsed() const override;
  bool parsed(std::errc& err) const override;
  std::optional<std::string> to_string() const override;

  std::optional<std::string> get_scheme() const;
  std::optional<std::string> get_authority() const;
  std::optional<std::string> get_path() const;
  std::optional<std::string> get_query() const;
  std::optional<std::string> get_fragment() const;

 private:
  void parse_request_target(std::string& request_target);

  std::optional<std::string> request_target{};
  std::optional<std::string> scheme{};
  std::optional<std::string> authority{};
  std::optional<std::string> path{};
  std::optional<std::string> query{};
  std::optional<std::string> fragment{};
};

struct HttpVersion : HttpCheck {
  HttpVersion(std::string& http_version_string);
  HttpVersion() = default;
  ~HttpVersion() = default;

  bool parsed() const override;
  bool parsed(std::errc& err) const override;
  std::optional<std::string> to_string() const override;

  std::optional<std::string> get_name() const;
  std::optional<int> get_major() const;
  std::optional<int> get_minor() const;

 private:
  void parse_http_version(std::string& http_version);

  std::optional<std::string> http_version_string{};
  std::optional<std::string> name{};
  std::optional<int> major{};
  std::optional<int> minor{};
};

struct RequestStartLine : HttpStartLine {
  RequestStartLine() = default;
  RequestStartLine(std::string&);
  ~RequestStartLine() = default;

  bool parsed() const override;
  bool parsed(std::errc&) const override;
  std::optional<std::string> to_string() const override;

  std::optional<HttpMethod> get_method() const;
  std::optional<URI> get_request_target() const;
  std::optional<HttpVersion> get_version() const;

 private:
  void parse_start_line();

 private:
  std::string request_start_line;
  std::optional<HttpMethod> method = {};
  std::optional<URI> request_target = {};
  std::optional<HttpVersion> version = {};

};

struct HttpHeader : HttpCheck {
  HttpHeader(const std::string& header_line);
  ~HttpHeader() = default;

  bool parsed() const override;
  bool parsed(std::errc& err) const override;
  std::optional<std::string> to_string() const override;

  std::optional<std::string> get_field_name() const;
  std::optional<std::string> get_field_value() const;

 private:
  bool valid_field_name() const;

 private:
  bool successfully_parsed;
  std::string header_line;
  std::string field_name;
  std::string field_value;
  std::optional<std::string> parsed_header_line;
};

class Request : Message, HttpCheck {
 public:
  Request(std::string& message);
  ~Request() = default;

 protected:
  bool parse_start_line() override;
  bool parse_headers_and_body() override;

 public:
  const HttpStartLine& get_start_line() const override;
  const std::vector<HttpHeader>& get_headers() const override;
  std::optional<std::string> get_body() const override;

  bool parsed() const override;
  bool parsed(std::errc& err) const override;
  std::optional<std::string> to_string() const override;

 private:
  void parse_request();

 private:
  std::string& message;
  std::string request_start_line_str;
  RequestStartLine request_start_line;
  std::vector<HttpHeader> headers;
  std::string body;
  bool parsed_successfully;

  static const std::string message_body_delimiter;
};

std::string split_once(const std::string& s, const std::string& delimiter, int count);
size_t split_once_after(const std::string& source, std::string& destination, const std::string& delimiter, size_t pos);
bool is_http_white_space(uint8_t c);
bool is_http_token(const std::string& s);
#endif //NIMLIB_LIBRARY_H
