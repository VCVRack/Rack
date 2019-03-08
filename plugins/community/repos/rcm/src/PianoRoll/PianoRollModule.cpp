#include "../../include/PianoRoll/PianoRollModule.hpp"

using namespace rack;

namespace rack_plugin_rcm {

static const float PLUGGED_GATE_DURATION = std::numeric_limits<float>::max();
static const float AUDITION_GATE_DURATION = std::numeric_limits<float>::max();
static const float UNPLUGGED_GATE_DURATION = 2.0f;


PianoRollModule::PianoRollModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), runInputActive(false), transport(&patternData) {
}

void PianoRollModule::onReset() {
  transport.reset();
  patternData.reset();
}

json_t *PianoRollModule::toJson() {
  json_t *rootJ = Module::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  json_object_set_new(rootJ, "patterns", patternData.toJson());
  json_object_set_new(rootJ, "currentPattern", json_integer(transport.currentPattern()));
  json_object_set_new(rootJ, "currentStep", json_integer(transport.currentStepInPattern()));
  json_object_set_new(rootJ, "clockDelay", json_integer(clockDelay));
  json_object_set_new(rootJ, "sequenceRunning", json_boolean(transport.isRunning()));

  return rootJ;
}

void PianoRollModule::fromJson(json_t *rootJ) {
  Module::fromJson(rootJ);

  json_t *clockDelayJ = json_object_get(rootJ, "clockDelay");
  if (clockDelayJ) {
    clockDelay = json_integer_value(clockDelayJ);
  }

  json_t *patternsJ = json_object_get(rootJ, "patterns");
  if (patternsJ) {
    patternData.fromJson(patternsJ);
  }

  json_t *currentPatternJ = json_object_get(rootJ, "currentPattern");
  if (currentPatternJ) {
    transport.setPattern(json_integer_value(currentPatternJ));
  }

  json_t *currentStepJ = json_object_get(rootJ, "currentStep");
  if (currentStepJ) {
    transport.setStepInPattern(json_integer_value(currentStepJ));
  }

  json_t *sequenceRunningJ = json_object_get(rootJ, "sequenceRunning");
  if (sequenceRunningJ) {
    transport.setRun(json_boolean_value(sequenceRunningJ));
  }
}

int quantizePitch(float voct) {
	int oct = floor(voct);
	int note = abs(static_cast<int>( roundf( ( voct * 12.0f) ) ) ) % 12;
	if (voct < 0.0f && note > 0) {
		note = 12 - note;
	}

	return ((oct + 4) * 12) + note;
}

