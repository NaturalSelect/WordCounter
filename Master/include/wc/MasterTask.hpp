#pragma once
#ifndef _WC_MASTERTASK_HPP
#define _WC_MASTERTASK_HPP

#include <vector>

#include <sharpen/SpinLock.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IteratorOps.hpp>
#include <sharpen/ITimer.hpp>
#include <sharpen/Optional.hpp>

#include "MapTask.hpp"
#include "ReduceTask.hpp"

namespace wc
{
    class MasterTask:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = wc::MasterTask;
    
        sharpen::SpinLock lock_;
        std::vector<wc::MapTask> mapTasks_;
        std::size_t expectFileCount_;
        std::vector<std::string> reduceFiles_;
        std::int32_t step_;
        std::vector<wc::ReduceTask> reduceTasks_;
        std::vector<std::string> resultFiles_;

        static sharpen::Optional<sharpen::ByteBuffer> GetChannelMessage(sharpen::NetStreamChannelPtr channel,sharpen::TimerPtr timer);
        
    public:
    
        template<typename _Iterator,typename _Check = decltype(std::declval<std::string&>() = *std::declval<_Iterator&>())>
        MasterTask(_Iterator begin,_Iterator end)
            :lock_()
            ,mapTasks_()
            ,expectFileCount_(0)
            ,reduceFiles_()
            ,step_(0)
            ,reduceTasks_()
            ,resultFiles_()
        {
            this->reduceFiles_.reserve(26);
            this->reduceTasks_.reserve(26);
            std::size_t size{sharpen::GetRangeSize(begin,end)};
            this->mapTasks_.reserve(size);
            this->expectFileCount_ = size;
            this->reduceFiles_.reserve(size);
            while (begin != end)
            {
                this->mapTasks_.emplace_back(*begin);
                ++begin;
            }
        }
    
        ~MasterTask() noexcept = default;

        sharpen::Int32 CostumeChannel(sharpen::NetStreamChannelPtr channel,sharpen::TimerPtr timer);
    };   
}

#endif