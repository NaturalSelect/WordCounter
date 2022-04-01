#include <wc/MapMessage.hpp>

#include <cassert>
#include <cstring>
#include <stdexcept>

#include <sharpen/Varint.hpp>

wc::MapMessage::MapMessage()
    :MapMessage("")
{}

wc::MapMessage::MapMessage(std::string result)
    :header_()
    ,fileName_(std::move(result))
{
    this->header_.type_ = static_cast<std::uint64_t>(wc::MessageType::Map);
    this->header_.size_ = sizeof(this->header_);
    sharpen::Varuint64 build{this->fileName_.size()};
    this->header_.size_ += build.ComputeSize();
    this->header_.size_ += this->fileName_.size();
}

wc::MapMessage::MapMessage(wc::MapMessage &&other) noexcept
    :header_(other.header_)
    ,fileName_(std::move(other.fileName_))
{
    other.header_.size_ = sizeof(this->header_) + 1;
}

wc::MapMessage &wc::MapMessage::operator=(wc::MapMessage &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->header_ = other.header_;
        this->fileName_ = std::move(other.fileName_);
        other.header_.size_ = sizeof(this->header_) + 1;
    }
    return *this;
}

std::size_t wc::MapMessage::LoadFrom(const char *data,std::size_t size)
{
    if(size < sizeof(this->header_) + 1)
    {
        throw std::invalid_argument("invalid Map message buffer");
    }
    std::size_t offset{0};
    std::memcpy(&this->header_,data,sizeof(this->header_));
    offset += sizeof(this->header_);
    sharpen::Varuint64 builder{data + offset,size - offset};
    std::size_t resultSize{builder.Get()};
    offset += builder.ComputeSize();
    if (offset + resultSize > size)
    {
        throw sharpen::DataCorruptionException("Map message corruption");
    }
    this->fileName_.reserve(resultSize);
    for (std::size_t i = 0; i != resultSize; ++i)
    {
        this->fileName_.push_back(data[i + offset]);   
    }
    offset += resultSize;
    return offset;
}

std::size_t wc::MapMessage::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t wc::MapMessage::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::memcpy(data,&this->header_,sizeof(this->header_));
    offset += sizeof(this->header_);
    sharpen::Varuint64 builder{this->fileName_.size()};
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    size = this->fileName_.size();
    std::memcpy(data + offset,this->fileName_.data(),size);
    offset += size;
    return offset;
}

std::size_t wc::MapMessage::ComputeSize() const noexcept
{
    std::size_t offset{0};
    offset += sizeof(this->header_);
    sharpen::Varuint64 builder{this->fileName_.size()};
    std::size_t size{builder.ComputeSize()};
    offset += size;
    size = this->fileName_.size();
    offset += size;
    return offset;
}

std::size_t wc::MapMessage::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too samll");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t wc::MapMessage::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t needSize{this->ComputeSize()};
    std::size_t size{buf.GetSize() - offset};
    if(size < needSize)
    {
        buf.Extend(needSize - size);   
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}