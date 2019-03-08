#include "alikins.hpp"

#include "dsp/digital.hpp"
#include "window.hpp"
#include <global_pre.hpp>
#include <global_ui.hpp>

namespace rack_plugin_Alikins {

struct ShiftPedal : Module {
    enum ParamIds {
        LEFT_SHIFT_PARAM,
        RIGHT_SHIFT_PARAM,
        LEFT_CTRL_PARAM,
        RIGHT_CTRL_PARAM,
        LEFT_ALT_PARAM,
        RIGHT_ALT_PARAM,
        LEFT_SUPER_PARAM,
        RIGHT_SUPER_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        LEFT_SHIFT_GATE_OUTPUT,
        RIGHT_SHIFT_GATE_OUTPUT,
        EITHER_SHIFT_GATE_OUTPUT,
        LEFT_CTRL_GATE_OUTPUT,
        RIGHT_CTRL_GATE_OUTPUT,
        EITHER_CTRL_GATE_OUTPUT,
        LEFT_ALT_GATE_OUTPUT,
        RIGHT_ALT_GATE_OUTPUT,
        EITHER_ALT_GATE_OUTPUT,
        LEFT_SUPER_GATE_OUTPUT,
        RIGHT_SUPER_GATE_OUTPUT,
        EITHER_SUPER_GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    ShiftPedal() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    // TODO: should probably setup a pulse generator for the gate outs
};

void ShiftPedal::step() {
    // TODO: should probably setup a pulse generator for the gate outs

    outputs[LEFT_SHIFT_GATE_OUTPUT].value = params[LEFT_SHIFT_PARAM].value;
    outputs[RIGHT_SHIFT_GATE_OUTPUT].value = params[RIGHT_SHIFT_PARAM].value;
    outputs[EITHER_SHIFT_GATE_OUTPUT].value = params[LEFT_SHIFT_PARAM].value +
        params[RIGHT_SHIFT_PARAM].value >= 10.0f ? 10.0f : 0.0f;

    outputs[LEFT_CTRL_GATE_OUTPUT].value = params[LEFT_CTRL_PARAM].value;
    outputs[RIGHT_CTRL_GATE_OUTPUT].value = params[RIGHT_CTRL_PARAM].value;
    outputs[EITHER_CTRL_GATE_OUTPUT].value = params[LEFT_CTRL_PARAM].value +
        params[RIGHT_CTRL_PARAM].value >= 10.0f ? 10.0f : 0.0f;

    outputs[LEFT_ALT_GATE_OUTPUT].value = params[LEFT_ALT_PARAM].value;
    outputs[RIGHT_ALT_GATE_OUTPUT].value = params[RIGHT_ALT_PARAM].value;
    outputs[EITHER_ALT_GATE_OUTPUT].value = params[LEFT_ALT_PARAM].value +
        params[RIGHT_ALT_PARAM].value >= 10.0f ? 10.0f : 0.0f;

    outputs[LEFT_SUPER_GATE_OUTPUT].value = params[LEFT_SUPER_PARAM].value;
    outputs[RIGHT_SUPER_GATE_OUTPUT].value = params[RIGHT_SUPER_PARAM].value;
    outputs[EITHER_SUPER_GATE_OUTPUT].value = params[LEFT_SUPER_PARAM].value +
        params[RIGHT_SUPER_PARAM].value >= 10.0f ? 10.0f : 0.0f;
}

struct ShiftSwitch : SVGSwitch, ToggleSwitch {
    ShiftSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/ShiftIsOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/ShiftIsOn.svg")));
    }
};

struct CtrlSwitch : SVGSwitch, ToggleSwitch {
    CtrlSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/CtrlIsOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/CtrlIsOn.svg")));
    }
};

struct AltSwitch : SVGSwitch, ToggleSwitch {
    AltSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/AltIsOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/AltIsOn.svg")));
    }
};

struct SuperSwitch : SVGSwitch, ToggleSwitch {
    SuperSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/SuperIsOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/SuperIsOn.svg")));
    }
};

struct PurplePort : SVGPort {
	PurplePort() {
		setSVG(SVG::load(assetPlugin(plugin, "res/PurplePort.svg")));
	}
};

