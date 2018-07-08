#pragma once

#include <algorithm>

#include <engine.hpp>

#include "module.h"
#include "util/controls.h"
#include "util/range.h"

namespace rack_plugin_DHE_Modules {

struct SwaveModule : Module {
  SwaveModule() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  bool is_s_taper() const {
    return param(SHAPE_SWITCH) > 0.5f;
  }

  void step() override {
    outputs[SWAVE_OUT].value = to_signal(taper(to_phase(swave_in())));
  }

  float swave_in() const { return input(SWAVE_IN); }

  float taper(float phase) const {
    auto rotation = modulated(CURVE_KNOB, CURVE_CV);
    return is_s_taper() ? Taper::s(phase, rotation) : Taper::j(phase, rotation);
  }

  float to_signal(float phase) const {
    return BIPOLAR_SIGNAL_RANGE.scale(phase);
  }

  float to_phase(float signal) const {
    return BIPOLAR_SIGNAL_RANGE.normalize(signal);
  }

  enum ParameterIds {
    CURVE_KNOB,
    SHAPE_SWITCH,
    PARAMETER_COUNT
  };
  enum InputIds {
    CURVE_CV,
    SWAVE_IN,
    INPUT_COUNT
  };
  enum OutputIds {
    SWAVE_OUT,
    OUTPUT_COUNT
  };
};

}
