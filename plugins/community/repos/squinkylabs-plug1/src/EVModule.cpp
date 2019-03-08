
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _EV

#include "EvenVCO.h"


/**
 */
struct EVModule : Module
{
public:
    EVModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    EvenVCO<WidgetComposite> vco;
private:
};

void EVModule::onSampleRateChange()
{
}

EVModule::EVModule()
    : Module(vco.NUM_PARAMS,
    vco.NUM_INPUTS,
    vco.NUM_OUTPUTS,
    vco.NUM_LIGHTS),
    vco(this)
{
    onSampleRateChange();
}

void EVModule::step()
{
    vco.step();
}

////////////////////
// module widget
////////////////////

struct EVWidget : ModuleWidget
{
    EVWidget(EVModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void draw(NVGcontext *vg) override;

    void addPWM(EVModule *, float verticalShift);
    void addMiddle(EVModule *, float verticalShift);
    void addOutputs(EVModule *, float verticalShift);

    Label* octaveLabel;
    ParamWidget* octaveKnob;
    int lastOctave = -100;
};

void EVWidget::draw(NVGcontext *vg)
{
    float value = octaveKnob->value;
    int oct = roundf(value);
    if (oct != lastOctave) {
        const char * val = "yy";
        switch (oct) {
            case -5:
                val = "32'";
                break;
            case -4:
                val = "16'";
                break;
            case -3:
                val = "8'";
                break;
            case -2:
                val = "4'";
                break;
            case -1:
                val = "2'";
                break;
            case 0:
                val = "1'";
                break;
            case 1:
                val = "1/2'";
                break;
            case 2:
                val = "1/4'";
                break;
            case 3:
                val = "1/8'";
                break;
            case 4:
                val = "1/16'";
                break;
        }



        octaveLabel->text = val;
    }

    ModuleWidget::draw(vg);
}

void EVWidget::addPWM(EVModule * module, float verticalShift)
{
    addInput(Port::create<PJ301MPort>(Vec(72, 236 + verticalShift),
        Port::INPUT, module, module->vco.PWM_INPUT));

    addParam(ParamWidget::create<Rogan1PBlue>(Vec(16, 212 + verticalShift),
        module, module->vco.PWM_PARAM, -1.0, 1.0, 0.0));

    addLabel(Vec(30, 246 + verticalShift), "pwm");
}

void EVWidget::addMiddle(EVModule * module, float verticalShift)
{
    addParam(ParamWidget::create<Rogan1PBlue>(Vec(73, 125 + verticalShift),
        module, module->vco.TUNE_PARAM, -7.0, 7.0, 0.0));
    addLabel(Vec(69, 164 + verticalShift), "tune");

    addInput(Port::create<PJ301MPort>(Vec(10, 124 + verticalShift),
        Port::INPUT, module, module->vco.PITCH1_INPUT));

    addInput(Port::create<PJ301MPort>(Vec(34, 160 + verticalShift),
        Port::INPUT, module, module->vco.PITCH2_INPUT));
    addLabel(Vec(6, 164 + verticalShift), "cv");
    addInput(Port::create<PJ301MPort>(Vec(62, 194 + verticalShift),
        Port::INPUT, module, module->vco.FM_INPUT));
    addLabel(Vec(84, 200 + verticalShift), "fm");
//	addInput(Port::create<PJ301MPort>(Vec(86, 189), Port::INPUT, module, module->vco.SYNC_INPUT));


}

void EVWidget::addOutputs(EVModule * module, float verticalShift)
{
    const float penultimateRow = 273 + verticalShift;
    const float penultimateLabelRow = penultimateRow + 24;

    addOutput(Port::create<PJ301MPort>(Vec(10, penultimateRow), Port::OUTPUT, module, module->vco.TRI_OUTPUT));
    addLabel(Vec(8, penultimateLabelRow), "tri");

    addOutput(Port::create<PJ301MPort>(Vec(87, penultimateRow), Port::OUTPUT, module, module->vco.SINE_OUTPUT));
    addLabel(Vec(84, penultimateLabelRow), "sin");

    const float bottomRow = 317 + verticalShift;            // 320 -> 317 to make room?
    const float bottomLabelRow = bottomRow + 24;

    addOutput(Port::create<PJ301MPort>(Vec(48, bottomRow), Port::OUTPUT, module, module->vco.EVEN_OUTPUT));
    addLabel(Vec(38, bottomLabelRow), "even");
    addOutput(Port::create<PJ301MPort>(Vec(10, bottomRow), Port::OUTPUT, module, module->vco.SAW_OUTPUT));
    addLabel(Vec(4, bottomLabelRow), "saw");
    addOutput(Port::create<PJ301MPort>(Vec(87, bottomRow), Port::OUTPUT, module, module->vco.SQUARE_OUTPUT));
    addLabel(Vec(83, bottomLabelRow), "sqr");
}


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
EVWidget::EVWidget(EVModule *module) : ModuleWidget(module)
{
    box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    addPWM(module, -10);
    addMiddle(module, -14);
    addOutputs(module, -12);

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(15 * 6, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15 * 6, 365)));

    octaveKnob = ParamWidget::create<Rogan3PSBlue>(Vec(34, 32),
        module, module->vco.OCTAVE_PARAM, -5.0, 4.0, 0.0);

    addParam(octaveKnob);
    addLabel(Vec(20, 88), "octave:");
    //label->fontSize = 16;

    octaveLabel = addLabel(Vec(70, 90), "xx");
 }

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, EV) {
   Model *modelEVModule = Model::create<EVModule,
                                        EVWidget>("Squinky Labs",
                                                  "squinkylabs-evco",
                                                  "EvilVCO", OSCILLATOR_TAG);
   return modelEVModule;
}

#endif

