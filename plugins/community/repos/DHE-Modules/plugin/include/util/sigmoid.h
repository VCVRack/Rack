#pragma once

#include "range.h"

namespace DHE {
namespace Sigmoid {

static constexpr auto sigmoid_range = Range{-1.f, 1.f};
static constexpr auto proportion_range = Range{0.f, 1.f};

/**
 * Applies an inverse sigmoid function to the input.
 * <p>
 * The curvature determines the shape and intensity of the transfer function.
 * A positive curvature applies an inverted S-shaped transfer function.
 * A curvature of 0 applies a linear transfer function.
 * A negative curvature applies an S-shaped transfer function.
 * <p>
 * Before the function is applied:
 * <ul>
 * <li>The input is clamped to the range [-1.0, 1.0].</li>
 * <li>The curvature is clamped to the range [0.0001, 0.9999].</li>
 * </ul>
 * @param input the input to the inverse sigmoid function
 * @param curvature the intensity and direction of the curvature
 * @return the sigmoid function result
 */
inline auto inverse(float input, float curvature) -> float {
  static constexpr auto precision = 1e-4f;
  static constexpr auto max_curvature = 1.0f - precision;
  static constexpr auto curvature_range = Range{-max_curvature, max_curvature};

  curvature = curvature_range.clamp(curvature);
  input = sigmoid_range.clamp(input);

  return (input - input * curvature) /
         (curvature - std::abs(input) * 2.0f * curvature + 1.0f);
}

/**
 * Applies a sigmoid function to the input.
 * <p>
 * The curvature determines the shape and intensity of the transfer function.
 * A positive curvature applies an S-shaped transfer function.
 * A curvature of 0 applies a linear transfer function.
 * A negative curvature applies an inverted S-shaped transfer function.
 * <p>
 * Before the function is applied:
 * <ul>
 * <li>The input is clamped to the range [-1.0, 1.0].</li>
 * <li>The curvature is clamped to the range [0.0001, 0.9999].</li>
 * </ul>
 * @param input the input to the sigmoid function
 * @param curvature the intensity and direction of the curvature
 * @return the sigmoid function result
 */
inline auto curve(float input, float curvature) -> float {
  return inverse(input, -curvature);
}

/**
 * Applies a gentle S-shaped transfer function to map an input in the range
 * [0.0, 1.0] to an output in the range [-1.0, 1.0]. The transfer function makes
 * the output more sensitive to changes in inputs near 0.5 and less sensitive to
 * changes near 0.0 and 1.0.
 * <p>
 * This function is intended to translate DHE-Modules CURVE knob rotation to a
 * curvature value suitable to pass to the curve(), inverse(), j_taper(), and
 * s_taper() functions.
 *
 * @param input the value to map to a curvature
 * @return the curvature
 */
inline auto curvature(float input) -> float {
  // This curvature creates a gentle S curve, increasing sensitivity in the
  // middle of the input range and decreasing sensitivity toward the extremes.
  static constexpr auto gentle_s = 0.65f;
  auto scaled = sigmoid_range.scale(input);
  return curve(scaled, gentle_s);
}

/**
 * Applies a J-shaped transfer function to the input.
 * <p>
 * The curvature determines the shape and intensity of the taper.
 * A positive curvature applies a J-taper.
 * A curvature of 0 applies a linear taper.
 * A negative curvature applies an inverted J-taper.
 * <p>
 * Before the function is applied:
 * <ul>
 * <li>The input is clamped to the range [0.0, 1.0].</li>
 * <li>The curvature is clamped to the range [0.0001, 0.9999].</li>
 * </ul>
 * @param input the input to the taper function
 * @param curvature the intensity and direction of the taper
 * @return the taper function result
 */
inline auto j_taper(float input, float curvature) -> float {
  return inverse(proportion_range.clamp(input), curvature);
}

/**
 * Applies an S-shaped transfer function to the input.
 * <p>
 * The curvature determines the shape and intensity of the taper.
 * A positive curvature applies an S-taper.
 * A curvature of 0 applies a linear taper.
 * A negative curvature applies an inverted S-taper.
 * <p>
 * Before the function is applied:
 * <ul>
 * <li>The input is clamped to the range [0.0, 1.0].</li>
 * <li>The curvature is clamped to the range [0.0001, 0.9999].</li>
 * </ul>
 * @param input the input to the taper function
 * @param curvature the intensity and direction of the taper
 * @return the taper function result
 */
inline auto s_taper(float input, float curvature) -> float {
  const auto scaled = sigmoid_range.scale(input);
  const auto tapered = curve(scaled, curvature);
  return sigmoid_range.normalize(tapered);
}

inline auto taper(float input, float curvature, bool is_s) -> float {
  return is_s ? s_taper(input, curvature) : j_taper(input, curvature);
}
} // namespace Sigmoid
} // namespace DHE
