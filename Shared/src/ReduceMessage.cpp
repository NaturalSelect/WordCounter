#include <wc/ReduceMessage.hpp>

#include <cassert>
#include <cstring>

#include <sharpen/Varint.hpp>

wc::ReduceMessage::ReduceMessage(std::uint32_t tag)
    :header_()
    ,files_()
    ,tag_(tag)
{
    this->header_.type_ = static_cast<std::uint64_t>(wc::MessageType::Reduce);
    this->header_.size_ = sizeof(this->header_) + sizeof(this->tag_) + 1;
}

wc::ReduceMessage::ReduceMessage(wc::ReduceMessage &&other) noexcept
    :header_(other.header_)
    ,files_(std::move(other.files_))
    ,tag_(other.tag_)
{
    other.header_.size_ = sizeof(this->header_) + sizeof(this->tag_) + 1;
}

wc::ReduceMessage &wc::ReduceMessage::operator=(wc::ReduceMessage &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->header_ = other.header_;
        this->files_ = std::move(other.files_);
        this->tag_ = other.tag_;
        other.header_.size_ = sizeof(this->header_) + sizeof(this->tag_) + 1;
    }
    return *this;
}

void wc::ReduceMessage::AddFile(std::string file)
{
    sharpen::Varuint64 builder{this->files_.size()};
    std::size_t oldSize{this->header_.size_};
    oldSize -= builder.ComputeSize();
    oldSize += file.size();
    builder.Set(builder.Get() + 1);
    oldSize += builder.ComputeSize();
    builder.Set(file.size());
    oldSize += builder.ComputeSize();
    this->files_.emplace_back(std::move(file));
    this->header_.size_ = oldSize;
}

std::size_t wc::ReduceMessage::LoadFrom(const char *data,std::size_t size)
{
    if(size < sizeof(this->header_) + 1 + sizeof(std::uint32_t))
    {
        throw std::invalid_argument("invalid reduce buffer");
    }
    std::size_t offset{0};
    std::memcpy(&this->header_,data,sizeof(this->header_));
    offset += sizeof(this->header_);
    std::uint32_t tag{0};
    std::memcpy(&tag,data + offset,sizeof(tag));
    offset += sizeof(tag);
    this->tag_ = tag;
    sharpen::Varuint64 builder{data + offset,size - offset};
    std::size_t count{builder.Get()};
    offset += builder.ComputeSize();
    for (std::size_t i = 0; i != count; ++i)
    {
        if(offset >= size)
        {
            throw sharpen::DataCorruptionException("reduce message corruption");
        }
        builder.Set(data + offset,size - offset);
        std::size_t fileSize{builder.Get()};
        offset += builder.ComputeSize();
        std::string file;
        file.reserve(fileSize);
        for (std::size_t j = 0; j != fileSize; ++j)
        {
            file.push_back(data[offset + j]);
        }
        this->files_.emplace_back(std::move(file));
        offset += fileSize;
    }
    return offset;
}

std::size_t wc::ReduceMessage::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t wc::ReduceMessage::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::memcpy(data,&this->header_,sizeof(this->header_));
    offset += sizeof(this->header_);
    std::memcpy(data + offset,&this->tag_,sizeof(this->tag_));
    offset += sizeof(this->tag_);
    sharpen::Varuint64 builder{this->files_.size()};
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data + offset,builder.Data(),size);   
    offset += size;
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        builder.Set(begin->size());
        size = builder.ComputeSize();
        std::memcpy(data + offset,builder.Data(),size);
        offset += size;
        std::memcpy(data + offset,begin->data(),begin->size());
        offset += begin->size();
    }
    return offset;
}

std::size_t wc::ReduceMessage::ComputeSize() const noexcept
{
    std::size_t offset{0};
    offset += sizeof(this->header_);
    offset += sizeof(this->tag_);
    sharpen::Varuint64 builder{this->files_.size()};
    std::size_t size{builder.ComputeSize()};  
    offset += size;
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        builder.Set(begin->size());
        size = builder.ComputeSize();
        offset += size;
        offset += begin->size();
    }
    return offset;
}

std::size_t wc::ReduceMessage::StoreTo(char *data,sharpen::Size size) const
{
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t wc::ReduceMessage::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t size{buf.GetSize() - offset};
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}