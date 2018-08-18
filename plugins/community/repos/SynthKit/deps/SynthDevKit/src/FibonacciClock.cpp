#include <vector>
#include "FibonacciClock.hpp"

namespace SynthDevKit {
  FibonacciClock::FibonacciClock (uint16_t count, float threshold) : Clock(count, threshold){

    if (count > FIBONACCI_LIMIT) {
      throw CLK_ERROR_TOO_MANY;
    }

    // generate the Fibonacci numbers for up to Fibonacci_LIMIT
    generateFibonaccis();
  }

  bool *FibonacciClock::update (float input) {
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
        states[i] = (step == fibonaccis[i]) ? true : false;
      }
    } else if (current >= cv->triggerInterval() / 2) {
      for (uint16_t i = 0; i < triggerCount; i++) {
        states[i] = false;
      }
    }

    if (step >= fibonaccis[triggerCount - 1]) {
      step = 0;
    }

    return states;
  }

  void FibonacciClock::generateFibonaccis ( ) {
    fibonaccis.push_back(1);
    fibonaccis.push_back(2);
    fibonaccis.push_back(3);
    fibonaccis.push_back(5);
    fibonaccis.push_back(8);
    fibonaccis.push_back(13);
    fibonaccis.push_back(21);
    fibonaccis.push_back(34);
  }

}
