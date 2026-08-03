#pragma once

#include <functional>
#include <mutex>
#include <tuple>

template<typename OutputArg, typename... InputArgs>
class function_call_proxy
{
public:
    function_call_proxy(std::function<OutputArg(InputArgs...)> func)
    {
        _func = func;
    }

    void set(InputArgs... args)
    {
        std::lock_guard<std::mutex> guard(lock);

        _args = std::make_tuple(args...);

        _set_called = true;
        _call_called = false;
        _get_called = false;
    }

    void call()
    {
        std::lock_guard<std::mutex> guard(lock);

        _result = std::apply(_func, _args);

        _set_called = false;
        _call_called = true;
        _get_called = false;
    }

    OutputArg get()
    {
        std::lock_guard<std::mutex> guard(lock);

        _set_called = false;
        _call_called = false;
        _get_called = true;

        return _result;
    }

    bool was_set()
    {
        std::lock_guard<std::mutex> guard(lock);

        return _set_called;
    }

    bool was_called()
    {
        std::lock_guard<std::mutex> guard(lock);

        return _call_called;
    }

    bool was_get()
    {
        std::lock_guard<std::mutex> guard(lock);

        return _get_called;
    }

    void wait_for_call()
    {
        while(!was_called()) {};
    }

private:
    std::mutex lock;

    std::tuple<InputArgs...> _args;
    OutputArg _result;

    std::function<OutputArg(InputArgs...)> _func;

    bool _set_called = false;
    bool _call_called = false;
    bool _get_called = false;
};
