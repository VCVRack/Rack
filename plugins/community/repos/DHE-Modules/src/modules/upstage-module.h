#pragma once

#include <algorithm>

#include "module.h"
#include "util/controls.h"
#include "util/range.h"

namespace rack_plugin_DHE_Modules {

struct UpstageModule : Module {
  UpstageModule() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  float envelope_out() const {
    const auto &range = Level::range(params[LEVEL_SWITCH].value);
    auto rotation = modulated(LEVEL_KNOB, LEVEL_CV);
    return Level::scaled(rotation, range);
  }

  bool is_waiting() const {
    return std::max(inputs[WAIT_IN].value, gate_button(WAIT_BUTTON)) > 0.5f;
  }

  void step() override {
    outputs[TRIG_OUT].value = trigger_out();
    outputs[ENVELOPE_OUT].value = envelope_out();
  }

  float trigger_in() const {
    return param(TRIG_BUTTON) > 0.5f ? 10.f : input(TRIG_IN);
  }

  float trigger_out() const {
    return is_waiting() ? 0.f : trigger_in();
  }

  enum ParameterIds {
    LEVEL_KNOB,
    TRIG_BUTTON,
    WAIT_BUTTON,
    LEVEL_SWITCH,
    PARAMETER_COUNT
  };
  enum InputIds {
    TRIG_IN,
    WAIT_IN,
    LEVEL_CV,
    INPUT_COUNT
  };
  enum OutputIds {
    TRIG_OUT,
    ENVELOPE_OUT,
    OUTPUT_COUNT
  };
};

}
