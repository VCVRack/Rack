#include <stdio.h>
#include <sstream>
#include <iomanip>

#include "alikins.hpp"
#include "dsp/digital.hpp"
// #include "util.hpp"

namespace rack_plugin_Alikins {

/* IdleSwitch
 *
 * What:
 *
 * If no input events are seen at Input Source within the timeout period
 * emit a gate on Idle Gate Output that lasts until there are input events
 * again. Then reset the timeout period.
 *
 * Sort of metaphoricaly like an idle handler or timeout in event based
 * programming like GUI main loops.
 *
 * The timeout period is set by the value
 * of the 'Time before idle' param.
 *
 * If there is a 'Reset idle' source, when it gets an event, the timeout period
 * is reset. After a reset event, the Idle Gate Output will remain on until
 * an input event is seen at Input Source. When there is an input event, the Idle
 * Gate Output is turned off until the expiration of the 'Time before idle' or
 * the next 'Reset idle'.
 *
 * To use the eventloop/gui main loop analogy, a 'Reset idle' event is equilivent to
 * running an idle handler directly (or running a mainloop iteration with no non-idle
 * events pending).
 *
 * Why:
 *
 * Original intentional was to use in combo with a human player and midi/cv keyboard.
 * As long as the human is playing, the IdleSwitch output is 'off', but if they go
 * idle for some time period the output is turned on. For example, a patch may plain
 * loud drone when idle, but would turn the drone off or down when the human played
 * and then turn it back on when it stopped. Or maybe it could be used to start an
 * drum fill...
 *
 * The 'Reset idle' input allows this be kind of synced to a clock, beat, or sequence.
 * In the dronevexample above, the drone would then only come back in on a beat.
 *
 * And perhaps most importantly, it can be used to do almost random output and
 * make weird noises.
 */


/* TODO
 *   - is there a 'standard' for communicating lengths of time (like delay time)?
 * - idle start trigger
 * - idle end trigger
 * - switch for output to be high for idle or low for idle
 * - time display widget for timeout length
 * - Fine/Course params fors for timeout
 * - idle timeout countdown display for remaining time before timeout
 *   - gui 'progress' widget?
*/

struct IdleSwitch : Module {
    enum ParamIds {
        TIME_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_SOURCE_INPUT,
        HEARTBEAT_INPUT,
        TIME_INPUT,
        PULSE_INPUT,
        SWITCHED_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        IDLE_GATE_OUTPUT,
        TIME_OUTPUT,
        IDLE_START_OUTPUT,
        IDLE_END_OUTPUT,
        FRAME_COUNT_OUTPUT,
        ON_WHEN_IDLE_OUTPUT,
        OFF_WHEN_IDLE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    int idleTimeoutMS = 140;
    int idleTimeLeftMS = 0;

    SchmittTrigger inputTrigger;

    // FIXME: these names are confusing
    SchmittTrigger heartbeatTrigger;

    // clock mode stuff
    SchmittTrigger pulseTrigger;
    int pulseFrame = 0;
    bool waiting_for_pulse = false;
    bool pulse_mode = false;

    PulseGenerator idleStartPulse;
    PulseGenerator idleEndPulse;

    // FIXME: not really counts
    int frameCount = 0;
    int maxFrameCount = 0;

    float idleGateOutput = 0.0;

    float deltaTime = 0;

    bool is_idle = false;

    IdleSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void IdleSwitch::step() {
    bool pulse_seen = false;
    bool time_exceeded = false;
    pulse_mode = inputs[PULSE_INPUT].active;

    float sampleRate = engineGetSampleRate();

    // Compute the length of our idle time based on the knob + time cv
    // -or-
    // base it one the time since the last clock pulse
    if (pulse_mode) {
        if (inputTrigger.process(inputs[PULSE_INPUT].value)) {
            // keep track of which frame we got a pulse
            // FIXME: without a max time, frameCount can wrap?
            // update pulseFrame to point to current frame count
            pulseFrame = frameCount;

            waiting_for_pulse = true;
            pulse_seen = true;

        }

        deltaTime = fmax(frameCount - pulseFrame, 0) / sampleRate;
       // if we are waiting, maxframeCount is the time since last pulse and increasing
        maxFrameCount = frameCount;

    } else {
        deltaTime = params[TIME_PARAM].value;
        if (inputs[TIME_INPUT].active) {
            deltaTime += clamp(inputs[TIME_INPUT].value, 0.0f, 10.0f);
        }

        // TODO: refactor into submethods if not subclass
        maxFrameCount = (int)ceilf(deltaTime * sampleRate);
    }

    idleTimeoutMS = std::round(deltaTime*1000);

    // debug("is_idle: %d pulse_mode: %d pulse_frame: %d frameCount: %d maxFrameCount: %d ", is_idle, pulse_mode, pulseFrame, frameCount, maxFrameCount);
    // debug("is_idle: %d pulse_mode: %d w_f_pulse: %d pulse_seen: %d pulseFrame: %d frameCount: %d deltaTime: %f",
    //        is_idle, pulse_mode, waiting_for_pulse, pulse_seen, pulseFrame, frameCount, deltaTime);

    if (inputs[HEARTBEAT_INPUT].active &&
          heartbeatTrigger.process(inputs[HEARTBEAT_INPUT].value)) {
            frameCount = 0;
    }

    // time_left_s is always 0 for pulse mode until we predict the future
    float frames_left = fmax(maxFrameCount - frameCount, 0);
    float time_left_s = frames_left / sampleRate;

    // TODO: simplify the start/end/gate on logic... really only a few states to check

    // the start of idle  (not idle -> idle trans)
    if ((frameCount > maxFrameCount) || (waiting_for_pulse && pulse_seen)) {
        time_exceeded = true;
        if (!is_idle) {
            idleStartPulse.trigger(0.01);
        }

    }

    // stay idle once we start until there is an input event
    is_idle = (is_idle || time_exceeded);

    if (is_idle) {
        idleGateOutput = 10.0;
        outputs[ON_WHEN_IDLE_OUTPUT].value = inputs[SWITCHED_INPUT].value;
        outputs[OFF_WHEN_IDLE_OUTPUT].value = 0.0f;

    } else {
        idleGateOutput = 0.0;
        outputs[ON_WHEN_IDLE_OUTPUT].value = 0.0f;
        outputs[OFF_WHEN_IDLE_OUTPUT].value = inputs[SWITCHED_INPUT].value;

        is_idle = false;

        // if we arent idle yet, the idleTimeLeft is changing and we need to update time remaining display
        // update idletimeLeftMS which drives the digit display widget
        idleTimeLeftMS = time_left_s*1000;
    }

    frameCount++;

    if (inputs[INPUT_SOURCE_INPUT].active &&
            inputTrigger.process(inputs[INPUT_SOURCE_INPUT].value)) {

        // only end idle if we are already idle (idle->not idle transition)
        if (is_idle) {
            idleEndPulse.trigger(0.01);
        }

        is_idle = false;

        waiting_for_pulse = false;
        frameCount = 0;
        pulseFrame = 0;
    }

    // once clock input works, could add an output to indicate how long between clock
    // If in pulse mode, deltaTime can be larger than 10s internal, but the max output
    // to "Time output" is 10V. ie, after 10s the "Time Output" stops increasing.
    outputs[TIME_OUTPUT].value = clamp(deltaTime, 0.0f, 10.0f);
    outputs[IDLE_GATE_OUTPUT].value = idleGateOutput;

    outputs[IDLE_START_OUTPUT].value = idleStartPulse.process(1.0/engineGetSampleRate()) ? 10.0 : 0.0;
    outputs[IDLE_END_OUTPUT].value = idleEndPulse.process(1.0/engineGetSampleRate()) ? 10.0 : 0.0;

}


//  From AS DelayPlus.cpp https://github.com/AScustomWorks/AS
struct IdleSwitchMsDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  IdleSwitchMsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  }

