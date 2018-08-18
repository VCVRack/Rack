#pragma once

#include <cstdint>

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
    protected:
      bool triggered;
      bool lastTriggered;
      uint32_t triggerCount;
    private:
      float threshold;
      float lastValue;
      uint32_t triggerIntervalCount;
      uint32_t lastTriggerInterval;
  };
}
