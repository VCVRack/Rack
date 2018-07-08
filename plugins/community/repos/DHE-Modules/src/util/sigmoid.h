#pragma once

#include <cmath>
#include <functional>

#include "range.h"

namespace rack_plugin_DHE_Modules {

inline float sigmoid(float x, float curvature) {
  static constexpr auto precision = 1e-4f;
  static constexpr auto max_curvature = 1.0f - precision;
  static const auto curvature_range = Range{-max_curvature, max_curvature};

  curvature = curvature_range.clamp(curvature);
  x = BIPOLAR_PHASE_RANGE.clamp(x);

  return (x - x*curvature)/(curvature - std::abs(x)*2.0f*curvature + 1.0f);
}
}
