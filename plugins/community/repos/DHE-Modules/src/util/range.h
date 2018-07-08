#pragma once

namespace rack_plugin_DHE_Modules {

inline float scale(float proportion, float lower_bound, float upper_bound) {
  return proportion*(upper_bound - lower_bound) + lower_bound;
}

struct Range {
  const float lower_bound;

  const float upper_bound;

  constexpr Range(float lower_bound, float upper_bound) noexcept : lower_bound(lower_bound), upper_bound(upper_bound) {}

  float scale(float proportion) const {
    return rack_plugin_DHE_Modules::scale(proportion, lower_bound, upper_bound);
  }

  float scale(bool state) const {
    return state ? upper_bound : lower_bound;
  }

  float normalize(float member) const {
    return (member - lower_bound)/(upper_bound - lower_bound);
  }

  float clamp(float f) const {
    if (f < lower_bound)
      return lower_bound;
    if (f > upper_bound)
      return upper_bound;
    return f;
  }
};

constexpr auto UNIPOLAR_PHASE_RANGE = Range{0.0f, 1.0f};
constexpr auto BIPOLAR_PHASE_RANGE = Range{-1.0f, 1.0f};
constexpr auto UNIPOLAR_SIGNAL_RANGE = Range{0.0f, 10.0f};
constexpr auto BIPOLAR_SIGNAL_RANGE = Range{-5.0f, 5.0f};
}
