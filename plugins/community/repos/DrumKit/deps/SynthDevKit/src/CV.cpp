#include "CV.hpp"

namespace SynthDevKit {
  CV::CV (float threshold) {
    this->threshold = threshold;
    this->reset();
  }

  bool CV::newTrigger ( ) {
    // check to see if this is a status change, if so reset the states and return true
    if (this->triggered == true && this->lastTriggered == false) {
      this->lastTriggered = true;

      return true;
    }

    this->lastTriggered = this->triggered;

    return false;
  }

  void CV::update (float current) {
    // set the last value to whatever the current value is
    this->lastValue = current;

    // increase the trigger interval count
    this->triggerIntervalCount++;

    // check the threshold, if it meets or is greater, then we make a change
    if (current >= this->threshold) {
      if (this->triggered == false) {
        this->triggered = true;

        // increment the total number of triggers fired
        this->triggerCount++;

        // set the last trigger interval to the interval
        this->lastTriggerInterval = this->triggerIntervalCount;

        // reset the count to 0
        this->triggerIntervalCount = 0;
      }
    } else {
      this->triggered = false;
    }
  }

  bool CV::isHigh ( ) {
    return this->triggered;
  }

  bool CV::isLow ( ) {
    return !this->triggered;
  }

  void CV::reset ( ) {
    this->triggered = false;
    this->lastTriggered = false;
    this->lastValue = 0;
    this->triggerCount = 0;
    this->triggerIntervalCount = 0;
    this->lastTriggerInterval = 0;
  }

  float CV::currentValue ( ) {
    return this->lastValue;
  }

  uint32_t CV::triggerInterval ( ) {
    return this->lastTriggerInterval;
  }

  uint32_t CV::triggerTotal ( ) {
    return this->triggerCount;
  }
}
