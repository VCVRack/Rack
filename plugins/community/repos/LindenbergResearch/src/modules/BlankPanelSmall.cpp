#include "../dsp/DSPMath.hpp"
#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {

using namespace rack;
using namespace lrt;


struct BlankPanelSmall : LRModule {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        M1_INPUT,
        M2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        M1_OUTPUT,
        M2_OUTPUT,
        M3_OUTPUT,
        M4_OUTPUT,
        M5_OUTPUT,
        M6_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    BlankPanelSmall() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    LRIOPortCV *ioports[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    bool multiple = false;

    void step() override;
    void createPorts();


    void showPorts() {
        /* set all to visible */
        for (int i = 0; i < 8; i++) {
            ioports[i]->visible = true;
        }
    }


    void hidePorts() {
        /* set all to invisible */
        for (int i = 0; i < 8; i++) {
            ioports[i]->visible = false;
        }
    }


    void onReset() override {
        multiple = false;
    }

};


void BlankPanelSmall::step() {

    for (int i = 0; i < 8; i++) {
        if (ioports[i] == nullptr) return;
    }

    if (multiple) {
        if (!ioports[0]->visible) {
            showPorts();
        }

        if (inputs[M1_INPUT].active) {
            float sig = inputs[M1_INPUT].value;
            outputs[M1_OUTPUT].value = sig;
            outputs[M2_OUTPUT].value = sig;
            outputs[M3_OUTPUT].value = sig;
        }


        if (inputs[M2_INPUT].active) {
            float sig = inputs[M2_INPUT].value;
            outputs[M4_OUTPUT].value = sig;
            outputs[M5_OUTPUT].value = sig;
            outputs[M6_OUTPUT].value = sig;
        }
    } else {
        if (ioports[0]->visible) {
            hidePorts();
        }
    }
}


/**
 * @brief Blank Panel Small
 */
struct BlankPanelWidgetSmall : LRModuleWidget {
    BlankPanelWidgetSmall(BlankPanelSmall *module);
    void appendContextMenu(Menu *menu) override;


    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};


void BlankPanelSmall::createPorts() {
    /* INPUTS */
    ioports[0] = Port::create<LRIOPortCV>(Vec(16.5, 19.5), Port::INPUT, this, BlankPanelSmall::M1_INPUT);
    ioports[1] = Port::create<LRIOPortCV>(Vec(16.5, 228.5), Port::INPUT, this, BlankPanelSmall::M2_INPUT);

    /* OUTPUTS */
    ioports[2] = Port::create<LRIOPortCV>(Vec(16.5, 53.5), Port::OUTPUT, this, BlankPanelSmall::M1_OUTPUT);
    ioports[3] = Port::create<LRIOPortCV>(Vec(16.5, 87.5), Port::OUTPUT, this, BlankPanelSmall::M2_OUTPUT);
    ioports[4] = Port::create<LRIOPortCV>(Vec(16.5, 120.5), Port::OUTPUT, this, BlankPanelSmall::M3_OUTPUT);
    ioports[5] = Port::create<LRIOPortCV>(Vec(16.5, 262.5), Port::OUTPUT, this, BlankPanelSmall::M4_OUTPUT);
    ioports[6] = Port::create<LRIOPortCV>(Vec(16.5, 296.5), Port::OUTPUT, this, BlankPanelSmall::M5_OUTPUT);
    ioports[7] = Port::create<LRIOPortCV>(Vec(16.5, 329.5), Port::OUTPUT, this, BlankPanelSmall::M6_OUTPUT);

    hidePorts();
}


BlankPanelWidgetSmall::BlankPanelWidgetSmall(BlankPanelSmall *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/BlankPanelSmall.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/BlankPanelSmallLight.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/BlankPanelSmallAged.svg")));

    panel->init();
    addChild(panel);
    box.size = panel->box.size;

    module->createPorts();

    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(23.4, 1)));
    addChild(Widget::create<ScrewLight>(Vec(23.4, 366)));
    // ***** SCREWS **********


    // ***** IO-PORTS **********
    addInput(module->ioports[0]);
    addInput(module->ioports[1]);

    addOutput(module->ioports[2]);
    addOutput(module->ioports[3]);
    addOutput(module->ioports[4]);
    addOutput(module->ioports[5]);
    addOutput(module->ioports[6]);
    addOutput(module->ioports[7]);
    // ***** IO-PORTS **********
}


struct BlankPanelMultiple : MenuItem {
    BlankPanelSmall *blankPanelSmall;


    void onAction(EventAction &e) override {
        if (blankPanelSmall->multiple) {
            for (int i = 0; i < 2; i++) {
                if (blankPanelSmall->inputs[i].active) {
                    blankPanelSmall->multiple = true;
                    return;
                }
            }

            for (int i = 0; i < 6; i++) {
                if (blankPanelSmall->outputs[i].active) {
                    blankPanelSmall->multiple = true;
                    return;
                }

                blankPanelSmall->multiple = false;
            }
        } else {
            blankPanelSmall->multiple = true;
        }
    }


    void step() override {
        rightText = CHECKMARK(blankPanelSmall->multiple);
    }
};


void BlankPanelWidgetSmall::appendContextMenu(Menu *menu) {
    menu->addChild(MenuEntry::create());

    BlankPanelSmall *blankPanelSmall = dynamic_cast<BlankPanelSmall *>(module);
    assert(blankPanelSmall);

    BlankPanelMultiple *mergeItem = MenuItem::create<BlankPanelMultiple>("Dual Multiple");
    mergeItem->blankPanelSmall = blankPanelSmall;
    menu->addChild(mergeItem);
}


json_t *BlankPanelWidgetSmall::toJson() {
    json_t *rootJ = LRModuleWidget::toJson();

    BlankPanelSmall *blankPanelSmall = dynamic_cast<BlankPanelSmall *>(module);

    json_object_set_new(rootJ, "multiple", json_boolean(blankPanelSmall->multiple));
    return rootJ;
}


void BlankPanelWidgetSmall::fromJson(json_t *rootJ) {
    BlankPanelSmall *blankPanelSmall = dynamic_cast<BlankPanelSmall *>(module);

    LRModuleWidget::fromJson(rootJ);

    json_t *multJ = json_object_get(rootJ, "multiple");

    if (multJ)
        blankPanelSmall->multiple = json_boolean_value(multJ);
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, BlankPanelSmall) {
   Model *modelBlankPanelSmall = Model::create<BlankPanelSmall, BlankPanelWidgetSmall>(
      "Lindenberg Research",
      "BlankPanel Small",
      "Blank: Small / Multiple",
      BLANK_TAG, MULTIPLE_TAG);
   return modelBlankPanelSmall;
}
