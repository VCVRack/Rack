#pragma once

namespace DHE {

inline auto scale(float proportion, float lower_bound, float upper_bound)
    -> float {
  return proportion * (upper_bound - lower_bound) + lower_bound;
}

class Range {
public:
  constexpr Range(float lower_bound, float upper_bound) noexcept
      : lower_bound(lower_bound), upper_bound(upper_bound) {}

  auto scale(float proportion) const -> float {
    return DHE::scale(proportion, lower_bound, upper_bound);
  }

  auto scale(bool state) const -> float {
    return state ? upper_bound : lower_bound;
  }

  auto normalize(float member) const -> float {
    return (member - lower_bound) / (upper_bound - lower_bound);
  }

  auto clamp(float f) const -> float {
    if (f < lower_bound) {
      return lower_bound;
    }
    if (f > upper_bound) {
      return upper_bound;
    }
    return f;
  }

  const float lower_bound;
  const float upper_bound;
};
} // namespace DHE
