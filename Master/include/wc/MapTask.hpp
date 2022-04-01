#pragma once
#ifndef _WC_MAPTASK_HPP
#define _WC_MAPTASK_HPP

#include <string>

#include <wc/MapMessage.hpp>

namespace wc
{
    class MapTask
    {
    private:
        using Self = wc::MapTask;
    
        std::string fileName_;        
    public:

        MapTask() = default;
    
        explicit MapTask(std::string fileName)
            :fileName_(std::move(fileName))
        {}
    
        MapTask(const Self &other) = default;
    
        MapTask(Self &&other) noexcept = default;
    
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
                this->fileName_ = std::move(other.fileName_);
            }
            return *this;
        }
    
        ~MapTask() noexcept = default;

        const std::string &GetFileName() const noexcept
        {
            return this->fileName_;
        }

        inline wc::MapMessage MakeMessage() const
        {
            wc::MapMessage msg{this->fileName_};
            return msg;
        }
    };   
}

#endif