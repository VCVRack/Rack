#pragma once

#include "range.h"
#include "sigmoid.h"

namespace rack_plugin_DHE_Modules {

namespace Duration {
constexpr auto RANGE_MAX_TO_MIN_RATIO = 1000.f;
constexpr auto MEDIUM_RANGE_MAX = 10.0f;
constexpr auto MEDIUM_RANGE_MIN = MEDIUM_RANGE_MAX/RANGE_MAX_TO_MIN_RATIO;

constexpr auto SCALE_STEP = 10.f;
constexpr auto SHORT_RANGE = Range{MEDIUM_RANGE_MIN/SCALE_STEP, MEDIUM_RANGE_MAX/SCALE_STEP};
constexpr auto MEDIUM_RANGE = Range{MEDIUM_RANGE_MIN, MEDIUM_RANGE_MAX};
constexpr auto LONG_RANGE = Range{MEDIUM_RANGE_MIN*SCALE_STEP, MEDIUM_RANGE_MAX*SCALE_STEP};

inline const Range &range(float switch_value) {
  return switch_value < 0.5f ? SHORT_RANGE :
         switch_value < 1.5f ? MEDIUM_RANGE : LONG_RANGE;
}

inline float scaled(float rotation, const Range &range) {
  // Shapes the J taper to yield ~0.1 given a rotation of 0.5. So if the
  // rotation is at dead center, the return value will be ~1/10 of the range's
  // maximum.
  constexpr auto KNOB_CURVATURE = 0.8f;

  // Assuming a rotation value in the range [0,1], sigmoid() yields a J taper
  // in the range [0,1]
  auto j_tapered_rotation = sigmoid(rotation, KNOB_CURVATURE);

  // Scale the tapered rotation to the desired range.
  return range.scale(j_tapered_rotation);
}
}

namespace Level {

inline const Range &range(float switch_value) {
  return switch_value > 0.5f ? UNIPOLAR_SIGNAL_RANGE : BIPOLAR_SIGNAL_RANGE;
}

inline float scaled(float rotation, const Range &range = UNIPOLAR_SIGNAL_RANGE) {
  return range.scale(rotation);
}
}

namespace Taper {

inline float curvature(float rotation) {
  // Scale the rotation to [-1,1] to use the entire range of the sigmoid curve.
  auto bipolar_rotation = BIPOLAR_PHASE_RANGE.scale(rotation);

  // This knob curvature gives an S taper that gently increases sensitivity in
  // the middle of the rotation and decreases sensitivity toward the ends.
  constexpr auto KNOB_CURVATURE = -0.65f;

  // Apply the S taper to the bipolar rotation.
  return sigmoid(bipolar_rotation, KNOB_CURVATURE);
}

inline float j(float phase, float rotation) {
  return sigmoid(phase, curvature(rotation));
}

inline float s(float phase, float rotation) {
  // Scale the phase to [-1,1] to use the entire range of the sigmoid curve.
  auto bipolar_phase = BIPOLAR_PHASE_RANGE.scale(phase);

  // Invert the curvature so that rotation greater than 0.5 gives an S taper.
  auto s_tapered_bipolar_phase = sigmoid(bipolar_phase, -curvature(rotation));

  // Scale the tapered phase back to the range [0,1].
  return BIPOLAR_PHASE_RANGE.normalize(s_tapered_bipolar_phase);
}

}
}
