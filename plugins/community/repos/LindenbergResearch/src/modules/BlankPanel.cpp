#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {
using namespace lrt;


struct BlankPanel : LRModule {
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


    BlankPanel() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


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
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/BlankPanel.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/BlankPanelLight.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/BlankPanelLight.svg")));

    gestalt = LRGestalt::AGED;
    patina = true;
    gradient = true;

    noVariants = true;

    panel->init();
    addChild(panel);
    box.size = panel->box.size;

    panel->patinaWidgetClassic->strength = .5f;
    panel->patinaWidgetWhite->strength = .5f;

    panel->gradients[LIGHT]->setInnerColor(nvgRGBAf(0.5, 0.5, 0.f, 0.1f));
    panel->gradients[LIGHT]->setOuterColor(nvgRGBAf(0.f, 0.f, 0.f, 0.73f));

    float speed = 0.007;

    addChild(SVGRotator::create(Vec(105.5, 55), SVG::load(assetPlugin(plugin, "res/elements/CogBig.svg")), speed, 0.7, 0.4));
    addChild(SVGRotator::create(Vec(139, 43.7), SVG::load(assetPlugin(plugin, "res/elements/CogMiddle.svg")), speed * 1.9f, 0.7, 0.4));
    addChild(SVGRotator::create(Vec(120, 40), SVG::load(assetPlugin(plugin, "res/elements/CogSmall.svg")), -speed * 1.3f, 0.7, 0.4));


    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, BlankPanel) {
Model *modelBlankPanel = Model::create<BlankPanel, BlankPanelWidget>("Lindenberg Research", "BlankPanel 01", "Blank: Logo", BLANK_TAG);
   return modelBlankPanel;
}
