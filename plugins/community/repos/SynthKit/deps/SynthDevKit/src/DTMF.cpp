#include <cmath>
#include "DTMF.hpp"

namespace SynthDevKit {
  DTMF::DTMF (uint32_t sampleRate) {
    this->sampleRate = sampleRate;
    reset();
  }

  void DTMF::reset ( ) {
    step = 0;
    lowFreq = 0;
    highFreq = 0;
  }

  void DTMF::setTone (char key) {
    switch(key) {
    case '1': case '2': case '3': case 'A': lowFreq =  697; break;
    case '4': case '5': case '6': case 'B': lowFreq =  770; break;
    case '7': case '8': case '9': case 'C': lowFreq =  852; break;
    case '*': case '0': case '#': case 'D': lowFreq =  941; break;
    default: lowFreq = 0;
    }

    switch (key) {
    case '1': case '4': case '7': case '*': highFreq =  1209; break;
    case '2': case '5': case '8': case '0': highFreq =  1336; break;
    case '3': case '6': case '9': case '#': highFreq =  1477; break;
    case 'A': case 'B': case 'C': case 'D': highFreq =  1633; break;
    default: lowFreq = 0;
    }
  }

  float DTMF::stepValue ( ) {
    if (lowFreq == 0) {
      return 0;
    }

    double pi_prod_1 = (2.0 * DTMF_PI * lowFreq) / sampleRate;
    double pi_prod_2 = (2.0 * DTMF_PI * highFreq) / sampleRate;

    float ret = ((128 + (63 * sin(step * pi_prod_1)) + (63 * sin(step * pi_prod_2))) / 25.5) - 5;

    step++;

    return ret;
  }
}
