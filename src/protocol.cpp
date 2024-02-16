#include "protocol.h"

Protocol::Protocol() {}

Protocol::~Protocol() {}

ParseResult Protocol::parse(std::stringstream& incoming_message, std::stringstream& outgoing_message)
{
    std::array<char, 4> s{ 'd', 'o', 'n', 'e' };
    for (auto c : s) outgoing_message << c;
    return ParseResult::WRITE_AND_DIE;
}

// =======================================================================
// class HttpRouter {};
// class HttpParser {};
// class HttpObject {};
// class HttpRequest {};
// class HttpResponse {};
// =======================================================================