#pragma once

#include <sstream>

#include "common.h"

struct ProtocolInterface
{
    virtual ~ProtocolInterface() = default;

    // ProtocolInterface(const ProtocolInterface&) = delete;
    // ProtocolInterface& operator=(const ProtocolInterface&) = delete;

    // ProtocolInterface(ProtocolInterface&&) noexcept = delete;
    // ProtocolInterface& operator=(ProtocolInterface&&) noexcept = delete;

    virtual ParseResult parse(std::stringstream& incoming_message, std::stringstream& outgoing_message) = 0;
};

class Protocol : public ProtocolInterface
{
public:
    explicit Protocol();
    ~Protocol() override;
    ParseResult parse(std::stringstream&, std::stringstream&) override;
};
