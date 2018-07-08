#pragma once

#include <util/d-flip-flop.h>
#include <util/ramp.h>
#include <util/mode.h>

#include <utility>
#include "util/controls.h"
#include "module.h"

namespace rack_plugin_DHE_Modules {

#define sMIN(a,b) (((a)>(b))?(b):(a))
#define sMAX(a,b) (((a)>(b))?(a):(b))

struct BoosterStageModule : Module {
  Mode stage_mode = {};
  Mode defer_mode = {};

  // TODO: Move this inside stage mode or an envelope class.
  float phase_0_voltage{0.f};
  bool is_active{false};
  bool is_eoc{false};

  Ramp envelope = Ramp{[this] { return sample_time()/duration_in(); }};
  Ramp eoc_pulse = Ramp{[this] { return sample_time()/1e-3f; }};
  DFlipFlop envelope_trigger = DFlipFlop{[this] { return envelope_gate_in(); }};

  SubmodeSwitch executor = {[this] { return defer_gate_in(); }, &stage_mode, &defer_mode};

  BoosterStageModule()
      : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {

    defer_mode.on_entry([this] {
      is_active = true;
    });
    defer_mode.on_step([this] {
      send_envelope(envelope_in());
    });

    stage_mode.on_entry([this] {
      is_active = false;
      phase_0_voltage = envelope_in();
      envelope_trigger.enable();
    });
    stage_mode.on_step([this] {
      envelope_trigger.step();
      envelope.step();
      send_envelope(envelope_voltage(envelope.phase())); // TODO: Move to envelope.on_step()
    });
    stage_mode.on_exit([this] {
      envelope_trigger.disable();
      envelope.stop();
    });

    envelope_trigger.on_rise([this] {
      phase_0_voltage = envelope_in();
      envelope.start();
    });

    envelope.on_start([this] {
      is_active = true;
    });
    envelope.on_completion([this] {
      is_active = false;
      eoc_pulse.start();
    });

    eoc_pulse.on_start([this] {
      is_eoc = true;
    });
    eoc_pulse.on_completion([this] {
      is_eoc = false;
    });

    executor.on_step([this] { eoc_pulse.step(); });
    executor.enter();
  }

  void step() override {
    executor.step();
    send_active();
    send_eoc();
  }

  float defer_gate_in() const {
    return sMAX(input(DEFER_IN), gate_button(DEFER_BUTTON));
  }

  float duration_in() const {
    auto rotation = modulated(DURATION_KNOB, DURATION_CV);
    const auto &range = Duration::range(param(DURATION_SWITCH));
    return Duration::scaled(rotation, range);
  }

  float envelope_gate_in() const {
    return sMAX(input(TRIGGER_IN), gate_button(TRIGGER_BUTTON));
  }

  float envelope_in() const {
    return input(ENVELOPE_IN);
  }

  float envelope_voltage(float phase) const {
    return scale(taper(phase), phase_0_voltage, level_in());
  }

  bool is_s_taper() const {
    return param(SHAPE_SWITCH) > 0.5;
  }

  float level_in() const {
    const auto &range = Level::range(param(LEVEL_SWITCH));
    auto rotation = modulated(LEVEL_KNOB, LEVEL_CV);
    return Level::scaled(rotation, range);
  }

  void send_active() {
    outputs[ACTIVE_OUT].value = UNIPOLAR_SIGNAL_RANGE.scale(is_active || param(ACTIVE_BUTTON) > 0.5f);
  }

  void send_envelope(float voltage) {
    outputs[ENVELOPE_OUT].value = voltage;
  }

  void send_eoc() {
    outputs[EOC_OUT].value = UNIPOLAR_SIGNAL_RANGE.scale(is_eoc || param(EOC_BUTTON) > 0.5f);
  }

  float sample_time() const {
    return rack::engineGetSampleTime();
  }

  float taper(float phase) const {
    auto rotation = modulated(CURVE_KNOB, CURVE_CV);
    return is_s_taper() ? Taper::s(phase, rotation) : Taper::j(phase, rotation);
  }

  enum ParameterIds {
    ACTIVE_BUTTON,
    CURVE_KNOB,
    DEFER_BUTTON,
    DURATION_KNOB,
    DURATION_SWITCH,
    EOC_BUTTON,
    LEVEL_KNOB,
    LEVEL_SWITCH,
    SHAPE_SWITCH,
    TRIGGER_BUTTON,
    PARAMETER_COUNT
  };

  enum InputIds {
    CURVE_CV,
    DEFER_IN,
    DURATION_CV,
    LEVEL_CV,
    ENVELOPE_IN,
    TRIGGER_IN,
    INPUT_COUNT
  };

  enum OutputIds {
    ACTIVE_OUT,
    EOC_OUT,
    ENVELOPE_OUT,
    OUTPUT_COUNT
  };
};
}
