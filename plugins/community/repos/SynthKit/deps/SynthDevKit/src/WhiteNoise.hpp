#pragma once

#include <cstdint>
#include <random>

namespace SynthDevKit {
  class WhiteNoise {
  public:
    WhiteNoise (uint32_t seed)  : rd { }, mt { rd() }, dist{ -5.0, 5.0 } { };
    void reset ( ) { };
    float stepValue ( ) { return dist(mt); };
  private:
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_real_distribution<double>  dist;
  };
}
