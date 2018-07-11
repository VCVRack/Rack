#include "Clock.hpp"

namespace SynthDevKit {
  Clock::Clock (uint16_t count, float threshold) {
    if (count > CLOCK_LIMIT) {
      throw CLK_ERROR_TOO_MANY;
    }

    triggerThreshold = threshold;
    triggerCount = count;
    current = 0;
    ready = false;
    step = 0;

    cv = new CV(threshold);

    for (uint16_t i = 0; i < CLOCK_LIMIT; i++) {
      states[i] = false;
    }
  }

  bool *Clock::update (float input) {
    cv->update(input);

    // only become ready after the first trigger has occurred.  this allows for
    // an interval to be set up
    if (!ready) {
      if (cv->newTrigger()) {
        ready = true;
      }

      return states;
    }

    current++;

    if (cv->newTrigger()) {
      step++;
      current = 0;

      for (uint16_t i = 0; i < triggerCount; i++) {
        states[i] = ((step % (i + 1)) == 0) ? true : false;
      }
    } else if (current >= cv->triggerInterval() / 2) {
      for (uint16_t i = 0; i < triggerCount; i++) {
        states[i] = false;
      }
    }

    if (step >= triggerCount) {
      step = 0;
    }

    return states;
  }

  void Clock::reset ( ) {
    current = 0;
    ready = false;
    step = 0;

    for (uint16_t i = 0; i < CLOCK_LIMIT; i++) {
      states[i] = false;
    }

    cv->reset();
  }
}
