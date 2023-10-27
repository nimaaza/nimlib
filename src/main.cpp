#include "main.h"

#include <regex>
#include <string>
#include <utility>

const std::string Request::message_body_delimiter{HTTP_DELIMITER + HTTP_DELIMITER};

HttpMethod::HttpMethod(std::string& request_method) {
  method = parse_http_method(request_method);
  if (method) {
    method_string = {request_method};
  } else {
    method_string = {};
  }
}

bool HttpMethod::parsed() const { return method.has_value(); }

bool HttpMethod::parsed(std::errc& err) const { return method.has_value(); }

std::optional<HttpMethod::HTTP_METHOD> HttpMethod::get_method() const { return method; }

std::optional<std::string> HttpMethod::to_string() const { return method_string; }

std::optional<HttpMethod::HTTP_METHOD> HttpMethod::parse_http_method(std::string& request_method) const {
  const auto& m = method_string_to_enum.find(request_method);
  if (m == method_string_to_enum.end()) {
    return {};
  } else {
    return {m->second};
  }
}

URI::URI(std::string& request_target) { parse_request_target(request_target); }

bool URI::parsed() const { return request_target.has_value(); };

bool URI::parsed(std::errc& err) const { return request_target.has_value(); };

std::optional<std::string> URI::to_string() const { return request_target; };

std::optional<std::string> URI::get_scheme() const { return scheme; };

std::optional<std::string> URI::get_authority() const { return authority; };

std::optional<std::string> URI::get_path() const { return path; };

std::optional<std::string> URI::get_query() const { return query; };

std::optional<std::string> URI::get_fragment() const { return fragment; };

void URI::parse_request_target(std::string& request_target_str) {
  // According to https://www.rfc-editor.org/rfc/pdfrfc/rfc3986.txt.pdf pages 50 & 51
  //  scheme = $2
  //  authority = $4
  //  path = $5
  //  query = $7
  //  fragment = $9
  static const std::regex uri_regex{"^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"};
  std::smatch matches;

  if (request_target_str.empty() || !std::regex_match(request_target_str, matches, uri_regex)) {
    return;
  }

  request_target.emplace(request_target_str);
  scheme.emplace(matches[2].str());
  authority.emplace(matches[4].str());
  path.emplace(matches[5].str());
  query.emplace(matches[7].str());
  fragment.emplace(matches[9].str());
}

HttpVersion::HttpVersion(std::string& http_version_string) {
  parse_http_version(http_version_string);
}

bool HttpVersion::parsed() const { return http_version_string.has_value(); }

bool HttpVersion::parsed(std::errc& err) const { return http_version_string.has_value(); }

std::optional<std::string> HttpVersion::to_string() const { return http_version_string; }

std::optional<std::string> HttpVersion::get_name() const { return name; }

std::optional<int> HttpVersion::get_major() const { return major; }

std::optional<int> HttpVersion::get_minor() const { return minor; }

void HttpVersion::parse_http_version(std::string& http_version) {
  if (http_version.find("HTTP/") != 0) return;

  auto decimal_sign_position = http_version.find('.', 5);
  if (decimal_sign_position == std::string::npos || decimal_sign_position == http_version.length()) return;

  auto major_str = http_version.substr(5, http_version.length() - decimal_sign_position - 1);
  auto minor_str = http_version.substr(decimal_sign_position + 1);

  try {
    major.emplace(std::stoi(major_str));
    minor.emplace(std::stoi(minor_str));
    name.emplace("HTTP");
    http_version_string.emplace(http_version);
  } catch (const std::exception&) {}
}

RequestStartLine::RequestStartLine(std::string& request_start_line)
    : request_start_line{request_start_line} {
  parse_start_line();
}

bool RequestStartLine::parsed() const {
  return method.has_value() && request_target.has_value() && version.has_value();
}

bool RequestStartLine::parsed(std::errc& err) const {
  return method.has_value() && request_target.has_value() && version.has_value();
}

std::optional<std::string> RequestStartLine::to_string() const {
  std::errc err;
  if (this->parsed(err)) return {request_start_line};
  else return {};
}

std::optional<HttpMethod> RequestStartLine::get_method() const { return method; }

std::optional<URI> RequestStartLine::get_request_target() const { return request_target; }

std::optional<HttpVersion> RequestStartLine::get_version() const { return version; }

void RequestStartLine::parse_start_line() {
  std::errc err{};

  // The request start line has three components separated by space
  // characters. A request line that does not contain exactly two
  // space characters can be regarded as invalid.
  if (std::count(request_start_line.begin(), request_start_line.end(), ' ') != 2) return;

  std::string method_str = split_once(request_start_line, " ", 0);
  HttpMethod method_obj{method_str};
  if (method_obj.parsed(err)) method = {method_obj};

  std::string request_target_str = split_once(request_start_line, " ", 1);
  URI request_target_obj{request_target_str};
  if (request_target_obj.parsed(err)) request_target = {request_target_obj};

  std::string version_str = split_once(request_start_line, " ", 2);
  HttpVersion version_obj{version_str};
  if (version_obj.parsed(err)) version = {version_obj};
}

