#include "dsp/digital.hpp"
#include "util/math.hpp"
#include "qwelk.hpp"


#define CHANNELS 8

namespace rack_plugin_Qwelk {

struct ModuleAutomaton : Module {
    enum ParamIds {
        PARAM_SCAN,
        PARAM_STEP,
        PARAM_CELL,
        NUM_PARAMS = PARAM_CELL + CHANNELS * 2
    };
    enum InputIds {
        INPUT_SCAN,
        INPUT_STEP,
        INPUT_RULE,
        NUM_INPUTS = INPUT_RULE + 8
    };
    enum OutputIds {
        OUTPUT_COUNT,
        OUTPUT_NUMBER,
        OUTPUT_CELL,
        NUM_OUTPUTS = OUTPUT_CELL + CHANNELS
    };
    enum LightIds {
        LIGHT_POS_SCAN,
        LIGHT_NEG_SCAN,
        LIGHT_STEP,
        LIGHT_MUTE,
        NUM_LIGHTS = LIGHT_MUTE + CHANNELS * 2
    };

    int             fun = 0;
    int             scan = 1;
    int             scan_sign = 0;
    SchmittTrigger  trig_step_input;
    SchmittTrigger  trig_step_manual;
    SchmittTrigger  trig_scan_manual;
    SchmittTrigger  trig_scan_input;
    SchmittTrigger  trig_cells[CHANNELS*2];
    int             states[CHANNELS*2] {};
    
    const float     output_volt = 5.0;
    const float     output_volt_uni = output_volt * 2;

    
    ModuleAutomaton() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    json_t *toJson() override {
        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "scan", json_integer(scan));
        json_object_set_new(rootJ, "fun", json_integer(fun));

        json_t *statesJ = json_array();
        for (int i = 0; i < CHANNELS*2; i++) {
            json_t *stateJ = json_integer(states[i]);
            json_array_append_new(statesJ, stateJ);
        }
        json_object_set_new(rootJ, "states", statesJ);

        return rootJ;
    }

    void fromJson(json_t *rootJ) override {
        json_t *scanJ = json_object_get(rootJ, "scan");
        if (scanJ)
            scan = json_integer_value(scanJ);

        json_t *funJ = json_object_get(rootJ, "fun");
        if (funJ)
            fun = json_integer_value(funJ);

        // gates
        json_t *statesJ = json_object_get(rootJ, "states");
        if (statesJ) {
            for (int i = 0; i < 8; i++) {
                json_t *gateJ = json_array_get(statesJ, i);
                if (gateJ)
                    states[i] = json_integer_value(gateJ);
            }
        }
    }

    void reset() override {
        fun = 0;
        scan = 1;
        for (int i = 0; i < CHANNELS * 2; i++)
            states[i] = 0;
    }

    void randomize() override {
        scan = (randomUniform() > 0.5) ? 1 : -1;
        for (int i = 0; i < CHANNELS; i++)
            states[i] = (randomUniform() > 0.5);
    }
};

void ModuleAutomaton::step()
{
    int nextstep = 0;
    if (trig_step_manual.process(params[PARAM_STEP].value)
        || trig_step_input.process(inputs[INPUT_STEP].value))
        nextstep = 1;

    // determine scan direction
    int scan_input_sign = (int)sgn(inputs[INPUT_SCAN].normalize(scan));
    if (scan_input_sign != scan_sign) 
        scan = scan_sign = scan_input_sign;
    // manual tinkering with step?
    if (trig_scan_manual.process(params[PARAM_SCAN].value))
        scan *= -1;
    
    if (nextstep) {
        int rule = 0;
        // read rule from inputs
        for (int i = 0; i < CHANNELS; ++i)
            if (inputs[INPUT_RULE + i].active && inputs[INPUT_RULE + i].value > 0.0)
                rule |= 1 << i;
        // copy prev state to output cells
        for (int i = 0; i < CHANNELS; ++i)
            states[CHANNELS + i] = states[i];
        // determine the next gen
        for (int i = 0; i < CHANNELS; ++i) {
            int sum = 0;
            int tl  = i == 0 ? CHANNELS - 1 : i - 1;
            int tm  = i;
            int tr  = i < CHANNELS - 1 ? i + (1 - fun) : 0;
            sum |= states[CHANNELS + tr] ? (1 << 0) : 0;
            sum |= states[CHANNELS + tm] ? (1 << 1) : 0;
            sum |= states[CHANNELS + tl] ? (1 << 2) : 0;
            states[i] = (rule & (1 << sum)) != 0 ? 1 : 0;
        }
    }

    // handle manual tinkering with the state
    for (int i = 0; i < CHANNELS * 2; ++i)
        if (trig_cells[i].process(params[PARAM_CELL + i].value))
            states[i] ^= 1;
    
    int count = 0, number = 0;
    for (int i = 0; i < CHANNELS; ++i) {
        count += states[i + CHANNELS];
        if (scan >= 0)
            number |= ((1 << i) * states[CHANNELS + i]);
        else
            number |= ((1 << (CHANNELS - 1 - i)) * states[CHANNELS + i]);
    }

    // individual gate output
    for (int i = 0; i < CHANNELS; ++i)
        outputs[OUTPUT_CELL + i].value = states[i + CHANNELS] ? output_volt : 0.0;
    // number of LIVE cells
    outputs[OUTPUT_COUNT].value = ((float)count / (float)CHANNELS) * output_volt_uni;
    // the binary number LIVE cells represent 
    outputs[OUTPUT_NUMBER].value = ((float)number / (float)((1 << CHANNELS) - 1)) * output_volt_uni;

    // indicate step direction
    lights[LIGHT_POS_SCAN].setBrightness(scan < 0 ? 0.0 : 0.9);
    lights[LIGHT_NEG_SCAN].setBrightness(scan < 0 ? 0.9 : 0.0);
    // indicate next generation
    lights[LIGHT_STEP].setBrightness(trig_step_manual.isHigh() || trig_step_input.isHigh() ? 0.9 : 0.0);
    // blink according to state
    for (int i = 0; i < CHANNELS * 2; ++i)
        lights[LIGHT_MUTE + i].setBrightness(states[i] ? 0.9 : 0.0);
}



