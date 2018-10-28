
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#include "Blank.h"


/**
 */
struct BlankModule : Module
{
public:
    BlankModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    Blank<WidgetComposite> blank;
private:

};

void BlankModule::onSampleRateChange()
{
}

BlankModule::BlankModule()
    : Module(blank.NUM_PARAMS,
    blank.NUM_INPUTS,
    blank.NUM_OUTPUTS,
    blank.NUM_LIGHTS),
    blank(this)
{
    onSampleRateChange();
    blank.init();
}

void BlankModule::step()
{
    blank.step();
}

////////////////////
// module widget
////////////////////

struct BlankWidget : ModuleWidget
{
    BlankWidget(BlankModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
BlankWidget::BlankWidget(BlankModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }


    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, Blank) {
   Model *modelBlankModule = Model::create<BlankModule,
                                           BlankWidget>("Squinky Labs",
                                                        "squinkylabs-blank",
                                                        "-- Blank --", RANDOM_TAG);
   return modelBlankModule;
}
