#include "dsp/digital.hpp"
#include "ui.hpp"

#include "alikins.hpp"

namespace rack_plugin_Alikins {

struct Reference : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        MINUS_TEN_OUTPUT,
        MINUS_FIVE_OUTPUT,
        MINUS_ONE_OUTPUT,
        ZERO_OUTPUT,
        PLUS_ONE_OUTPUT,
        PLUS_FIVE_OUTPUT,
        PLUS_TEN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    Reference() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    void onReset() override {
    }

};

void Reference::step() {
    outputs[MINUS_TEN_OUTPUT].value = -10.0f;
    outputs[MINUS_FIVE_OUTPUT].value = -5.0f;
    outputs[MINUS_ONE_OUTPUT].value = -1.0f;

    outputs[ZERO_OUTPUT].value = 0.0f;

    outputs[PLUS_ONE_OUTPUT].value = 1.0f;
    outputs[PLUS_FIVE_OUTPUT].value = 5.0f;
    outputs[PLUS_TEN_OUTPUT].value = 10.0f;

}

struct ReferenceWidget : ModuleWidget {
    ReferenceWidget(Reference *module);
};

ReferenceWidget::ReferenceWidget(Reference *module) : ModuleWidget(module) {

    // box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/Reference.svg")));

    float y_pos = 18.0f;

    float x_pos = 2.0f;
    float y_offset = 50.0f;

    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                       Port::OUTPUT,
                                       module,
                                       Reference::PLUS_TEN_OUTPUT));

    y_pos += y_offset;
    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                       Port::OUTPUT,
                                       module,
                                       Reference::PLUS_FIVE_OUTPUT));

    y_pos += y_offset;
    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                       Port::OUTPUT,
                                       module,
                                       Reference::PLUS_ONE_OUTPUT));

    y_pos += y_offset;
    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                       Port::OUTPUT,
                                       module,
                                       Reference::ZERO_OUTPUT));

    y_pos += y_offset;
    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                       Port::OUTPUT,
                                       module,
                                       Reference::MINUS_ONE_OUTPUT));

    y_pos += y_offset;
    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                       Port::OUTPUT,
                                       module,
                                       Reference::MINUS_FIVE_OUTPUT));

    y_pos += y_offset;
    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                       Port::OUTPUT,
                                       module,
                                       Reference::MINUS_TEN_OUTPUT));


    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

RACK_PLUGIN_MODEL_INIT(Alikins, Reference) {
   Model *modelReference = Model::create<Reference, ReferenceWidget>(
      "Alikins", "Reference", "Reference Voltages", UTILITY_TAG);
   return modelReference;
}
