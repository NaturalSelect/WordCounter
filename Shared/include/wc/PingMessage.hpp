#pragma once
#ifndef _WC_PINGMESSAGE_HPP
#define _WC_PINGMESSAGE_HPP

#include <utility>

#include <sharpen/ByteBuffer.hpp>

#include "MessageHeader.hpp"

namespace wc
{
    class PingMessage
    {
    private:
        using Self = wc::PingMessage;
    
        wc::MessageHeader header_;
    public:
    
        PingMessage();
    
        PingMessage(const Self &other) = default;
    
        PingMessage(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~PingMessage() noexcept = default;

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        inline constexpr std::size_t ComputeSize() const noexcept
        {
            return sizeof(this->header_);
        }

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t size) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }
    };
}

#endif