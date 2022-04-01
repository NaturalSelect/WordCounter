#pragma once
#ifndef _WC_MAPMESSAGE_HPP
#define _WC_MAPMESSAGE_HPP

#include <string>
#include <utility>

#include <sharpen/ByteBuffer.hpp>
#include <sharpen/DataCorruptionException.hpp>

#include "MessageHeader.hpp"

namespace wc
{
    class MapMessage
    {
    private:
        using Self = wc::MapMessage;
    
        wc::MessageHeader header_;
        std::string fileName_;
    public:
    
        MapMessage();

        explicit MapMessage(std::string fileName);
    
        MapMessage(const Self &other) = default;
    
        MapMessage(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~MapMessage() noexcept = default;

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t ComputeSize() const noexcept;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t size) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        const std::string &GetFileName() const noexcept
        {
            return this->fileName_;
        }
    };
}

#endif