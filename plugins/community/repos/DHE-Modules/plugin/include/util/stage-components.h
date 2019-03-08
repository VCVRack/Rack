#pragma once

#include "gate.h"
#include "mode.h"
#include "phase-accumulator.h"
#include "trigger.h"

namespace DHE {
template <typename M> class DeferGate : public Gate {
public:
  explicit DeferGate(M *module) : module{module} {}

protected:
  auto state_in() const -> bool override { return module->defer_gate_in(); }

  void on_rise() override { module->on_defer_gate_rise(); }

  void on_fall() override { module->on_defer_gate_fall(); }

private:
  M *module;
};

template <typename M> class StageGate : public Gate {
public:
  explicit StageGate(M *module) : module{module} {}

protected:
  auto state_in() const -> bool override { return module->sustain_gate_in(); }

  void on_rise() override { module->on_stage_gate_rise(); }

  void on_fall() override { module->on_stage_gate_fall(); }

private:
  M *module;
};

template <typename M> class StageTrigger : public Trigger {
public:
  explicit StageTrigger(M *module) : module{module} {}

protected:
  auto state_in() const -> bool override { return module->stage_trigger_in(); }

  void on_rise() override { module->on_stage_trigger_rise(); }

private:
  M *module;
};

template <typename M> class StageGenerator : public PhaseAccumulator {
public:
  explicit StageGenerator(M *module) : module{module} {}

  auto duration() const -> float override { return module->duration(); }

  void on_finish() const override { module->on_stage_generator_finish(); }

private:
  M *module;
};

template <typename M> class EocGenerator : public PhaseAccumulator {
public:
  explicit EocGenerator(M *module) : module{module} {}

  void on_start() const override { module->set_eoc(true); }

  auto duration() const -> float override { return 1e-3; }

  void on_finish() const override { module->set_eoc(false); }

private:
  M *module;
};

template <typename M> class DeferringMode : public Mode {
public:
  explicit DeferringMode(M *module) : module{module} {}

  void enter() override { module->set_active(true); }

  void step() override { module->send_input(); }

private:
  M *module;
};

template <typename M> class FollowingMode : public Mode {
public:
  explicit FollowingMode(M *module, Trigger *stage_trigger)
      : module{module}, stage_trigger{stage_trigger} {}

  void enter() override { module->set_active(false); }

  void step() override {
    module->send_stage();
    stage_trigger->step();
  }

private:
  M *module;
  Trigger *stage_trigger;
};

template <typename M> class GeneratingMode : public Mode {
public:
  explicit GeneratingMode(M *module, PhaseAccumulator *stage_generator,
                          Trigger *stage_trigger)
      : module{module}, stage_trigger{stage_trigger}, stage_generator{
                                                          stage_generator} {}

  void enter() override {
    module->hold_input();
    module->set_active(true);
    start();
  }

  void step() override {
    stage_generator->step();
    module->send_stage();
    stage_trigger->step();
  }

  void start() { stage_generator->start(); }

private:
  M *module;
  Trigger *stage_trigger;
  PhaseAccumulator *stage_generator;
};

template <typename M> class SustainingMode : public Mode {
public:
  explicit SustainingMode(M *module, Gate *sustain_gate)
      : module{module}, sustain_gate{sustain_gate} {}

  void enter() override {
    module->hold_input();
    module->set_active(true);
  }

  void step() override {
    module->send_held();
    sustain_gate->step();
  }

private:
  M *module;
  Gate *sustain_gate;
};
} // namespace DHE
