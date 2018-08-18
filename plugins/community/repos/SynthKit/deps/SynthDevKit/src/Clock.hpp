#pragma once

#include <cstdint>

#include "CV.hpp"

#define CLOCK_LIMIT 1024

#define CLK_ERROR_TOO_MANY 1

namespace SynthDevKit {
  class Clock {
    public:
      Clock (uint16_t, float);
      bool *update (float);
      void reset ( );
    protected:
      CV *cv;
      uint16_t triggerCount;
      bool ready;
      uint64_t current;
      uint16_t step;
      float triggerThreshold;
      bool states[CLOCK_LIMIT];
  };
}
