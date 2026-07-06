
/** $VER: UIElementTracker.h (2026.07.06) P. Stuer - Implements a tracker for user interface elements. **/

#pragma once

#include "UIElement.h"

/// <summary>
/// Implements a tracker for user interface elements.
/// </summary>
class uielement_tracker_t
{
public:
    uielement_tracker_t() : _CurrentItem() { }

    uielement_tracker_t(const uielement_tracker_t &) = delete;
    uielement_tracker_t & operator=(const uielement_tracker_t &) = delete;
    uielement_tracker_t(uielement_tracker_t &&) = delete;
    uielement_tracker_t & operator=(uielement_tracker_t &&) = delete;

    virtual ~uielement_tracker_t() { };

    void Add(uielement_t * element) noexcept
    {
        std::unique_lock Lock(_Mutex);

        _Items.push_back(element);

        SetCurrentElement(element);
    }

    void Remove(uielement_t * element) noexcept
    {
        std::unique_lock Lock(_Mutex);

        auto Iter = std::find(_Items.begin(), _Items.end(), element); 
  
        if (Iter != _Items.end())
        {
            _Items.erase(Iter); 

            SetCurrentElement(nullptr);
        }
    }

    uielement_t * GetCurrentElement() const noexcept
    {
        return _CurrentItem;
    }

    void SetCurrentElement(uielement_t * element) noexcept
    {
        _CurrentItem = element;
    }

private:
    std::mutex _Mutex;
    std::vector<uielement_t *> _Items;

    uielement_t * _CurrentItem;
};

extern uielement_tracker_t _UIElementTracker;