HttpHeader::HttpHeader(const std::string& header_line)
    : successfully_parsed{false},
      header_line{header_line},
      parsed_header_line{} {
  size_t p = split_once_after(header_line, field_name, ":", 0);

  if (valid_field_name() && p != std::string::npos) {
    field_value = header_line.substr(p);
    std::transform(
        field_name.begin(),
        field_name.end(),
        field_name.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );

    size_t leading_white_space_pos{};
    size_t trailing_white_space_pos{field_value.length() - 1};
    while (is_http_white_space(field_value[leading_white_space_pos])) leading_white_space_pos++;
    while (is_http_white_space(field_value[trailing_white_space_pos])) trailing_white_space_pos--;
    field_value = field_value.substr(leading_white_space_pos, trailing_white_space_pos - leading_white_space_pos + 1);

    parsed_header_line = field_name + ": " + field_value;
    successfully_parsed = true;
  }
}

bool HttpHeader::parsed() const { return successfully_parsed; }

bool HttpHeader::parsed(std::errc& err) const { return successfully_parsed; }

std::optional<std::string> HttpHeader::to_string() const { return parsed_header_line; }

std::optional<std::string> HttpHeader::get_field_name() const {
  if (parsed()) return {field_name};
  else return {};
}

std::optional<std::string> HttpHeader::get_field_value() const {
  if (parsed()) return {field_value};
  else return {};
}

bool HttpHeader::valid_field_name() const {
  if (is_http_white_space(field_name[field_name.length()])) return false;
  if (!is_http_token(field_name)) return false;
  return true;
}

Request::Request(std::string& message)
    : parsed_successfully{false},
      message{message},
      request_start_line_str{split_once(message, HTTP_DELIMITER, 0)} {
  parse_request();
}

bool Request::parse_start_line() {
  std::errc err;
  request_start_line = RequestStartLine(request_start_line_str);
  return request_start_line.parsed(err);
}

bool Request::parse_headers_and_body() {
  bool headers_parse_success = true;
  std::errc err{};
  std::string header_line{};
  size_t p = request_start_line_str.length() + HTTP_DELIMITER.length();

  while (headers_parse_success) {
    p = split_once_after(message, header_line, HTTP_DELIMITER, p);
    // split_once_after returns an empty string as the null string
    // between \r\n\r\n before message body. Even if the request
    // has no headers, this will still work with the initial value
    // of p pointing to the \r\n before the message body. When the
    // message has no header and/or no body, p will be npos and
    // header_line will be empty.
    if (header_line.empty()) {
      break;
    }
    // When this happens while processing the headers, it means that
    // the message is not correctly delimited and the message body and
    // headers are indistinguishable.
    if (p == std::string::npos) {
      headers_parse_success = false;
      break;
    }

    const HttpHeader& http_header{header_line};
    if (http_header.parsed(err)) {
      headers.push_back(http_header);
    } else {
      headers_parse_success = false;
    }
    header_line.clear();
  }

  // For a correctly formatted message with body, after processing
  // headers (whether any exists), p should point to right after \r\n\r\n
  // before the body. When body does not exit, p is set to npos in the
  // loop above hence the check p < message.length().
  if (headers_parse_success && p < message.length()) {
    body = message.substr(p);
  }

  return headers_parse_success;
}

const HttpStartLine& Request::get_start_line() const { return request_start_line; }

const std::vector<HttpHeader>& Request::get_headers() const { return headers; }

std::optional<std::string> Request::get_body() const {
  if (body.empty()) return {};
  else return {body};
}

bool Request::parsed() const { return parsed_successfully; }

bool Request::parsed(std::errc& err) const { return parsed_successfully; }

std::optional<std::string> Request::to_string() const { return {}; };

void Request::parse_request() {
  if (!parse_start_line()) return;
  if (!parse_headers_and_body()) return;
  parsed_successfully = true;
}

std::string split_once(const std::string& s, const std::string& delimiter, int count) {
  for (size_t position = 0, i = 0; position < s.length(); i++) {
    auto next_position = s.find(delimiter, position);

    if (next_position == std::string::npos && position == 0) break;

    if (i == count) return s.substr(position, next_position - position);

    if (next_position == std::string::npos) break;

    position = next_position;
    do {
      position += delimiter.length();
    } while (s.substr(position, delimiter.length()) == delimiter);
  }

  return {};
}

size_t split_once_after(const std::string& source, std::string& destination, const std::string& delimiter, size_t pos) {
  if (auto next = source.find(delimiter, pos); next != std::string::npos) {
    destination = source.substr(pos, next - pos);
    next += delimiter.length();
    return next <= source.length() ? next : std::string::npos;
  } else {
    if (pos < source.length()) {
      destination = source.substr(pos);
    }
    return std::string::npos;
  }
}

bool is_http_white_space(uint8_t c) { return c == ' ' || c == '\t'; }

bool is_http_token(const std::string& s) {
  static std::vector<uint8_t> token_chars{'!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};

  if (s.empty()) return false;

  for (uint8_t c : s) {
    if (c >= '0' && c <= '9') continue;
    if (c >= 'a' && c <= 'z') continue;
    if (c >= 'A' && c <= 'Z') continue;
    if (std::find(token_chars.begin(), token_chars.end(), c) != token_chars.end()) continue;
    return false;
  }

  return true;
}
