#include <wc/MasterTask.hpp>

#include <wc/OkMessage.hpp>

sharpen::Optional<sharpen::ByteBuffer> wc::MasterTask::GetChannelMessage(sharpen::NetStreamChannelPtr channel,sharpen::TimerPtr timer)
{
    wc::MessageHeader header;
    char *p = reinterpret_cast<char*>(&header);
    for (auto begin = p,end = p + sizeof(header) ; begin != end;)
    {
        sharpen::AwaitableFuture<bool> timeout;
        sharpen::AwaitableFuture<sharpen::Size> future;
        timeout.SetCallback([&channel](sharpen::Future<bool> &future)
        {
            if(future.Get())
            {
                channel->Cancel();
            }
        });
        channel->ReadAsync(begin,end - begin,future);
        timer->WaitAsync(timeout,std::chrono::seconds(1));
        future.WaitAsync();
        if(future.IsError() && timeout.IsCompleted())
        {
            return sharpen::EmptyOpt;
        }
        timer->Cancel();
        timeout.WaitAsync();
        try
        {
            std::size_t rz = future.Get();
            if(!rz)
            {
                return sharpen::EmptyOpt;
            }
            begin += rz;
        }
        catch(const std::exception&)
        {
            return sharpen::EmptyOpt;
        }
    }
    sharpen::ByteBuffer buf{header.size_};
    std::memcpy(buf.Data(),&header,sizeof(header));
    p = buf.Data() + sizeof(header);
    for (auto begin = p,end = buf.Data() + buf.GetSize(); begin != end;)
    {
        sharpen::AwaitableFuture<bool> timeout;
        sharpen::AwaitableFuture<sharpen::Size> future;
        timeout.SetCallback([&channel](sharpen::Future<bool> &future)
        {
            if(future.Get())
            {
                channel->Cancel();
            }
        });
        channel->ReadAsync(begin,end - begin,future);
        timer->WaitAsync(timeout,std::chrono::seconds(1));
        future.WaitAsync();
        if(future.IsError() && timeout.IsCompleted())
        {
            return sharpen::EmptyOpt;
        }
        timer->Cancel();
        timeout.WaitAsync();
        try
        {
            std::size_t rz = future.Get();
            if(!rz)
            {
                return sharpen::EmptyOpt;
            }
            begin += rz;
        }
        catch(const std::exception&)
        {
            return sharpen::EmptyOpt;
        }
    }
    return buf;
}

sharpen::Int32 wc::MasterTask::CostumeChannel(sharpen::NetStreamChannelPtr channel,sharpen::TimerPtr timer)
{
    wc::MapTask mapTask;
    wc::ReduceTask reduceTask;
    char type{0};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if(!this->mapTasks_.empty())
        {
            mapTask = std::move(this->mapTasks_.back());
            this->mapTasks_.pop_back();
        }
        else if(!this->reduceTasks_.empty())
        {
            type = 1;
            reduceTask = std::move(this->reduceTasks_.back());
            this->reduceTasks_.pop_back();
        }
        else
        {
            return this->resultFiles_.size() == 26;
        }
    }
    {
        sharpen::ByteBuffer buf;
        if(!type)
        {
            wc::MapMessage msg{mapTask.MakeMessage()};
            msg.StoreTo(buf);
        }
        else
        {   
            wc::ReduceMessage msg{reduceTask.MakeMessage()};
            msg.StoreTo(buf);
        }
        channel->WriteAsync(buf);
    }
    while (1)
    {
        auto msgBuf{Self::GetChannelMessage(channel,timer)};
        //timeout
        if(!msgBuf.Exist())
        {
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            if(!type)
            {
                this->mapTasks_.emplace_back(std::move(mapTask));
            }
            else
            {
                this->reduceTasks_.emplace_back(std::move(reduceTask));
            }
            return -1;
        }
        sharpen::ByteBuffer &buf = msgBuf.Get();
        wc::MessageHeader *header = reinterpret_cast<wc::MessageHeader*>(buf.Data());
        if(header->type_ == static_cast<std::uint64_t>(wc::MessageType::Ok))
        {
            wc::OkMessage msg;
            msg.LoadFrom(buf);
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            if(!this->step_)
            {
                this->reduceFiles_.emplace_back(msg.GetResult());
                if(this->reduceFiles_.size() == this->expectFileCount_)
                {
                    this->step_ = 1;
                    //create reduce tasks
                    for (std::size_t i = 0; i != 26; ++i)
                    {
                        this->reduceTasks_.emplace_back(i,this->reduceFiles_.begin(),this->reduceFiles_.end());   
                    }
                }
                break;
            }
            this->resultFiles_.emplace_back(msg.GetResult());
            return this->resultFiles_.size() == 26;
        }   
    }
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        return this->resultFiles_.size() == 26;
    }
}