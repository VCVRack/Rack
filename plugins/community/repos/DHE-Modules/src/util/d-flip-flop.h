#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "latch.h"

namespace rack_plugin_DHE_Modules {

/**
 * A flip-flop is a latch determines its own state by comparing a signal to the
 * specified thresholds.
 */
struct DFlipFlop : public Latch {
  /**
   * Creates a flip-flop that compares the signal to the given thresholds.
   *
   * @param signal supplies the signal to evaluate
   * @param low_threshold signal value at or below which the flip-flop state is LOW
   * @param high_threshold signal value at or above which the flip-flop state is HIGH
   */
  DFlipFlop(std::function<float()> signal, float low_threshold, float high_threshold)
      : signal{std::move(signal)},
        low_threshold{low_threshold},
        high_threshold{high_threshold} {}

  /**
   * Creates a flip-flop that compares the signal to a low threshold
   * of 0.1 and a high threshold of 0.9.
   *
   * @param signal supplies the signal to evaluate
   */
  explicit DFlipFlop(std::function<float()> signal)
      : signal{std::move(signal)} {}

  /**
   * Sets the state by comparing the signal to the thresholds.
   * - Fires 'rise' if the state changes to HIGH.
   * - Fires 'fall' if the state changes to LOW.
   */
  void step() {
    if (signal() >= high_threshold)
      set_state(State::HIGH);
    else if (signal() <= low_threshold)
      set_state(State::LOW);
  }

private:
  const std::function<float()> signal;
  const float low_threshold = {0.1f};
  const float high_threshold = {0.9f};
};
}
