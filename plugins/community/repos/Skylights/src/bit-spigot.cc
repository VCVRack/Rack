#include "bit-spigot.hh"
#include <ctime>
//#include <immintrin.h>

bit_spigot::bit_spigot() {
  // TODO grab from some OS-specific decent entropy source
  m_prng.seed(time(NULL));
  reset();
}

void bit_spigot::reset() {
#if 0
  // this uses a hardware random number generator, but is only
  // available if you are using an intel cpu
  
  unsigned int x;
  _rdrand32_step(&x);
  
  m_data = x;
  m_taps = sizeof(unsigned int) * 8;
#endif

  m_data = m_prng.next();
  m_taps = 31;
}

bool bit_spigot::next() {
  if (m_taps == 0) reset();
  bool result = m_data & 0x1;
  m_data >>= 1;
  m_taps--;
  return result;
}
