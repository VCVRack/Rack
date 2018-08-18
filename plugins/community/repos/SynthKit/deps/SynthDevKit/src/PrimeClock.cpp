#include <vector>
#include "PrimeClock.hpp"

namespace SynthDevKit {
  PrimeClock::PrimeClock (uint16_t count, float threshold) : Clock(count, threshold){

    if (count > PRIME_LIMIT) {
      throw CLK_ERROR_TOO_MANY;
    }

    // generate the prime numbers for up to PRIME_LIMIT
    generatePrimes();
  }

  bool *PrimeClock::update (float input) {
    cv->update(input);

    // only become ready after the first trigger has occurred.  this allows for
    // an interval to be set up
    if (!ready) {
      if (cv->newTrigger()) {
        ready = true;
      }

      return states;
    }

    current++;

    if (cv->newTrigger()) {
      step++;
      current = 0;

      for (uint16_t i = 0; i < triggerCount; i++) {
        states[i] = (step == primes[i]) ? true : false;
      }
    } else if (current >= cv->triggerInterval() / 2) {
      for (uint16_t i = 0; i < triggerCount; i++) {
        states[i] = false;
      }
    }

    if (step >= primes[triggerCount - 1]) {
      step = 0;
    }

    return states;
  }

  void PrimeClock::generatePrimes ( ) {
    primes.push_back(2);
    primes.push_back(3);
    primes.push_back(5);
    primes.push_back(7);
    primes.push_back(11);
    primes.push_back(13);
    primes.push_back(17);
    primes.push_back(19);
    primes.push_back(23);
    primes.push_back(29);
    primes.push_back(31);
    primes.push_back(37);
    primes.push_back(41);
    primes.push_back(43);
    primes.push_back(47);
    primes.push_back(53);
  }

}
