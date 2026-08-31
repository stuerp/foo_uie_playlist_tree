
/** $VER: SharedState.h (2026.08.31) P. Stuer **/

#pragma once

#include <memory>
#include <mutex>

#include "State.h"

/// <summary>
/// Implements a primitive state hand-over mechanism between the UI element and the preferences page.
/// </summary>
class shared_state_t
{
public:
    shared_state_t() : _State() { }

    shared_state_t(shared_state_t &&) = delete;
    shared_state_t & operator=(shared_state_t &&) = delete;

    static shared_state_t & Instance()
    {
        static shared_state_t Object;

        return Object;
    }

    void Put(state_t * state)
    {
        std::scoped_lock lock(_Mutex);

        _State = state;
    }

    state_t * Get()
    {
        std::scoped_lock lock(_Mutex);

        return _State;
    }

    void Clear()
    {
        std::scoped_lock lock(_Mutex);

        _State = nullptr;
    }

private:
    std::mutex _Mutex;

    state_t * _State;
};
