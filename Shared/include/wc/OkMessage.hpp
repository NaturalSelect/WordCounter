#pragma once
#ifndef _WC_OKMESSAGE_HPP
#define _WC_OKMESSAGE_HPP

#include <utility>
#include <string>

#include <sharpen/ByteBuffer.hpp>
#include <sharpen/DataCorruptionException.hpp>

#include "MessageHeader.hpp"

namespace wc
{
    class OkMessage
    {
    private:
        using Self = wc::OkMessage;
    
        wc::MessageHeader header_;
        std::string result_;
    public:
    
        OkMessage();

        explicit OkMessage(std::string result);
    
        OkMessage(const Self &other) = default;
    
        OkMessage(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~OkMessage() noexcept = default;

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t ComputeSize() const noexcept;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        inline const std::string &GetResult() const noexcept
        {
            return this->result_;
        }
    };
}

#endif