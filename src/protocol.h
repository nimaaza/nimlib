#pragma once

#include <sstream>

#include "common.h"

class Protocol
{
public:
    Protocol();
    ~Protocol();

    // Protocol(const Protocol&) = delete;
    // Protocol& operator=(const Protocol&) = delete;

    // Protocol(Protocol&&) noexcept = delete;
    // Protocol& operator=(Protocol&&) noexcept = delete;

    ParseResult parse(std::stringstream& incoming_message, std::stringstream& outgoing_message);
};

