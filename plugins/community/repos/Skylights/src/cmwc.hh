#pragma once
#include <cstddef>
#include <cstdint>

// complimentary multiply with carry, pseudo random number generator
// adapted from the public source code from wikipedia
// https://en.wikipedia.org/wiki/Multiply-with-carry_pseudorandom_number_generator
class cmwc {
  static const size_t CMWC_CYCLE = 4096;      // as Marsaglia recommends
  static const size_t CMWC_C_MAX = 809430660; // as Marsaglia recommends
  
  uint32_t Q[CMWC_CYCLE];
  uint32_t c;			// must be limited with CMWC_C_MAX
  unsigned i;

public:
  void seed(unsigned int value);
  uint32_t next();
};
