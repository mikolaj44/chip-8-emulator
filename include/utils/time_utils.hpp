#pragma once

#include <chrono>

using timestamp = std::chrono::time_point<std::chrono::system_clock>;

timestamp current_timestamp()
{
    return std::chrono::system_clock::now();
}

size_t timestamp_difference(timestamp ts_1, timestamp ts_2)
{
    return static_cast<size_t>(std::chrono::duration_cast<std::chrono::milliseconds>(ts_1 - ts_2).count());
}

size_t ms_from_now(timestamp ts)
{
    return static_cast<size_t>(std::chrono::duration_cast<std::chrono::milliseconds>(current_timestamp() - ts).count());
}
