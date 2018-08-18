#pragma once

#include <cstdint>
#include <vector>

#include "Clock.hpp"

#define PRIME_LIMIT 16

namespace SynthDevKit {
  class PrimeClock : public Clock {
    public:
      PrimeClock (uint16_t, float);
      bool *update (float);
      std::vector<uint16_t> primes;
    private:
      void generatePrimes ( );
  };
}
