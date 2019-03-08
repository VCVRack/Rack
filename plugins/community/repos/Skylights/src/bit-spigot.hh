
#pragma once

#include "cmwc.hh"

// uses the CPU's random number generator to obtain a bank of random
// bits, which we can then query as needed. since this is all done by
// the CPU, we don't seed it or anything like that.
class bit_spigot {
  uint32_t m_data;
  unsigned char m_taps;
  cmwc m_prng;
  
public:
  bit_spigot();

  void reset();
  bool next();
};
