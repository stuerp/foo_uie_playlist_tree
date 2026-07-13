
/** $VER: Tracker.h (2026.07.13) P. Stuer - Tracks the registered instances. **/

#pragma once

#include <vector>

template <typename T>
class tracker_t
{
public:
    tracker_t() : _Current() { }

    tracker_t(const tracker_t &) = delete;
    tracker_t & operator=(const tracker_t &) = delete;
    tracker_t(tracker_t &&) = delete;
    tracker_t & operator=(tracker_t &&) = delete;

    virtual ~tracker_t() { };

    void Add(T * item) noexcept
    {
        std::unique_lock Lock(_Mutex);

        _Items.push_back(item);

        SetCurrent(item);
    }

    void Remove(T * item) noexcept
    {
        std::unique_lock Lock(_Mutex);

        auto Iter = std::find(_Items.begin(), _Items.end(), item); 
  
        if (Iter != _Items.end())
        {
            _Items.erase(Iter); 

            SetCurrent(nullptr);
        }
    }

    T * GetCurrent() const noexcept
    {
        return _Current;
    }

    void SetCurrent(T * item) noexcept
    {
        _Current = item;
    }

private:
    std::mutex _Mutex;

    T * _Current;
    std::vector<T *> _Items;
};
