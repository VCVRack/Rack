#include "../dsp/DSPMath.hpp"
#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {

using namespace rack;
using namespace lrt;


struct BlankPanelWood : LRModule {
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


    BlankPanelWood() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void updateComponents();

    SVGWidget *patina, *logoStamp;
    ScrewDarkB *screw1, *screw2;
    LRPanel *panel;

    bool aged = true;
    bool screws = true;
    bool logo = true;

    void step() override;
    void randomize() override;


    json_t *toJson() override {
        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "AGED", json_boolean(aged));
        json_object_set_new(rootJ, "screws", json_boolean(screws));
        json_object_set_new(rootJ, "logo", json_boolean(logo));
        return rootJ;
    }


    void fromJson(json_t *rootJ) override {
        LRModule::fromJson(rootJ);

        json_t *agedJ = json_object_get(rootJ, "AGED");
        if (agedJ)
            aged = json_boolean_value(agedJ);

        json_t *screwsJ = json_object_get(rootJ, "screws");
        if (screwsJ)
            screws = json_boolean_value(screwsJ);

        json_t *logoJ = json_object_get(rootJ, "logo");
        if (logoJ)
            logo = json_boolean_value(logoJ);

        updateComponents();
    }
};


void BlankPanelWood::step() {
}


void BlankPanelWood::updateComponents() {
    //randomize();
    screw1->visible = screws;
    screw2->visible = screws;

    logoStamp->visible = logo;

    patina->visible = aged;

    panel->dirty = true;
    //panel->dirty = true;
}


void BlankPanelWood::randomize() {
    Module::randomize();
    patina->box.pos = Vec(-randomUniform() * 1000, -randomUniform() * 200);
    //panel->dirty = true;
}


struct BlankPanelWidgetWood : LRModuleWidget {
    BlankPanelWidgetWood(BlankPanelWood *module);

    void appendContextMenu(Menu *menu) override;
};


BlankPanelWidgetWood::BlankPanelWidgetWood(BlankPanelWood *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/WoodLeftTop.svg")));
    // panel->addSVGVariant(SVG::load(assetPlugin(plugin, "res/panels/WoodLeftTop.svg")));
    // panel->addSVGVariant(SVG::load(assetPlugin(plugin, "res/panels/WoodLeftTop.svg")));

    noVariants = true;
    gestalt = LRGestalt::DARK;
    patina = false;
    gradient = false;

    panel->init();
    addChild(panel);

    module->panel = panel;

    box.size = panel->box.size;

    auto gradientDark = new LRGradientWidget(box.size, nvgRGBAf(1.4f * .369f, 1.4f * 0.357f, 1.4f * 0.3333f, 0.05f),
                                             nvgRGBAf(0.f, 0.f, 0.f, 0.15f), Vec(-10, 10));
    gradientDark->visible = true;

    panel->addChild(gradientDark);

    module->patina = new SVGWidget();
    module->patina->setSVG(SVG::load(assetPlugin(plugin, "res/panels/WoodPatina.svg")));
    panel->addChild(module->patina);

    module->logoStamp = new SVGWidget();
    module->logoStamp->setSVG(SVG::load(assetPlugin(plugin, "res/elements/LogoSmallPlate.svg")));
    module->logoStamp->box.pos = Vec(8.5, 348.8);
    addChild(module->logoStamp);

    module->randomize();


    // ***** SCREWS **********
    module->screw1 = Widget::create<ScrewDarkB>(Vec(23, 6));
    addChild(module->screw1);

    module->screw2 = Widget::create<ScrewDarkB>(Vec(23, box.size.y - 20));
    addChild(module->screw2);
    // ***** SCREWS **********
}


struct BlankPanelWoodAged : MenuItem {
    BlankPanelWood *blankPanelWood;


    void onAction(EventAction &e) override {
        if (blankPanelWood->aged) {
            blankPanelWood->aged = false;
        } else {
            blankPanelWood->aged = true;
        }

        blankPanelWood->updateComponents();
    }


    void step() override {
        rightText = CHECKMARK(blankPanelWood->aged);
    }
};


struct BlankPanelWoodScrews : MenuItem {
    BlankPanelWood *blankPanelWood;


    void onAction(EventAction &e) override {
        if (blankPanelWood->screws) {
            blankPanelWood->screws = false;
        } else {
            blankPanelWood->screws = true;
        }

        blankPanelWood->updateComponents();
    }


    void step() override {
        rightText = CHECKMARK(blankPanelWood->screws);
    }
};


struct BlankPanelWoodLogo : MenuItem {
    BlankPanelWood *blankPanelWood;


    void onAction(EventAction &e) override {
        if (blankPanelWood->logo) {
            blankPanelWood->logo = false;
        } else {
            blankPanelWood->logo = true;
        }

        blankPanelWood->updateComponents();
    }


    void step() override {
        rightText = CHECKMARK(blankPanelWood->logo);
    }
};


void BlankPanelWidgetWood::appendContextMenu(Menu *menu) {
    menu->addChild(MenuEntry::create());

    BlankPanelWood *blankPanelWood = dynamic_cast<BlankPanelWood *>(module);
    assert(blankPanelWood);

    BlankPanelWoodAged *mergeItemAged = MenuItem::create<BlankPanelWoodAged>("Use AGED look");
    mergeItemAged->blankPanelWood = blankPanelWood;
    menu->addChild(mergeItemAged);

    BlankPanelWoodScrews *mergeItemScrews = MenuItem::create<BlankPanelWoodScrews>("Show Screws");
    mergeItemScrews->blankPanelWood = blankPanelWood;
    menu->addChild(mergeItemScrews);

    BlankPanelWoodLogo *mergeItemLogo = MenuItem::create<BlankPanelWoodLogo>("Show Logo Plate");
    mergeItemLogo->blankPanelWood = blankPanelWood;
    menu->addChild(mergeItemLogo);
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, BlankPanelWood) {
   Model *modelBlankPanelWood = Model::create<BlankPanelWood, BlankPanelWidgetWood>(
      "Lindenberg Research",
      "BlankPanel Wood",
      "Blank: Wood ",
      BLANK_TAG);
   return modelBlankPanelWood;
}
