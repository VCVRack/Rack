#pragma once

#include <cstdint>
#include <random>

namespace SynthDevKit {
  class PinkNoise {
  public:
    PinkNoise (uint32_t seed)  : rd { }, mt { rd() }, dist{ -1.0, 1.0 } {
      f0 = f1 = f2 = f3 = f4 = f5 = f6 = 0.0f;
    };
    void reset ( ) { };
    float stepValue ( ) {
      float white = dist(mt);

      f0 = 0.99886f * f0 + 0.0555179f * white;
      f1 = 0.99332f * f1 + 0.0750759f * white;
      f2 = 0.96900f * f2 + 0.1538520f * white;
      f3 = 0.86650f * f3 + 0.3104856f * white;
      f4 = 0.55000f * f4 + 0.5329522f * white;
      f5 = -0.7616 * f5 - 0.0168980f * white;
      float out = f0 + f1 + f2 + f3 + f4 + f5 + f6 + white * 0.5362f;
      out *= 0.55f;
      f6 = white * 0.115926f;

      return out;
   };

  private:
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_real_distribution<double>  dist;
    float f0, f1, f2, f3, f4, f5, f6;
  };
}
