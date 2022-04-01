#pragma once
#ifndef _WC_REDUCEMESSAGE_HPP
#define _WC_REDUCEMESSAGE_HPP

#include <utility>
#include <vector>
#include <string>

#include <sharpen/ByteBuffer.hpp>
#include <sharpen/DataCorruptionException.hpp>

#include "MessageHeader.hpp"

namespace wc
{
    class ReduceMessage
    {
    private:
        using Self = wc::ReduceMessage;
    
        wc::MessageHeader header_;
        std::vector<std::string> files_;
        std::uint32_t tag_;
    public:
        using ConstIterator = typename std::vector<std::string>::const_iterator;
        using ConstReverseIterator = typename std::vector<std::string>::const_reverse_iterator;

        explicit ReduceMessage(std::uint32_t tag);
    
        ReduceMessage(const Self &other) = default;
    
        ReduceMessage(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~ReduceMessage() noexcept = default;

        void AddFile(std::string file);

        inline std::uint32_t GetTag() const noexcept
        {
            return this->tag_;
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->files_.begin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->files_.end();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->files_.rbegin();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->files_.rend();
        }

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
    };
}

#endif