void PianoRollModule::step() {
	bool clockTick = false;

	while((int)clockBuffer.size() <= clockDelay) {
		clockBuffer.push(inputs[CLOCK_INPUT].value);
	}

	float currentClockLevel = 0.f;

	while((int)clockBuffer.size() > clockDelay) {
		currentClockLevel = clockBuffer.shift();
		clockTick |= clockInputTrigger.process(currentClockLevel);
	}

	if (resetInputTrigger.process(inputs[RESET_INPUT].value)) {
    transport.reset();
		gateOutputPulse.reset();
		if (currentClockLevel > 1.f) {
			clockTick = true;
		}
	}

	if (inputs[PATTERN_INPUT].active) {
		int nextPattern = clamp(quantizePitch(inputs[PATTERN_INPUT].value)  - 48, 0, 63);
    transport.setPattern(nextPattern);
	}

	if (recordingIn.process(inputs[RECORD_INPUT].value)) {
    transport.toggleRecording();
	}

	if (runInputTrigger.process(inputs[RUN_INPUT].value)) {
		transport.toggleRun();

		if (currentClockLevel > 1.f) {
			clockTick = true;
		}

		if (!transport.isRunning()) {
			gateOutputPulse.reset();
		}
	}

	if (clockTick) {
    transport.advanceStep();
	}

	runInputActive.process(inputs[RUN_INPUT].active);

	if (runInputActive.changed && transport.isRunning()) {
		if (runInputActive.value == true) {
			bool triggerGateAgain = gateOutputPulse.process(0);
			gateOutputPulse.reset();
			if (triggerGateAgain) {
				// We've plugged in, the sequence is running and our gate is high
				// Trigger the gate for the full plugged in duration (forever)
				gateOutputPulse.trigger(PLUGGED_GATE_DURATION);
			}
		}

		if (runInputActive.value == false) {
			float gateTimeRemaining = UNPLUGGED_GATE_DURATION - gateOutputPulse.time;
			bool triggerGateAgain = gateOutputPulse.process(0) && gateTimeRemaining > 0;
			gateOutputPulse.reset();
			if (triggerGateAgain) {
				// We've unplugged and the sequence is running and the gate is high
				// retrigger it for the time remaining if it had been triggered
				// when the cable was already unplugged. This is to prevent the gate sounding
				// forever - even when the clock is stopped
				gateOutputPulse.trigger(gateTimeRemaining);
			}
		}
	}

	if (!transport.isRecording()) {
    voctInBuffer.clear();
    gateInBuffer.clear();
    retriggerInBuffer.clear();
    velocityInBuffer.clear();
	}

	if (transport.isRecording() && transport.isRunning()) {

		while (!voctInBuffer.full()) { voctInBuffer.push(inputs[VOCT_INPUT].value); }
		while (!gateInBuffer.full()) { gateInBuffer.push(inputs[GATE_INPUT].value); }
		while (!retriggerInBuffer.full()) { retriggerInBuffer.push(inputs[RETRIGGER_INPUT].value); }
		while (!velocityInBuffer.full()) { velocityInBuffer.push(inputs[VELOCITY_INPUT].value); }

    int pattern = transport.currentPattern();
		int measure = transport.currentMeasure();
		int stepInMeasure = transport.currentStepInMeasure();

		if (inputs[VOCT_INPUT].active) {
			auto voctIn = voctInBuffer.shift();
			patternData.setStepPitch(pattern, measure, stepInMeasure, quantizePitch(voctIn));
		}

		if (inputs[GATE_INPUT].active) {
			auto gateIn = gateInBuffer.shift();

			if (clockTick && gateIn < 0.1f) {
        // Only turn off at the start of the step, user may let go early - we still want this step active
				patternData.setStepActive(pattern, measure, stepInMeasure, false);
			}
			
			if (gateIn >= 1.f) {
				patternData.setStepActive(pattern, measure, stepInMeasure, true);
			}
		}

		if (inputs[RETRIGGER_INPUT].active) {
			auto retriggerIn = retriggerInBuffer.shift();

			if (clockTick && retriggerIn < 0.1f) {
        // Only turn off at the start of the step, this will only trigger briefly within the step
        patternData.setStepRetrigger(pattern, measure, stepInMeasure, false);
			}
			
			if (retriggerIn >= 1.f) {
        patternData.setStepRetrigger(pattern, measure, stepInMeasure, true);
			}
		}

		if (inputs[VELOCITY_INPUT].active) {
			auto velocityIn = velocityInBuffer.shift();

			if (clockTick) {
        patternData.setStepVelocity(pattern, measure, stepInMeasure, 0.f);
			}
			
			if (velocityIn > 0.f) {
        patternData.increaseStepVelocityTo(pattern, measure, stepInMeasure, rescale(velocityIn, 0.f, 10.f, 0.f, 1.f));
			}
		}

	}

	if (auditioner.isAuditioning()) {
    int pattern = transport.currentPattern();
		int measure = auditioner.stepToAudition() / patternData.getStepsPerMeasure(pattern);
		int stepInMeasure = auditioner.stepToAudition() % patternData.getStepsPerMeasure(pattern);

		if (patternData.isStepActive(pattern, measure, stepInMeasure)) {
			bool retrigger = auditioner.consumeRetrigger();

			if (retrigger) {
				retriggerOutputPulse.trigger(1e-3f);
			}

			gateOutputPulse.trigger(AUDITION_GATE_DURATION);

			outputs[VELOCITY_OUTPUT].value = patternData.getStepVelocity(pattern, measure, stepInMeasure) * 10.f;

			float octave = patternData.getStepPitch(pattern, measure, stepInMeasure) / 12;
			float semitone = patternData.getStepPitch(pattern, measure, stepInMeasure) % 12;

			outputs[VOCT_OUTPUT].value = (octave-4.f) + ((1.f/12.f) * semitone);
		}
	}

	if (auditioner.consumeStopEvent()) {
		gateOutputPulse.reset();
	}

	if (((transport.isRunning() && clockTick)) && !transport.isRecording()) {
		if (transport.isLastStepOfPattern()) {
			eopOutputPulse.trigger(1e-3f);
		}

    int pattern = transport.currentPattern();
		int measure = transport.currentMeasure();
		int stepInMeasure = transport.currentStepInMeasure();

		if (patternData.isStepActive(pattern, measure, stepInMeasure)) {
			if (gateOutputPulse.process(0) == false || patternData.isStepRetriggered(pattern, measure, stepInMeasure)) {
				retriggerOutputPulse.trigger(1e-3f);
			}

			gateOutputPulse.trigger(runInputActive.value ? PLUGGED_GATE_DURATION : UNPLUGGED_GATE_DURATION);

			outputs[VELOCITY_OUTPUT].value = patternData.getStepVelocity(pattern, measure, stepInMeasure) * 10.f;

			float octave = patternData.getStepPitch(pattern, measure, stepInMeasure) / 12;
			float semitone = patternData.getStepPitch(pattern, measure, stepInMeasure) % 12;

			outputs[VOCT_OUTPUT].value = (octave-4.f) + ((1.f/12.f) * semitone);

		} else {
			gateOutputPulse.reset();
		}
	}

	outputs[RETRIGGER_OUTPUT].value = retriggerOutputPulse.process(engineGetSampleTime()) ? 10.f : 0.f;
	outputs[GATE_OUTPUT].value = gateOutputPulse.process(engineGetSampleTime()) ? 10.f : 0.f;
	if (outputs[RETRIGGER_OUTPUT].active == false && outputs[RETRIGGER_OUTPUT].value > 0.f) {
		// If we're not using the retrigger output, the gate output to 0 for the trigger duration instead
		outputs[GATE_OUTPUT].value = 0.f;
	}
	outputs[END_OF_PATTERN_OUTPUT].value = eopOutputPulse.process(engineGetSampleTime()) ? 10.f : 0.f;

	if (inputs[GATE_INPUT].active && inputs[GATE_INPUT].value > 1.f) {
		if (inputs[VOCT_INPUT].active) { outputs[VOCT_OUTPUT].value = inputs[VOCT_INPUT].value; }
		if (inputs[GATE_INPUT].active) { 
			if (outputs[RETRIGGER_OUTPUT].active == false && inputs[RETRIGGER_INPUT].active) {
				outputs[GATE_OUTPUT].value = inputs[GATE_INPUT].value - inputs[RETRIGGER_INPUT].value;
			} else {
				outputs[GATE_OUTPUT].value = inputs[GATE_INPUT].value;
			}
		}
		if (inputs[RETRIGGER_INPUT].active) { outputs[RETRIGGER_OUTPUT].value = inputs[RETRIGGER_INPUT].value; }
		if (inputs[VELOCITY_INPUT].active) { outputs[VELOCITY_OUTPUT].value = inputs[VELOCITY_INPUT].value; }
	}

  // Send our chaining outputs
	outputs[CLOCK_OUTPUT].value = inputs[CLOCK_INPUT].value;
	outputs[RESET_OUTPUT].value = inputs[RESET_INPUT].value;
	outputs[PATTERN_OUTPUT].value = transport.currentPattern() * (1.f/12.f);
	outputs[RUN_OUTPUT].value = inputs[RUN_INPUT].value;
	outputs[RECORD_OUTPUT].value = inputs[RECORD_INPUT].value;
}

} // namespace rack_plugin_rcm
