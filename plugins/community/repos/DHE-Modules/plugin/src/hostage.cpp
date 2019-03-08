#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/duration.h"
#include "util/phase-accumulator.h"
#include "util/rotation.h"
#include "util/stage-components.h"

namespace DHE {

class Hostage : public rack::Module {
public:
  Hostage() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {
    mode->enter();
  }

  void step() override {
    choose_stage_type();
    defer_gate.step();
    mode->step();
    eoc_generator.step();
  }

  auto defer_gate_in() const -> bool {
    return inputs[DEFER_GATE_IN].value > 0.1f;
  }

  auto duration() const -> float {
    auto rotation = modulated(DURATION_KNOB, DURATION_CV);
    return DHE::duration(rotation, *duration_range);
  }

  void hold_input() { held_voltage = envelope_in(); }

  void on_defer_gate_rise() { enter(&deferring_mode); }

  void on_defer_gate_fall() {
    if (stage_type == SUSTAIN && sustain_gate_in()) {
      enter(&generating_mode);
    } else {
      stop_generating();
    }
  }

  void on_stage_gate_rise() { enter(&generating_mode); }

  void on_stage_generator_finish() {
    if (stage_type == HOLD) {
      stop_generating();
    }
  }

  void on_stage_gate_fall() {
    if (stage_type == SUSTAIN) {
      eoc_generator.start();
      enter(&following_mode);
    }
  }

  void send_held() { send_out(held_voltage); }

  void send_input() { send_out(envelope_in()); }

  void send_stage() { send_held(); }

  void set_active(bool active) {
    outputs[ACTIVE_OUT].value = active ? 10.f : 0.f;
  }

  void set_duration_range(Range const *range) { duration_range = range; }

  void set_eoc(bool eoc) { outputs[EOC_OUT].value = eoc ? 10.f : 0.f; }

  auto sustain_gate_in() const -> bool {
    return inputs[STAGE_GATE_IN].value > 0.1f;
  }

  const Selector<Range const *> duration_range_selector{
      Duration::ranges, [this](Range const *range) { duration_range = range; }};

  enum InputIds {
    DEFER_GATE_IN,
    DURATION_CV,
    MAIN_IN,
    STAGE_GATE_IN,
    INPUT_COUNT
  };

  enum OutputIds { ACTIVE_OUT, MAIN_OUT, EOC_OUT, OUTPUT_COUNT };

  enum ParameterIds {
    DURATION_KNOB,
    DURATION_RANGE_SWITCH,
    HOSTAGE_MODE_SWITCH,
    PARAMETER_COUNT
  };

private:
  void choose_stage_type() {
    auto old_stage_type = stage_type;
    stage_type = stage_type_in() ? SUSTAIN : HOLD;

    // If no change, there's nothing more to do.
    if (stage_type == old_stage_type) {
      return;
    }

    // If no stage in progress, there's nothing more to do.
    if (mode != &generating_mode) {
      return;
    }

    // If we're now holding, continue holding.
    if (stage_type == HOLD) {
      return;
    }

    // If the sustain gate is up, continue sustaining.
    if (sustain_gate_in()) {
      return;
    }

    // The sustain gate is down, so there's no sustain to continue.
    stop_generating();
  }

  void enter(Mode *incoming) {
    mode->exit();
    mode = incoming;
    mode->enter();
  }

  auto envelope_in() const -> float { return inputs[MAIN_IN].value; }

  void stop_generating() {
    eoc_generator.start();
    enter(&following_mode);
  }

  auto modulated(ParameterIds knob_param, InputIds cv_input) const -> float {
    auto rotation = params[knob_param].value;
    auto cv = inputs[cv_input].value;
    return Rotation::modulated(rotation, cv);
  }

  void send_out(float voltage) { outputs[MAIN_OUT].value = voltage; }

  auto stage_type_in() const -> bool {
    return params[HOSTAGE_MODE_SWITCH].value > 0.5f;
  }

  DeferGate<Hostage> defer_gate{this};
  StageGate<Hostage> stage_gate{this};

  StageGenerator<Hostage> stage_generator{this};
  EocGenerator<Hostage> eoc_generator{this};

  DeferringMode<Hostage> deferring_mode{this};
  FollowingMode<Hostage> following_mode{this, &stage_gate};
  GeneratingMode<Hostage> generating_mode{this, &stage_generator, &stage_gate};

  enum StageType { HOLD, SUSTAIN };

  Mode *mode{&following_mode};
  float held_voltage{0.f};
  StageType stage_type{HOLD};
  Range const *duration_range = &Duration::medium_range;
};

class HostagePanel : public Panel<HostagePanel> {
public:
  explicit HostagePanel(Hostage *module) : Panel{module, hp} {
    auto widget_right_edge = width();

    auto column_1 = width() / 4.f + 0.333333f;
    auto column_2 = widget_right_edge / 2.f;
    auto column_3 = widget_right_edge - column_1;

    auto y = 25.f;
    auto dy = 18.5f;

    install(column_2, y, toggle<2>(Hostage::HOSTAGE_MODE_SWITCH, 0));

    y += dy;
    install(column_1, y, input(Hostage::DURATION_CV));
    install(column_3, y,
            toggle<3>(Hostage::DURATION_RANGE_SWITCH, 1,
                      module->duration_range_selector));

    y += dy;
    install(column_2, y, knob<LargeKnob>(Hostage::DURATION_KNOB));

    y = 82.f;
    dy = 15.f;

    install(column_1, y, input(Hostage::DEFER_GATE_IN));
    install(column_3, y, output(Hostage::ACTIVE_OUT));

    y += dy;
    install(column_1, y, input(Hostage::STAGE_GATE_IN));
    install(column_3, y, output(Hostage::EOC_OUT));

    y += dy;
    install(column_1, y, input(Hostage::MAIN_IN));
    install(column_3, y, output(Hostage::MAIN_OUT));
  }

  static constexpr auto module_slug = "hostage";

private:
  static constexpr auto hp = 5;
};
} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Hostage) {
   rack::Model *modelHostage =
      rack::Model::create<DHE::Hostage, DHE::HostagePanel>(
         "DHE-Modules", "Hostage", "Hostage", rack::ENVELOPE_GENERATOR_TAG);
   return modelHostage;
}
