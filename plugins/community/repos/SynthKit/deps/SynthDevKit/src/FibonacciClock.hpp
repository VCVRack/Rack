#pragma once

#include <cstdint>
#include <vector>

#include "Clock.hpp"

#define FIBONACCI_LIMIT 8

namespace SynthDevKit {
  class FibonacciClock : public Clock {
    public:
      FibonacciClock (uint16_t, float);
      bool *update (float);
      std::vector<uint16_t> fibonaccis;
    private:
      void generateFibonaccis ( );
  };
}
