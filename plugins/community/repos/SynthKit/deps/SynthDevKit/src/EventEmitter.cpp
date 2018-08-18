#include "EventEmitter.hpp"

namespace SynthDevKit {
  EventEmitter::EventEmitter ( ) {
    has_emitted = false;
  }

  void EventEmitter::clear (bool all) {
    for (int16_t i = 0; i < TOP_EVENT; i++) {
      if (i != (EVENT_MAX - EVENT_CLEAR) || all) {
        emitters[i].clear();
      }
    }

    // emit the clear event
    emit(EVENT_CLEAR, 0.0f);
  }

  void EventEmitter::on (int16_t event, void (*func)(int16_t, float)) {
    int16_t real_event = realEvent(event);

    // if this is an invalid event return
    if (real_event == -1) {
      return;
    }

    emitters[real_event].push_back(func);
  }

  void EventEmitter::emit (int16_t event, float value) {
    int16_t real_event = realEvent(event);

    // if this is an invalid event, or there is nothing to do, return
    if (real_event == -1) {
      return;
    }

    // event the first event if this is the first one fired
    if (!has_emitted) {
      has_emitted = true;
      emit(EVENT_FIRST, value);
    }

    // odd and even emitters - but only if normal event
    if (event >= 0) {
      if (real_event % 2) {
        emit(EVENT_ODD, 0.0f);
      } else {
        emit(EVENT_EVEN, 0.0f);
      }
    }

    // iterate through the events, and call each one
    for(auto const &iter: emitters[real_event]) {
      iter(event, value);
    }
  }

  void EventEmitter::removeListener (int16_t event, void (*func)(int16_t, float)) {
    int16_t real_event = realEvent(event);

    // if this is an invalid event, or there is nothing to do, return
    if (real_event == -1 || emitters[real_event].size() == 0) {
      return;
    }

    for (std::vector<void (*)(int16_t, float)>::iterator it = emitters[real_event].begin(); it != emitters[real_event].end(); ++it) {
      if (*it == func) {
        emitters[real_event].erase(it);
        return;
      }
    }
  }

  int16_t EventEmitter::listenerCount (int16_t event) {
    int16_t real_event = realEvent(event);

    // if this is an invalid event return 0
    if (real_event == -1) {
      return 0;
    }

    return emitters[real_event].size();
  }

  int16_t EventEmitter::realEvent (int16_t event) {
    int16_t real_event;

    // adjust the events to be positive event id's
    if (event < 0) {
      real_event = EVENT_MAX - event;
    } else {
      real_event = event;
    }

    if (real_event < 0 || real_event >= TOP_EVENT) {
      return -1;
    } else {
      return real_event;
    }
  }
}
