
/** $VER: UIElementTracker.h (2026.07.04) P. Stuer - Tracks the instances of the panel. **/

#pragma once

#include "UIElement.h"

class uielement_tracker_t
{
public:
    uielement_tracker_t() : _CurrentUIElement()
    {
    }

    uielement_tracker_t(const uielement_tracker_t &) = delete;
    uielement_tracker_t & operator=(const uielement_tracker_t &) = delete;
    uielement_tracker_t(uielement_tracker_t &&) = delete;
    uielement_tracker_t & operator=(uielement_tracker_t &&) = delete;

    virtual ~uielement_tracker_t() { };

    void Add(uielement_t * element) noexcept
    {
        std::unique_lock Lock(_Mutex);

        _UIElements.push_back(element);

        SetCurrentElement(element);
    }

    void Remove(uielement_t * element) noexcept
    {
        std::unique_lock Lock(_Mutex);

        auto Iter = std::find(_UIElements.begin(), _UIElements.end(), element); 
  
        if (Iter != _UIElements.end())
        {
            _UIElements.erase(Iter); 

            SetCurrentElement(nullptr);
        }
    }

    uielement_t * GetCurrentElement() const noexcept
    {
        return _CurrentUIElement;
    }

    void SetCurrentElement(uielement_t * element) noexcept
    {
        _CurrentUIElement = element;
    }

private:
    std::mutex _Mutex;

    uielement_t * _CurrentUIElement;
    std::vector<uielement_t *> _UIElements;
};

extern uielement_tracker_t _UIElementTracker;