struct ShiftPedalWidget : ModuleWidget {
    ShiftPedalWidget(ShiftPedal *module);

    ParamWidget *leftShiftButtonSwitch;
    ParamWidget *rightShiftButtonSwitch;
    ParamWidget *leftCtrlButtonSwitch;
    ParamWidget *rightCtrlButtonSwitch;
    ParamWidget *leftAltButtonSwitch;
    ParamWidget *rightAltButtonSwitch;
    ParamWidget *leftSuperButtonSwitch;
    ParamWidget *rightSuperButtonSwitch;

    void step() override;

};

ShiftPedalWidget::ShiftPedalWidget(ShiftPedal *module) : ModuleWidget(module) {
    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/ShiftPedal.svg")));

    // FIXME: change to #defines
    float buttonWidth = 30.0f;
    float buttonHeight = 55.5f;
    float y_start = 25.0f;
    float y_spacing = 1.5f;
    float y_row_spacing = 6.5f;
    float y_baseline = y_start;
    float port_x_start = 3.0f;
    float middle = box.size.x / 2.0f;

    // first row, shift
    leftShiftButtonSwitch = ParamWidget::create<ShiftSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_SHIFT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftShiftButtonSwitch);

    buttonHeight = leftShiftButtonSwitch->box.size.y;

    Port *leftShiftButtonPort =
        Port::create<PurplePort>(Vec(port_x_start, y_baseline + buttonHeight + y_spacing),
                                Port::OUTPUT,
                                module,
                                ShiftPedal::LEFT_SHIFT_GATE_OUTPUT);

    addOutput(leftShiftButtonPort);

    float portHeight = leftShiftButtonPort->box.size.y;
    float portWidth = leftShiftButtonPort->box.size.x;

    // Add the 'either' port

    Port *eitherShiftPort =
        Port::create<PurplePort>(Vec(middle - (portWidth/2.0f), y_baseline + buttonHeight + y_spacing),
                                Port::OUTPUT,
                                module,
                                ShiftPedal::EITHER_SHIFT_GATE_OUTPUT);
    addOutput(eitherShiftPort);

    rightShiftButtonSwitch = ParamWidget::create<ShiftSwitch>(Vec(buttonWidth, y_start),
                module,
                ShiftPedal::RIGHT_SHIFT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightShiftButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(box.size.x - portWidth - port_x_start, y_start + buttonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_SHIFT_GATE_OUTPUT));

    // next row
    y_baseline = y_baseline + buttonHeight + y_spacing + portHeight + y_row_spacing;

    leftCtrlButtonSwitch = ParamWidget::create<CtrlSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_CTRL_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftCtrlButtonSwitch);

    // update for this row, although ended up making all the buttons the same size for now
    buttonHeight = leftCtrlButtonSwitch->box.size.y;

    addOutput(Port::create<PurplePort>(Vec(port_x_start, y_baseline + buttonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::LEFT_CTRL_GATE_OUTPUT));

    //either
    Port *eitherCtrlPort =
        Port::create<PurplePort>(Vec(middle - (portWidth / 2.0f), y_baseline + buttonHeight + y_spacing),
                                 Port::OUTPUT,
                                 module,
                                 ShiftPedal::EITHER_CTRL_GATE_OUTPUT);
    addOutput(eitherCtrlPort);

    rightCtrlButtonSwitch = ParamWidget::create<CtrlSwitch>(Vec(buttonWidth, y_baseline),
                module,
                ShiftPedal::RIGHT_CTRL_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightCtrlButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(box.size.x - portWidth - port_x_start,
        y_baseline + buttonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_CTRL_GATE_OUTPUT));

    // third row Alt
    y_baseline = y_baseline + buttonHeight + y_spacing + portHeight + y_row_spacing;

    leftAltButtonSwitch = ParamWidget::create<AltSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_ALT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftAltButtonSwitch);

    float altButtonHeight = leftAltButtonSwitch->box.size.y;

    addOutput(Port::create<PurplePort>(Vec(2.0f, y_baseline + altButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::LEFT_ALT_GATE_OUTPUT));

    //either
    Port *eitherAltPort =
        Port::create<PurplePort>(Vec(middle - (portWidth / 2.0f), y_baseline + altButtonHeight + y_spacing),
                                 Port::OUTPUT,
                                 module,
                                 ShiftPedal::EITHER_ALT_GATE_OUTPUT);
    addOutput(eitherAltPort);

    rightAltButtonSwitch = ParamWidget::create<AltSwitch>(Vec(buttonWidth, y_baseline),
                module,
                ShiftPedal::RIGHT_ALT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightAltButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(box.size.x - portWidth - port_x_start, y_baseline + altButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_ALT_GATE_OUTPUT));

    // fourth row, super
    y_baseline = y_baseline + altButtonHeight + y_spacing + portHeight + y_row_spacing;

    leftSuperButtonSwitch = ParamWidget::create<SuperSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_SUPER_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftSuperButtonSwitch);

    float superButtonHeight = leftSuperButtonSwitch->box.size.y;

    addOutput(Port::create<PurplePort>(Vec(2.0f, y_baseline + superButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::LEFT_SUPER_GATE_OUTPUT));

    //either
    Port *eitherSuperPort =
        Port::create<PurplePort>(Vec(middle - (portWidth / 2.0f), y_baseline + superButtonHeight + y_spacing),
                                 Port::OUTPUT,
                                 module,
                                 ShiftPedal::EITHER_SUPER_GATE_OUTPUT);
    addOutput(eitherSuperPort);

    rightSuperButtonSwitch = ParamWidget::create<SuperSwitch>(Vec(buttonWidth, y_baseline),
                module,
                ShiftPedal::RIGHT_SUPER_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightSuperButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(box.size.x - portWidth - port_x_start, y_baseline + superButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_SUPER_GATE_OUTPUT));

    addChild(Widget::create<ScrewSilver>(Vec(0.0, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));

}

void ShiftPedalWidget::step() {

#if 1
    // LGLW (VSVR)
    uint32_t mod = lglw_keyboard_get_modifiers(rack::global_ui->window.lglw);
 
    leftShiftButtonSwitch->setValue((0 != (mod & LGLW_KMOD_LSHIFT)) ? 10.0f : 0.0f);
    rightShiftButtonSwitch->setValue((0 != (mod & LGLW_KMOD_RSHIFT)) ? 10.0f : 0.0f);
 
    leftCtrlButtonSwitch->setValue((0 != (mod & LGLW_KMOD_LCTRL)) ? 10.0f : 0.0f);
    rightCtrlButtonSwitch->setValue((0 != (mod & LGLW_KMOD_RCTRL)) ? 10.0f : 0.0f);
 
    leftAltButtonSwitch->setValue((0 != (mod & LGLW_KMOD_LALT))  ? 10.0f : 0.0f);
    rightAltButtonSwitch->setValue((0 != (mod & LGLW_KMOD_RALT)) ? 10.0f : 0.0f);
 
    leftSuperButtonSwitch->setValue((0 != (mod & LGLW_KMOD_LSUPER)) ? 10.0f : 0.0f);  // unreachable (windows key)
    rightSuperButtonSwitch->setValue((0 != (mod & LGLW_KMOD_RSUPER)) ? 10.0f : 0.0f);  // menu key
#else
   // GLFW (VCV)
    leftShiftButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 10.0f : 0.0f);
    rightShiftButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ? 10.0f : 0.0f);

    leftCtrlButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 10.0f : 0.0f);
    rightCtrlButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ? 10.0f : 0.0f);

    leftAltButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ? 10.0f : 0.0f);
    rightAltButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ? 10.0f : 0.0f);

    leftSuperButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ? 10.0f : 0.0f);
    rightSuperButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS ? 10.0f : 0.0f);
#endif

    ModuleWidget::step();
}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

RACK_PLUGIN_MODEL_INIT(Alikins, ShiftPedal) {
   Model *modelShiftPedal = Model::create<ShiftPedal, ShiftPedalWidget>(
      "Alikins", "ShiftPedal", "Shift Pedal - Gen gates on mod key presses", UTILITY_TAG);
   return modelShiftPedal;
}
