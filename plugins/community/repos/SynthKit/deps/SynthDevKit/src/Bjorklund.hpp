#pragma once

#include <cstdint>

#define MAX_SLOTS 32
#define BJORK_ERROR_TOO_MANY 1;

namespace SynthDevKit {
  class Bjorklund {
    public:
      Bjorklund (uint8_t);
      void update (uint8_t, uint8_t);
      uint8_t stepValue ( );
      void reset ( );
    private:
      uint8_t steps[MAX_SLOTS];
      uint8_t currentStep;
      uint8_t numSteps;
      uint8_t numSlots;
      uint8_t _level;
      uint8_t _remainders[MAX_SLOTS * 2];
      uint8_t _divisor;
      uint8_t _step;
      uint8_t _count[MAX_SLOTS * 2];
      void computeResults ( );
      void buildString (int8_t);
  };
}
