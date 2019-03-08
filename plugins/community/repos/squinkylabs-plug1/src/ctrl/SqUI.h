#pragma once

#ifdef __V1
#include "event.hpp"
#endif

namespace sq {
  
#ifdef __V1
    using EventAction = const event::Action;
    using EventChange = event::Change;
    using Event = rack::event::Event;
#else
    using Action = rack::EventAction;       // what is this?
    using EventAction = rack::EventAction;
    using EventChange = rack::EventChange;
    using Event = rack::Event;
#endif

#ifdef __V1
    inline void consumeEvent(const Event* evt, ParamWidget* widget)
    {
       evt->consume(widget);
    }
#else
    inline void consumeEvent(Event* evt, ParamWidget* dummy)
    {
        evt->consumed = true;
    }
#endif

}