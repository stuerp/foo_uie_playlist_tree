
/** $VER: Toggle.h (2026.07.19) P. Stuer - Adaped from foobar2000 SDK **/

#pragma once

#pragma once

template<class T> class toggle_t
{
public:
    template<typename arg_t> toggle_t(T & variable, arg_t && value) : _Variable(variable)
    {
        _OldValue = std::move(_Variable);
        _Variable = std::forward<arg_t>(value);
    }

    toggle_t(const toggle_t &) = delete;
    void operator=(const toggle_t &) = delete;

    ~toggle_t()
    {
        _Variable = std::move(_OldValue);
    }

private:
    T & _Variable;
    T _OldValue;
};
