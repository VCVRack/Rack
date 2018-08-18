#pragma once

#include <cstdint>

#define DTMF_PI 3.14159265358979323846

namespace SynthDevKit {
  class DTMF {
    public:
      DTMF (uint32_t);
      void reset ( );
      float stepValue ( );
      void setTone (char);
    private:
      uint32_t step;
      uint32_t sampleRate;
      uint16_t lowFreq;
      uint16_t highFreq;
  };
}
