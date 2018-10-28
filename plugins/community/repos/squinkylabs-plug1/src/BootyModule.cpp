#include "global_pre.hpp"
#include "Squinky.hpp"
#include "FrequencyShifter.h"
#include "WidgetComposite.h"
#include "global_ui.hpp"

/**
 * Implementation class for BootyModule
 */
struct BootyModule : Module
{
    BootyModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
    void onSampleRateChange() override;

    FrequencyShifter<WidgetComposite> shifter;
private:
    typedef float T;
public:
    ChoiceButton * rangeChoice;
};

extern float values[];
extern const char* ranges[];

BootyModule::BootyModule() : Module(shifter.NUM_PARAMS, shifter.NUM_INPUTS, shifter.NUM_OUTPUTS, shifter.NUM_LIGHTS),
shifter(this)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    shifter.init();
}

void BootyModule::onSampleRateChange()
{
    T rate = engineGetSampleRate();
    shifter.setSampleRate(rate);
}

json_t *BootyModule::toJson()
{
    json_t *rootJ = json_object();
    const int rg = shifter.freqRange;
    json_object_set_new(rootJ, "range", json_integer(rg));
    return rootJ;
}

void BootyModule::fromJson(json_t *rootJ)
{
    json_t *driverJ = json_object_get(rootJ, "range");
    if (driverJ) {
        const int rg = json_number_value(driverJ);

        // TODO: should we be more robust about float <> int issues?
        //need to tell the control what text to display
        for (int i = 0; i < 5; ++i) {
            if (rg == values[i]) {
                rangeChoice->text = ranges[i];
            }
        }
        shifter.freqRange = rg;
        fflush(stdout);
    }
}

void BootyModule::step()
{
    shifter.step();
}

/***********************************************************************************
 *
 * RangeChoice selector widget
 *
 ***********************************************************************************/

const char* ranges[5] = {
    "5 Hz",
    "50 Hz",
    "500 Hz",
    "5 kHz",
    "exp"
};

float values[5] = {
    5,
    50,
    500,
    5000,
    0
};

struct RangeItem : MenuItem
{
    RangeItem(int index, float * output, ChoiceButton * inParent) :
        rangeIndex(index), rangeOut(output), rangeChoice(inParent)
    {
        text = ranges[index];
    }

    const int rangeIndex;
    float * const rangeOut;
    ChoiceButton* const rangeChoice;

    void onAction(EventAction &e) override
    {
        rangeChoice->text = ranges[rangeIndex];
        *rangeOut = values[rangeIndex];
    }
};

struct RangeChoice : ChoiceButton
{
    RangeChoice(float * out, const Vec& pos, float width) : output(out)
    {
        assert(*output == 5);
        this->text = std::string(ranges[0]);
        this->box.pos = pos;
        this->box.size.x = width;
    }
    float * const output;
    void onAction(EventAction &e) override
    {
        Menu *menu = rack::global_ui->ui.gScene->createMenu();

        menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
        menu->box.size.x = box.size.x;
        {
            menu->addChild(new RangeItem(0, output, this));
            menu->addChild(new RangeItem(1, output, this));
            menu->addChild(new RangeItem(2, output, this));
            menu->addChild(new RangeItem(3, output, this));
            menu->addChild(new RangeItem(4, output, this));
        }
    }
};

////////////////////
// module widget
////////////////////

struct BootyWidget : ModuleWidget
{
    BootyWidget(BootyModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
BootyWidget::BootyWidget(BootyModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/booty_panel.svg")));
        addChild(panel);
    }

    const int leftInputX = 11;
    const int rightInputX = 55;

    const int row0 = 45;
    const int row1 = 102;
    static int row2 = 186;

    // Inputs on Row 0
    addInput(Port::create<PJ301MPort>(Vec(leftInputX, row0), Port::INPUT, module, module->shifter.AUDIO_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(rightInputX, row0), Port::INPUT, module, module->shifter.CV_INPUT));

    // shift Range on row 2
    const float margin = 16;
    float xPos = margin;
    float width = 6 * RACK_GRID_WIDTH - 2 * margin;

    // TODO: why do we need to reach into the module from here? I guess any
    // time UI callbacks need to go bak..
    module->rangeChoice = new RangeChoice(&module->shifter.freqRange, Vec(xPos, row2), width);
    addChild(module->rangeChoice);

    // knob on row 1
    addParam(ParamWidget::create<Rogan3PSBlue>(Vec(18, row1), module, module->shifter.PITCH_PARAM, -5.0, 5.0, 0.0));

    const float row3 = 317.5;

    // Outputs on row 3
    const float leftOutputX = 9.5;
    const float rightOutputX = 55.5;

    addOutput(Port::create<PJ301MPort>(Vec(leftOutputX, row3), Port::OUTPUT, module, module->shifter.SIN_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(rightOutputX, row3), Port::OUTPUT, module, module->shifter.COS_OUTPUT));

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, Booty) {
   Model *modelBootyModule = Model::create<BootyModule, BootyWidget>("Squinky Labs",
                                                                     "squinkylabs-freqshifter",
                                                                     "Booty Shifter: Frequency Shifter", EFFECT_TAG, RING_MODULATOR_TAG);
   return modelBootyModule;
}

