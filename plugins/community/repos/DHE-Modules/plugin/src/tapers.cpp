#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/rotation.h"
#include "util/sigmoid.h"
#include "util/signal.h"

namespace DHE {

class Tapers : public rack::Module {
public:
  Tapers() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  void step() override {
    outputs[OUT_1].value = taper(level1(), is_uni_1(), curvature1(), is_s_1());

    outputs[OUT_2].value = taper(level2(), is_uni_2(), curvature2(), is_s_2());
  }

  enum ParameterIds {
    LEVEL_1_KNOB,
    LEVEL_1_AV,
    RANGE_1_SWITCH,
    CURVE_1_KNOB,
    CURVE_1_AV,
    SHAPE_1_SWITCH,
    LEVEL_2_KNOB,
    LEVEL_2_AV,
    RANGE_2_SWITCH,
    CURVE_2_KNOB,
    CURVE_2_AV,
    SHAPE_2_SWITCH,
    PARAMETER_COUNT
  };
  enum InputIds { LEVEL_1_CV, CURVE_1_CV, LEVEL_2_CV, CURVE_2_CV, INPUT_COUNT };
  enum OutputIds { OUT_1, OUT_2, OUTPUT_COUNT };

private:
  auto curvature(ParameterIds knob, InputIds cv, ParameterIds av) const
      -> float {
    auto curvature = modulated(knob, cv, av);
    return Sigmoid::curvature(curvature);
  }

  auto curvature1() const -> float {
    return curvature(CURVE_1_KNOB, CURVE_1_CV, CURVE_1_AV);
  }

  auto curvature2() const -> float {
    return curvature(CURVE_2_KNOB, CURVE_2_CV, CURVE_2_AV);
  }

  auto is_uni_1() const -> bool { return params[RANGE_1_SWITCH].value > 0.5f; }
  auto is_uni_2() const -> bool { return params[RANGE_2_SWITCH].value > 0.5f; }
  auto is_s_1() const -> bool { return params[SHAPE_1_SWITCH].value > 0.5f; }
  auto is_s_2() const -> bool { return params[SHAPE_2_SWITCH].value > 0.5f; }

  auto level1() const -> float {
    return modulated(LEVEL_1_KNOB, LEVEL_1_CV, LEVEL_1_AV);
  }

  auto level2() const -> float {
    return modulated(LEVEL_2_KNOB, LEVEL_2_CV, LEVEL_2_AV);
  }

  auto modulated(ParameterIds knob_param, InputIds cv_input,
                 ParameterIds av_param) const -> float {
    auto rotation = params[knob_param].value;
    auto cv = inputs[cv_input].value;
    auto av = params[av_param].value;
    return Rotation::modulated(rotation, cv, av);
  }

  auto taper(float level, bool is_uni, float curve, bool is_s) const -> float {
    auto tapered = Sigmoid::taper(level, curve, is_s);
    return Signal::range(is_uni).scale(tapered);
  }
};

class TapersPanel : public Panel<TapersPanel> {
public:
  explicit TapersPanel(Tapers *module) : Panel{module, hp} {
    auto widget_right_edge = width();

    auto column_1 = width() / 5.f + 0.333333333f;
    auto column_2 = widget_right_edge / 2.f;
    auto column_3 = widget_right_edge - column_1;

    auto y = 24.f;
    auto dy = 16.f;
    auto panel_buffer = 4.f;

    install(column_1, y, input(Tapers::LEVEL_1_CV));
    install(column_2, y, knob<TinyKnob>(Tapers::LEVEL_1_AV));
    install(column_3, y, knob<MediumKnob>(Tapers::LEVEL_1_KNOB));
    y += dy;
    install(column_1, y, input(Tapers::CURVE_1_CV));
    install(column_2, y, knob<TinyKnob>(Tapers::CURVE_1_AV));
    install(column_3, y, knob<MediumKnob>(Tapers::CURVE_1_KNOB));
    y += dy;
    install(column_1, y, toggle<2>(Tapers::SHAPE_1_SWITCH, 0));
    install(column_2, y, toggle<2>(Tapers::RANGE_1_SWITCH, 1));
    install(column_3, y, output(Tapers::OUT_1));

    y += dy + panel_buffer;

    install(column_1, y, input(Tapers::LEVEL_2_CV));
    install(column_2, y, knob<TinyKnob>(Tapers::LEVEL_2_AV));
    install(column_3, y, knob<MediumKnob>(Tapers::LEVEL_2_KNOB));
    y += dy;
    install(column_1, y, input(Tapers::CURVE_2_CV));
    install(column_2, y, knob<TinyKnob>(Tapers::CURVE_2_AV));
    install(column_3, y, knob<MediumKnob>(Tapers::CURVE_2_KNOB));
    y += dy;
    install(column_1, y, toggle<2>(Tapers::SHAPE_2_SWITCH, 0));
    install(column_2, y, toggle<2>(Tapers::RANGE_2_SWITCH, 1));
    install(column_3, y, output(Tapers::OUT_2));
  }

  static constexpr auto module_slug = "tapers";

public:
  static constexpr auto hp = 9;
};
} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Tapers) {
   rack::Model *modelTapers = rack::Model::create<DHE::Tapers, DHE::TapersPanel>(
      "DHE-Modules", "Tapers", "Tapers", rack::UTILITY_TAG);
   return modelTapers;
}
