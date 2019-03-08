#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/range.h"
#include "util/rotation.h"
#include "util/signal.h"

namespace DHE {

class Ranger : public rack::Module {
public:
  Ranger() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  void step() override { send_main_out(scale(level(), limit2(), limit1())); }

  enum ParameterIds {
    LEVEL_KNOB,
    LEVEL_AV,
    LIMIT_1_KNOB,
    LIMIT_1_AV,
    LIMIT_1_RANGE_SWITCH,
    LIMIT_2_KNOB,
    LIMIT_2_AV,
    LIMIT_2_RANGE_SWITCH,
    PARAMETER_COUNT
  };
  enum InputIds { LEVEL_CV, LIMIT_1_CV, LIMIT_2_CV, INPUT_COUNT };
  enum OutputIds { MAIN_OUT, OUTPUT_COUNT };

private:
  auto level() const -> float {
    return modulated(LEVEL_KNOB, LEVEL_CV, LEVEL_AV);
  }

  auto limit(ParameterIds knob_param, InputIds cv_input, ParameterIds av_param,
             int range_switch_param) const -> float {
    auto is_uni = params[range_switch_param].value > 0.5f;
    auto range = Signal::range(is_uni);
    return range.scale(modulated(knob_param, cv_input, av_param));
  }

  auto limit1() const -> float {
    return limit(LIMIT_1_KNOB, LIMIT_1_CV, LIMIT_1_AV, LIMIT_1_RANGE_SWITCH);
  }

  auto limit2() const -> float {
    return limit(LIMIT_2_KNOB, LIMIT_2_CV, LIMIT_2_AV, LIMIT_2_RANGE_SWITCH);
  }

  auto modulated(ParameterIds knob_param, InputIds cv_input,
                 ParameterIds av_parm) const -> float {
    auto rotation = params[knob_param].value;
    auto cv = inputs[cv_input].value;
    auto av = params[av_parm].value;
    return Rotation::modulated(rotation, cv, av);
  }

  void send_main_out(float voltage) { outputs[MAIN_OUT].value = voltage; }
};

class RangerPanel : public Panel<RangerPanel> {
public:
  explicit RangerPanel(Ranger *module) : Panel{module, hp} {
    auto widget_right_edge = width();

    auto column_1 = width() / 3.5f + 0.333333333f;
    auto column_2 = widget_right_edge - column_1;

    auto y = 24.f;
    auto dy = 16.f;
    auto panel_buffer = 4.f;

    install(column_1, y, knob<MediumKnob>(Ranger::LEVEL_KNOB));
    install(column_2, y, output(Ranger::MAIN_OUT));

    y += dy;
    install(column_1, y, input(Ranger::LEVEL_CV));
    install(column_2, y, knob<TinyKnob>(Ranger::LEVEL_AV));

    y += dy + panel_buffer;
    install(column_1, y, knob<MediumKnob>(Ranger::LIMIT_1_KNOB));
    install(column_2, y, toggle<2>(Ranger::LIMIT_1_RANGE_SWITCH, 1));

    y += dy;
    install(column_1, y, input(Ranger::LIMIT_1_CV));
    install(column_2, y, knob<TinyKnob>(Ranger::LIMIT_1_AV));

    y += dy + panel_buffer;
    install(column_1, y, knob<MediumKnob>(Ranger::LIMIT_2_KNOB));
    install(column_2, y, toggle<2>(Ranger::LIMIT_2_RANGE_SWITCH, 1));

    y += dy;
    install(column_1, y, input(Ranger::LIMIT_2_CV));
    install(column_2, y, knob<TinyKnob>(Ranger::LIMIT_2_AV));
  }

  static constexpr auto module_slug = "ranger";

private:
  static constexpr auto hp = 6;
};
} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Ranger) {
   rack::Model *modelRanger = rack::Model::create<DHE::Ranger, DHE::RangerPanel>(
      "DHE-Modules", "Ranger", "Ranger", rack::UTILITY_TAG);
   return modelRanger;
}

