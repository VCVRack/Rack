#pragma once

#include <stdint.h>

namespace SynthDevKit {
  class CV {
    public:
      CV (float);
      bool newTrigger( );
      void update (float);
      void reset ( );
      float currentValue ( );
      uint32_t triggerInterval ( );
      uint32_t triggerTotal ( );
      bool isHigh ( );
      bool isLow ( );
    private:
      float threshold;
      bool triggered;
      bool lastTriggered;
      float lastValue;
      uint32_t triggerIntervalCount;
      uint32_t lastTriggerInterval;
      uint32_t triggerCount;
  };
}
