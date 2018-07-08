#pragma once

#include <engine.hpp>

#include "util/range.h"

namespace rack_plugin_DHE_Modules {

struct Module : rack::Module {
  Module(int param_count, int input_count, int output_count)
      : rack::Module(param_count, input_count, output_count) {}

  float gate_button(int index) const {
    return UNIPOLAR_SIGNAL_RANGE.scale(param(index) > 0.5f);
  };

  float input(int index) const {
    return inputs[index].value;
  }

  float modulated(int param_index, int mod_index) const {
    return param(param_index) + input(mod_index)/10.f;
  }

  float param(int index) const {
    return params[index].value;
  }
};

}
