#pragma once
#ifndef _WC_REDUCETASK_HPP
#define _WC_REDUCETASK_HPP

#include <wc/ReduceMessage.hpp>

namespace wc
{
    class ReduceTask
    {
    private:
        using Self = ReduceTask;
    
        std::vector<std::string> fileNames_;
        std::uint32_t tag_;
    public:
        ReduceTask() = default;

        template<typename _Iterator>
        ReduceTask(std::uint32_t tag,_Iterator begin,_Iterator end)
            :fileNames_()
            ,tag_(tag)
        {
            this->fileNames_.reserve(sharpen::GetRangeSize(begin,end));
            while (begin != end)
            {
                this->fileNames_.emplace_back(*begin);
                ++begin;
            }
        }
    
        ReduceTask(const Self &other) = default;
    
        ReduceTask(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->fileNames_ = std::move(other.fileNames_);
                this->tag_ = other.tag_;
            }
            return *this;
        }
    
        ~ReduceTask() noexcept = default;

        void AddFile(std::string fileName)
        {
            this->fileNames_.emplace_back(std::move(fileName));
        }

        inline wc::ReduceMessage MakeMessage() const
        {
            wc::ReduceMessage msg{this->tag_};
            for (auto begin = this->fileNames_.begin(),end = this->fileNames_.end(); begin != end; ++begin)
            {
                msg.AddFile(*begin);
            }
            return msg;
        }
    };
}

#endif