  void draw(NVGcontext *vg) override {
    // Background
    // these go to...
    NVGcolor backgroundColor = nvgRGB(0x11, 0x11, 0x11);

    NVGcolor borderColor = nvgRGB(0xff, 0xff, 0xff);

    nvgBeginPath(vg);

    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    nvgStrokeWidth(vg, 3.0);
    nvgStrokeColor(vg, borderColor);

    nvgStroke(vg);

    // text
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;
    to_display << std::right  << std::setw(5) << *value;

    Vec textPos = Vec(0.5f, 19.0f);

    NVGcolor textColor = nvgRGB(0x65, 0xf6, 0x78);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};


struct IdleSwitchWidget : ModuleWidget {
    IdleSwitchWidget(IdleSwitch *module);
};


IdleSwitchWidget::IdleSwitchWidget(IdleSwitch *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/IdleSwitch.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(5, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 20, 365)));

    addInput(Port::create<PJ301MPort>(Vec(37, 20.0), Port::INPUT, module, IdleSwitch::INPUT_SOURCE_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(37, 60.0), Port::INPUT, module, IdleSwitch::HEARTBEAT_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(70, 60.0), Port::INPUT, module, IdleSwitch::PULSE_INPUT));

    // idle time display
    // FIXME: handle large IdleTimeoutMs (> 99999ms) better
    IdleSwitchMsDisplayWidget *idle_time_display = new IdleSwitchMsDisplayWidget();
    idle_time_display->box.pos = Vec(20, 115);
    idle_time_display->box.size = Vec(70, 24);
    idle_time_display->value = &module->idleTimeoutMS;
    addChild(idle_time_display);

    addInput(Port::create<PJ301MPort>(Vec(10, 155.0), Port::INPUT, module, IdleSwitch::TIME_INPUT));
    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(38.86, 150.0), module, IdleSwitch::TIME_PARAM, 0.0, 10.0, 0.25));
    addOutput(Port::create<PJ301MPort>(Vec(80, 155.0), Port::OUTPUT, module, IdleSwitch::TIME_OUTPUT));

    IdleSwitchMsDisplayWidget *time_remaining_display = new IdleSwitchMsDisplayWidget();
    time_remaining_display->box.pos = Vec(20, 225);
    time_remaining_display->box.size = Vec(70, 24);
    time_remaining_display->value = &module->idleTimeLeftMS;
    addChild(time_remaining_display);

    addOutput(Port::create<PJ301MPort>(Vec(10, 263.0), Port::OUTPUT, module, IdleSwitch::IDLE_START_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(47.5, 263.0), Port::OUTPUT, module, IdleSwitch::IDLE_GATE_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(85, 263.0), Port::OUTPUT, module, IdleSwitch::IDLE_END_OUTPUT));

    addInput(Port::create<PJ301MPort>(Vec(10.0f, 315.0f), Port::INPUT, module, IdleSwitch::SWITCHED_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(47.5f, 315.0f), Port::OUTPUT, module, IdleSwitch::ON_WHEN_IDLE_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(85.0f, 315.0f), Port::OUTPUT, module, IdleSwitch::OFF_WHEN_IDLE_OUTPUT));
}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

RACK_PLUGIN_MODEL_INIT(Alikins, IdleSwitch) {
   Model *modelIdleSwitch = Model::create<IdleSwitch, IdleSwitchWidget>(
      "Alikins", "IdleSwitch", "Idle Switch", SWITCH_TAG  , UTILITY_TAG);
   return modelIdleSwitch;
}
