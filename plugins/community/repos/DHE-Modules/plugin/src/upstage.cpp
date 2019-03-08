#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/range.h"
#include "util/rotation.h"
#include "util/signal.h"

namespace DHE {

class Upstage : public rack::Module {
public:
  Upstage() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  void step() override {
    auto is_triggered = trigger_in() && !wait_in();
    send_trigger(is_triggered);
    send_envelope(envelope_voltage());
  }

  const Selector<Range const *> level_range_selector{
      Signal::ranges, [this](Range const *range) { level_range = range; }};

  enum ParameterIds {
    LEVEL_KNOB,
    TRIGGER_BUTTON,
    WAIT_BUTTON,
    LEVEL_RANGE_SWITCH,
    PARAMETER_COUNT
  };

  enum InputIds { TRIGGER_IN, WAIT_IN, LEVEL_CV, INPUT_COUNT };

  enum OutputIds { TRIGGER_OUT, MAIN_OUT, OUTPUT_COUNT };

private:
  auto envelope_voltage() const -> float { return level_range->scale(level()); }

  auto level() const -> float { return modulated(LEVEL_KNOB, LEVEL_CV); }

  auto modulated(ParameterIds knob_param, InputIds cv_input) const -> float {
    auto rotation = params[knob_param].value;
    auto cv = inputs[cv_input].value;
    return Rotation::modulated(rotation, cv);
  }

  void send_envelope(float voltage) { outputs[MAIN_OUT].value = voltage; }

  void send_trigger(bool is_triggered) {
    outputs[TRIGGER_OUT].value = is_triggered ? 10.f : 0.f;
  }

  auto trigger_in() const -> bool {
    auto trigger_button = params[TRIGGER_BUTTON].value > 0.1f;
    auto trigger_input = inputs[TRIGGER_IN].value > 0.1f;
    return trigger_button || trigger_input;
  }

  auto wait_in() const -> bool {
    auto wait_button = params[WAIT_BUTTON].value > 0.1f;
    auto wait_input = inputs[WAIT_IN].value > 0.1f;
    return wait_button || wait_input;
  }

  Range const *level_range = &Signal::bipolar_range;
};

class UpstagePanel : public Panel<UpstagePanel> {
public:
  explicit UpstagePanel(Upstage *module) : Panel{module, hp} {
    auto widget_right_edge = width();

    auto column_1 = width() / 4.f + 0.333333333f;
    auto column_2 = widget_right_edge / 2.f;
    auto column_3 = widget_right_edge - column_1;

    auto y = 25.f;
    auto dy = 18.5f;

    install(column_2, y, knob<LargeKnob>(Upstage::LEVEL_KNOB));

    y += dy;
    install(column_1, y, input(Upstage::LEVEL_CV));
    install(column_3, y,
            toggle<2>(Upstage::LEVEL_RANGE_SWITCH, 1,
                      module->level_range_selector));

    y += dy;
    install(column_1, y, button(Upstage::WAIT_BUTTON));
    install(column_3, y, button(Upstage::TRIGGER_BUTTON));

    y = 82.f;
    dy = 15.f;

    install(column_1, y, input(Upstage::WAIT_IN));

    y += dy;
    install(column_1, y, input(Upstage::TRIGGER_IN));
    install(column_3, y, output(Upstage::TRIGGER_OUT));

    y += dy;
    install(column_3, y, output(Upstage::MAIN_OUT));
  }

  static constexpr auto module_slug = "upstage";

private:
  static constexpr auto hp = 5;
};
} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Upstage) {
   rack::Model *modelUpstage =
      rack::Model::create<DHE::Upstage, DHE::UpstagePanel>(
         "DHE-Modules", "Upstage", "Upstage", rack::ENVELOPE_GENERATOR_TAG);
// rack::createModel<DHE::Upstage, DHE::UpstagePanel>("Upstage");
   return modelUpstage;
}
