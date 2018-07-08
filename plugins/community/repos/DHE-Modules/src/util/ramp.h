#pragma once

#include <functional>
#include <utility>

#include "d-latch.h"
#include "sigmoid.h"

namespace rack_plugin_DHE_Modules {

/**
 * A ramp that advances its phase from 0 to 1 in increments supplied by a given
 * supplier.
 *
 * When the phase advances to 1, the ramp fires 'completion' and stops
 * advancing.
 */
class Ramp {
public:
  /*!
   * Constructs a ramp that advances its phase on each step by the amount
   * supplied by the given supplier.
   *
   * The newly constructed ramp is at phase 0 and inactive.
   *
   * @param phase_increment_supplier supplies the amount to advance the phase on
   * each step
   */
  explicit Ramp(std::function<float()> phase_increment_supplier)
      : phase_increment{std::move(phase_increment_supplier)} {
    stop();
  }

  /**
   * Sets the phase to 0, actives the ramp, and fires 'start'. Subsequent steps
   * will advance the phase.
   */
  void start() {
    progress = 0.0;
    active.enable();
    active.set();
  }

  /*!
   * Sets the phase to 0 and deactives the ramp. Subsequent steps will not
   * advance the phase.
   *
   * Stopping the ramp does not fire any events.
   */
  void stop() {
    progress = 0.0;
    active.disable();
    active.reset();
  }

  /**
   * Advances the phase by the amount supplied by the phase increment supplier.
   *
   * If the phase advances to 1, the ramp fires 'completion' and becomes
   * inactive.
   *
   * If the ramp is inactive, this function has no effect.
   */
  void step() {
    if (!is_active())
      return;

    progress = UNIPOLAR_PHASE_RANGE.clamp(progress + phase_increment());

    if (progress >= 1.0f) {
      active.reset();
    };
  }

  /**
   * Indicates whether the ramp is active.
   *
   * @return whether the ramp is active
   */
  bool is_active() const { return active.is_high(); }

  /**
   * Returns the ramp's phase.
   *
   * If the ramp is active, the phase indicates the ramp's progress
   * from 0 to 1.
   *
   * If the ramp is inactive, the phase is 1 (if the ramp became inactive by
   * completing its advancement) or 0 (if the ramp was stopped).
   *
   * @return the ramp's phase
   */
  float phase() const { return progress; }

  /**
   * Registers an action to be called when the ramp starts.
   * @param action called when the ramp starts
   */
  void on_start(std::function<void()> action) {
    active.on_rise(std::move(action));
  }

  /**
   * Registers an action to be called when the ramp's phase advances to 1.
   * @param action called when the ramp's phase advances to 1
   */
  void on_completion(std::function<void()> action) {
    active.on_fall(std::move(action));
  }

private:
  float progress = 0.0f;
  DLatch active{};
  const std::function<float()> phase_increment;
};
}
