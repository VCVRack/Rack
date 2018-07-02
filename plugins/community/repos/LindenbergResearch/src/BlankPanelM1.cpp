#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {

struct BlankPanelM1 : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    BlankPanelM1() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}


    void step() override;
};


void BlankPanelM1::step() {
}


/**
 * @brief Blank Panel Mark I
 */
struct BlankPanelWidgetM1 : LRModuleWidget {
    BlankPanelWidgetM1(BlankPanelM1 *module);
};


BlankPanelWidgetM1::BlankPanelWidgetM1(BlankPanelM1 *module) : LRModuleWidget(module) {
    // setPanel(SVG::load(assetPlugin(plugin, "res/BlankPanelM1.svg")));

    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/BlankPanelM1.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, BlankPanelM1) {
   Model *modelBlankPanelM1 = Model::create<BlankPanelM1, BlankPanelWidgetM1>("Lindenberg Research", "BlankPanel 02", "Blank Panel 12TE",
                                                                              BLANK_TAG);
   return modelBlankPanelM1;
}
