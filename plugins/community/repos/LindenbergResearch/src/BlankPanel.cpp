#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {
using namespace lrt;

struct BlankPanel : Module {
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


    BlankPanel() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}


    void step() override;
};


void BlankPanel::step() {

}


/**
 * @brief Blank Panel with Logo
 */
struct BlankPanelWidget : LRModuleWidget {
    BlankPanelWidget(BlankPanel *module);
};


BlankPanelWidget::BlankPanelWidget(BlankPanel *module) : LRModuleWidget(module) {
    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/BlankPanel.svg")));
    addChild(panel);

    box.size = panel->box.size;

    panel->setInner(nvgRGBAf(1.4f * .369f, 1.4f * 0.357f, 1.5f * 0.3333f, 0.27f));
    panel->setOuter(nvgRGBAf(0.f, 0.f, 0.f, 0.15f));
    float speed = 0.004;

    addChild(SVGRotator::create(Vec(140.5, 65), SVG::load(assetPlugin(plugin, "res/CogBig.svg")), speed));
    addChild(SVGRotator::create(Vec(120, 96.7), SVG::load(assetPlugin(plugin, "res/CogSmall.svg")), -speed * 1.6));


    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, BlankPanel) {
Model *modelBlankPanel = Model::create<BlankPanel, BlankPanelWidget>("Lindenberg Research", "BlankPanel 01", "Blank: Logo", BLANK_TAG);
   return modelBlankPanel;
}
