#pragma once

#include <cstdint>
#include <mutex>

class timer
{
public:
    timer(uint8_t value = 0)
        : value_(value) {};

    auto operator=(const timer& other)
    {
        std::lock_guard<std::mutex> guard(lock);

        value_ = other.value_;
    }

    bool operator==(const timer& other)
    {
        std::lock_guard<std::mutex> guard(lock);

        return other.value_ == value_;
    }

    operator uint8_t()
    {
        std::lock_guard<std::mutex> guard(lock);

        return value_;
    }
    void decrement()
    {
        std::lock_guard<std::mutex> guard(lock);

        uint8_t prev = value_--;

        if(prev < value_)
        {
            value_ = 0;
        }
    }

private:
    std::mutex lock;

    uint8_t value_;
};
