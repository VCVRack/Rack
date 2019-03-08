#pragma once

#include "util/range.h"

namespace DHE {
namespace Rotation {
constexpr auto gain_range = Range{0.f, 2.f};
constexpr auto av_range = Range{-1.f, 1.f};

inline auto cv_offset(float bipolar_voltage) -> float {
  static constexpr auto cv_to_offset = 0.1f;
  return bipolar_voltage * cv_to_offset;
}

inline auto av_multiplier(float av_amount) -> float {
  return av_range.scale(av_amount);
}

inline auto gain_multiplier(float gain_amount) -> float {
  return gain_range.scale(gain_amount);
}

inline auto modulated(float knob_rotation, float cv_bipolar_voltage,
                      float av_amount) -> float {
  return knob_rotation +
         av_multiplier(av_amount) * cv_offset(cv_bipolar_voltage);
}

inline auto modulated(float knob_rotation, float cv_bipolar_voltage) -> float {
  return knob_rotation + cv_offset(cv_bipolar_voltage);
}
} // namespace Rotation
} // namespace DHE
