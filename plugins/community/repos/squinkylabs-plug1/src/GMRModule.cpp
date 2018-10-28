
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _GMR
#include "GMR.h"


/**
 */
struct GMRModule : Module
{
public:
    GMRModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    GMR<WidgetComposite> gmr;
private:
};

void GMRModule::onSampleRateChange()
{
    float rate = engineGetSampleRate();
    gmr.setSampleRate(rate);
}

GMRModule::GMRModule()
    : Module(gmr.NUM_PARAMS,
    gmr.NUM_INPUTS,
    gmr.NUM_OUTPUTS,
    gmr.NUM_LIGHTS),
    gmr(this)
{
    onSampleRateChange();
    gmr.init();
}

void GMRModule::step()
{
    gmr.step();
}

////////////////////
// module widget
////////////////////

struct GMRWidget : ModuleWidget
{
    GMRWidget(GMRModule *);

    void addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
    }
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
GMRWidget::GMRWidget(GMRModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    addInput(Port::create<PJ301MPort>(
        Vec(40, 200), Port::INPUT, module, module->gmr.CLOCK_INPUT));
    addOutput(Port::create<PJ301MPort>(
        Vec(40, 300), Port::OUTPUT, module, module->gmr.TRIGGER_OUTPUT));


    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, GMR) {
   Model *modelGMRModule = Model::create<GMRModule,
                                         GMRWidget>("Squinky Labs",
                                                    "squinkylabs-GMR",
                                                    "GMR", EFFECT_TAG, LFO_TAG);
   return modelGMRModule;
}
#endif

