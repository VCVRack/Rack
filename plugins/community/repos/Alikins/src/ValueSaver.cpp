#include "dsp/digital.hpp"
#include "alikins.hpp"

#define VALUE_COUNT 4
#define CLOSE_ENOUGH 0.01f

namespace rack_plugin_Alikins {

struct ValueSaver : Module {
    enum ParamIds {
        ENUMS(VALUE_PARAM, VALUE_COUNT),
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(VALUE_INPUT, VALUE_COUNT),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(VALUE_OUTPUT, VALUE_COUNT),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ValueSaver() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    void onReset() override {
    }

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

    float values[VALUE_COUNT] = {0.0f};
    float prevInputs[VALUE_COUNT] = {0.0f};

    bool initialized = false;

    bool changingInputs[VALUE_COUNT] = {};

    SchmittTrigger valueUpTrigger[VALUE_COUNT];
    SchmittTrigger valueDownTrigger[VALUE_COUNT];

};

void ValueSaver::step()
{
    // states:
    //  - active inputs, meaningful current input -> output
    //  - active inputs,
    //  - in active inputs, meaningful 'saved' input -> output
    //  - in active inputs, default/unset value -> output
    //
    for (int i = 0; i < VALUE_COUNT; i++) {
        // Just output the "saved" value if NO ACTIVE input
        if (!inputs[VALUE_INPUT + i].active) {
            outputs[VALUE_OUTPUT + i].value = prevInputs[i];
            continue;
        }

        // ACTIVE INPUTS
        // process(rescale(in, low, high, 0.f, 1.f))
        // if we haven't already figured out this is a useful changing input, check
        if (!changingInputs[i]) {
            float down = rescale(inputs[VALUE_INPUT + i].value, 0.0f, -CLOSE_ENOUGH, 0.f, 1.f);
            float up = rescale(inputs[VALUE_INPUT + i].value, 0.0f, CLOSE_ENOUGH, 0.f, 1.f);

            // TODO: if input is changing from 0.0f
            //       if input is 0.0f but that is changing from prevInput (explicitly sent 0.0f)
            if (valueUpTrigger[i].process(up) || valueDownTrigger[i].process(down)) {
                // debug("value*Trigger[%d] triggered value: %f %f", i, values[i], up);
                changingInputs[i] = true;
            }
        }

        if (!changingInputs[i]) {
            // active input but it is 0.0f, like a midi-1 on startup that hasn't sent any signal yet
            // debug("[%d] ACTIVE but input is ~0.0f, prevInputs[%d]=%f", i, i, prevInputs[i]);
            values[i] = prevInputs[i];
            outputs[VALUE_OUTPUT + i].value = values[i];
        }
        else {
            // input value copied to output value and stored in values[]
            values[i] = inputs[VALUE_INPUT + i].value;
            outputs[VALUE_OUTPUT + i].value = values[i];
            prevInputs[i] = values[i];

            // We are getting meaningful input values (ie, not just 0.0f), we can
            // pay attention to the inputs now
            changingInputs[i] = true;
            continue;
        }
    }
}

json_t* ValueSaver::toJson() {
    json_t *rootJ = json_object();

    json_t *valuesJ = json_array();
    for (int i = 0; i < VALUE_COUNT; i++)
    {
        // debug("toJson current values[%d]: %f", i, values[i]);
        json_t *valueJ = json_real(values[i]);
        json_array_append_new(valuesJ, valueJ);
    }
    json_object_set_new(rootJ, "values", valuesJ);

    return rootJ;
}

void ValueSaver::fromJson(json_t *rootJ) {
    json_t *valuesJ = json_object_get(rootJ, "values");
    float savedInput;

    if (valuesJ) {
        for (int i = 0; i < VALUE_COUNT; i++) {
            json_t *valueJ = json_array_get(valuesJ, i);
            if (valueJ) {
                savedInput = json_number_value(valueJ);
                prevInputs[i] = savedInput;
                values[i] = savedInput;
                changingInputs[i] = false;
            }
        }
    }

    initialized = true;
}

struct LabelTextField : LedDisplayTextField {
    LabelTextField() {
        color = COLOR_CYAN;
        textOffset = Vec(-2.0f, -3.0f);
        multiline = true;
        text = "";
    }
};

struct ValueSaverWidget : ModuleWidget {
    ValueSaverWidget(ValueSaver *module);

    LabelTextField *labelTextFields[VALUE_COUNT];

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};

json_t *ValueSaverWidget::toJson() {
    json_t *rootJ = ModuleWidget::toJson();

    json_t *labelsJ = json_array();
    for (int i = 0; i < VALUE_COUNT; i++) {
        json_t *labelJ  = json_string(labelTextFields[i]->text.c_str());
        json_array_append_new(labelsJ, labelJ);
    }
    json_object_set_new(rootJ, "labels", labelsJ);

	return rootJ;
}

void ValueSaverWidget::fromJson(json_t *rootJ) {
    ModuleWidget::fromJson(rootJ);

    json_t *labelsJ = json_object_get(rootJ, "labels");
    if (labelsJ) {
        for (int i = 0; i < VALUE_COUNT; i++) {
            json_t *labelJ = json_array_get(labelsJ, i);
            if (labelJ) {
                labelTextFields[i]->text = json_string_value(labelJ);
            }
        }
    }
}


ValueSaverWidget::ValueSaverWidget(ValueSaver *module) : ModuleWidget(module) {

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/ValueSaverPanel.svg")));

    float y_baseline = 48.0f;
    float y_pos = y_baseline;

    for (int i = 0; i < VALUE_COUNT; i++) {
        float x_pos = 4.0f;

        addInput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          ValueSaver::VALUE_INPUT + i));

        x_pos += 30.0f;

        addOutput(Port::create<PJ301MPort>(Vec(box.size.x - 30.0f, y_pos),
                                           Port::OUTPUT,
                                           module,
                                           ValueSaver::VALUE_OUTPUT + i));

        y_pos += 28.0f;
        labelTextFields[i] = new LabelTextField();

        labelTextFields[i]->box.pos = (Vec(4.0f, y_pos));
        labelTextFields[i]->box.size = Vec(box.size.x - 8.0f, 38.0f);
		addChild(labelTextFields[i]);

        y_pos += labelTextFields[i]->box.size.y;
        y_pos += 14.0f;
    }

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

RACK_PLUGIN_MODEL_INIT(Alikins, ValueSaver) {
   Model *modelValueSaver = Model::create<ValueSaver, ValueSaverWidget>(
      "Alikins", "ValueSaver", "Value Saver", UTILITY_TAG);
   return modelValueSaver;
}
