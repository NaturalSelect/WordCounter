#pragma once
#ifndef _WC_MESSAGEHEADER_HPP
#define _WC_MESSAGEHEADER_HPP

#include <cstdint>

namespace wc
{
    enum class MessageType : std::uint64_t
    {
        Ping,
        Ok,
        Map,
        Reduce
    };

    struct MessageHeader
    {
        std::uint64_t size_;
        std::uint64_t type_;
    };
}

#endif