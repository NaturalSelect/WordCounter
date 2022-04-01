#include <wc/PingMessage.hpp>

#include <cstring>
#include <stdexcept>
#include <cassert>

wc::PingMessage::PingMessage()
    :header_()
{
    this->header_.type_ = static_cast<std::uint64_t>(wc::MessageType::Ping);
    this->header_.size_ = sizeof(this->header_);
}

wc::PingMessage &wc::PingMessage::operator=(wc::PingMessage &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->header_ = other.header_;
    }
    return *this;
}

std::size_t wc::PingMessage::LoadFrom(const char *data,std::size_t size)
{
    if(size < sizeof(this->header_))
    {
        throw std::invalid_argument("invalid ping message buffer");
    }
    std::memcpy(&this->header_,data,sizeof(this->header_));
    return sizeof(this->header_);
}

std::size_t wc::PingMessage::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t wc::PingMessage::UnsafeStoreTo(char *data) const noexcept
{
    std::memcpy(data,&this->header_,sizeof(this->header_));
    return sizeof(this->header_);
}

std::size_t wc::PingMessage::StoreTo(char *data,std::size_t size) const
{
    if(size < sizeof(this->header_))
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t wc::PingMessage::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t size{buf.GetSize() - offset};
    std::size_t needSize{this->ComputeSize()};
    if (size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}