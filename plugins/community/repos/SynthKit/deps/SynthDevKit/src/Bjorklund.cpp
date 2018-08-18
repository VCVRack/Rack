#include "Bjorklund.hpp"

namespace SynthDevKit {
  Bjorklund::Bjorklund (uint8_t maxSlots) {
    if (maxSlots > MAX_SLOTS) {
      throw BJORK_ERROR_TOO_MANY;
    }

    numSlots = maxSlots;
    numSteps = maxSlots;

    // set step[0] to off as an initial value
    steps[0] = 0;
    reset();
  }

  void Bjorklund::reset ( ) {
    _step = 0;
    currentStep = 0;
  }

  void Bjorklund::buildString (int8_t level)  {
    if (level == -1) {
      steps[_step] = 0;
      _step++;
    } else if (level == -2)  {
      steps[_step] = 1;
      _step++;
    } else {
      for (uint8_t i = 0; i < _count[level]; i++) {
        buildString(level-1);
      }

      if (_remainders[level] !=0) {
        buildString(level-2);
      }
    }
  }

  void Bjorklund::computeResults( )  {
    if (numSteps > numSlots) {
      numSteps = numSlots;
    }

    _divisor = numSlots - numSteps;
    _remainders[0] = numSteps;
    _level = 0;

    do {
      _count[_level] = _divisor / _remainders[_level];
      _remainders[_level + 1] = _divisor % _remainders[_level];
      _divisor = _remainders[_level];
      _level++;
    } while (_remainders[_level] > 1);

    _count[_level] = _divisor;
    buildString (_level);

  }

  void Bjorklund::update (uint8_t steps, uint8_t slots) {
    if (slots > MAX_SLOTS) {
      throw BJORK_ERROR_TOO_MANY;
    }

    if (slots != numSlots || steps != numSteps) {
      numSlots = slots;
      numSteps = steps;
      reset();
      computeResults();
    }
  }

  uint8_t Bjorklund::stepValue ( ) {
    uint8_t ret = steps[currentStep];
    currentStep++;

    if (currentStep == numSlots) {
      currentStep = 0;
    }

    return ret;
  }
}