template <typename _BASE>
struct MuteLight : _BASE {
    MuteLight()
    {
        this->box.size = mm2px(Vec(6, 6));
    }
};

struct WidgetAutomaton : ModuleWidget {
    WidgetAutomaton(ModuleAutomaton *module);
    Menu *createContextMenu() override;
};

WidgetAutomaton::WidgetAutomaton(ModuleAutomaton *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Automaton.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
 
    const float ypad = 27.5;
    const float tlpy = 1.75;
    const float lghx = box.size.x / 2.0;
    const float tlpx = 2.25;
    const float dist = 25;
    
    float ytop = 55;

    addInput(Port::create<PJ301MPort>(               Vec(lghx - dist * 2      , ytop - ypad         ), Port::INPUT, module, ModuleAutomaton::INPUT_SCAN));
    addParam(ParamWidget::create<LEDBezel>(                 Vec(lghx + dist          , ytop - ypad         ), module, ModuleAutomaton::PARAM_SCAN, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MuteLight<GreenRedLight>>( Vec(lghx + dist + tlpx   , ytop - ypad + tlpy  ), module, ModuleAutomaton::LIGHT_POS_SCAN));

    ytop += ypad;
    
    addInput(Port::create<PJ301MPort>(           Vec(lghx - dist * 2     , ytop - ypad         ), Port::INPUT, module, ModuleAutomaton::INPUT_STEP));
    addParam(ParamWidget::create<LEDBezel>(             Vec(lghx + dist         , ytop - ypad         ), module, ModuleAutomaton::PARAM_STEP, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(lghx + dist + tlpx  , ytop - ypad + tlpy  ), module, ModuleAutomaton::LIGHT_STEP));
    
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(Port::create<PJ301MPort>(           Vec(lghx - dist * 2     , ytop + ypad * i       ), Port::INPUT, module, ModuleAutomaton::INPUT_RULE + i));
        addParam(ParamWidget::create<LEDBezel>(             Vec(lghx - dist         , ytop + ypad * i       ), module, ModuleAutomaton::PARAM_CELL + i, 0.0, 1.0, 0.0));
        addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(lghx - dist + tlpx  , ytop + ypad * i + tlpy), module, ModuleAutomaton::LIGHT_MUTE + i));
        addParam(ParamWidget::create<LEDBezel>(             Vec(lghx                , ytop + ypad * i       ), module, ModuleAutomaton::PARAM_CELL + CHANNELS + i, 0.0, 1.0, 0.0));
        addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(lghx + tlpx         , ytop + ypad * i + tlpy), module, ModuleAutomaton::LIGHT_MUTE + CHANNELS + i));
        addOutput(Port::create<PJ301MPort>(         Vec(lghx + dist         , ytop + ypad * i       ), Port::OUTPUT, module, ModuleAutomaton::OUTPUT_CELL + i));
    }
    
    const float output_y = ytop + ypad * CHANNELS;
    addOutput(Port::create<PJ301MPort>(Vec(lghx + dist, output_y        ), Port::OUTPUT, module, ModuleAutomaton::OUTPUT_NUMBER));
    addOutput(Port::create<PJ301MPort>(Vec(lghx + dist, output_y + ypad ), Port::OUTPUT, module, ModuleAutomaton::OUTPUT_COUNT));
}



struct MenuItemFun : MenuItem {
    ModuleAutomaton *automaton;
    void onAction(EventAction &e) override
    {
        automaton->fun ^= 1;
    }
    void step () override
    {
        rightText = (automaton->fun) ? "✔" : "";
    }
};

Menu *WidgetAutomaton::createContextMenu()
{
    Menu *menu = ModuleWidget::createContextMenu();

    MenuLabel *spacer = new MenuLabel();
    menu->addChild(spacer);

    ModuleAutomaton *automaton = dynamic_cast<ModuleAutomaton *>(module);
    assert(automaton);

    MenuItemFun *item = new MenuItemFun();
    item->text = "FUN";
    item->automaton = automaton;
    menu->addChild(item);

    return menu;
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Automaton) {
   Model *modelAutomaton = Model::create<ModuleAutomaton, WidgetAutomaton>(
      TOSTRING(SLUG), "Automaton", "Automaton", SEQUENCER_TAG);
   return modelAutomaton;
}
