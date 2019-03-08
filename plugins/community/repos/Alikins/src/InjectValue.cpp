#include "alikins.hpp"
#include <math.h>
#include "ui.hpp"
#include "window.hpp"
#include "dsp/digital.hpp"

#include "ParamFloatField.hpp"

namespace rack_plugin_Alikins {

struct InjectValue : Module
{
    enum ParamIds
    {
        INJECT_ENABLED_PARAM,
        INPUT_VOLTAGE_RANGE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        VALUE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        DEBUG1_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    enum InjectEnabled
    {
        OFF,
        WITH_SHIFT,
        ALWAYS
    };

    InjectValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    float param_value = 0.0f;
    float input_param_value = 0.0f;

    InjectEnabled enabled = WITH_SHIFT;
    VoltageRange inputRange = MINUS_PLUS_FIVE;
};

void InjectValue::step()
{
    enabled = (InjectEnabled) clamp((int) round(params[INJECT_ENABLED_PARAM].value), 0, 2);

    inputRange  = (VoltageRange) clamp((int) round(params[INPUT_VOLTAGE_RANGE_PARAM].value), 0, 2);

    if (!inputs[VALUE_INPUT].active) {
        return;
    }

    param_value = inputs[VALUE_INPUT].value;
}

struct InjectValueWidget : ModuleWidget
{
    InjectValueWidget(InjectValue *module);

    void step() override;
    void onChange(EventChange &e) override;

    // TODO: enum/params/ui for input range

    ParamWidget *enableInjectSwitch;
    ParamWidget *inputVoltageSwitch;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;

};

InjectValueWidget::InjectValueWidget(InjectValue *module) : ModuleWidget(module)
{
    setPanel(SVG::load(assetPlugin(plugin, "res/InjectValue.svg")));

    float y_baseline = 45.0f;

    Vec text_field_size = Vec(70.0f, 22.0f);

    float x_pos = 10.0f;

    y_baseline = 38.0f;

    param_value_field = new ParamFloatField(module);
    param_value_field->box.pos = Vec(x_pos, y_baseline);
    param_value_field->box.size = text_field_size;
    param_value_field->setValue(module->param_value);

    addChild(param_value_field);

    y_baseline = 78.0f;
    min_field = new TextField();
    min_field->box.pos = Vec(x_pos, y_baseline);
    min_field->box.size = text_field_size;

    addChild(min_field);

    y_baseline = 118.0f;
    max_field = new TextField();
    max_field->box.pos = Vec(x_pos, y_baseline);
    max_field->box.size = text_field_size;

    addChild(max_field);

    y_baseline = 158.0f;
    default_field = new TextField();
    default_field->box.pos = Vec(x_pos, y_baseline);
    default_field->box.size = text_field_size;

    addChild(default_field);

    y_baseline = 198.0f;
    widget_type_field = new TextField();
    widget_type_field->box.pos = Vec(x_pos, y_baseline);
    widget_type_field->box.size = text_field_size;

    addChild(widget_type_field);

    y_baseline = box.size.y - 128.0f;

    inputVoltageSwitch = ParamWidget::create<CKSSThree>(Vec(5.0f, y_baseline ), module,
        InjectValue::INPUT_VOLTAGE_RANGE_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(inputVoltageSwitch);

    Port *value_in_port = Port::create<PJ301MPort>(
        Vec(60.0f, y_baseline - 2.0),
        Port::INPUT,
        module,
        InjectValue::VALUE_INPUT);

    y_baseline = box.size.y - 65.0f;

    enableInjectSwitch = ParamWidget::create<CKSSThree>(Vec(5, box.size.y - 62.0f), module,
        InjectValue::INJECT_ENABLED_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(enableInjectSwitch);

    inputs.push_back(value_in_port);
    addChild(value_in_port);

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

    // fire off an event to refresh all the widgets
    EventChange e;
    onChange(e);
}

void InjectValueWidget::step() {
    InjectValue *injectValueModule = dynamic_cast<InjectValue *>(module);

    if (!injectValueModule) {
        return;
    }


    if (!RACK_PLUGIN_UI_HOVERED_WIDGET) {
        return;
    }

    // TODO/FIXME: I assume there is a better way to check type?
    ParamWidget *pwidget = dynamic_cast<ParamWidget *>(RACK_PLUGIN_UI_HOVERED_WIDGET);

    if (!pwidget) {
        min_field->setText("");
        max_field->setText("");
        default_field->setText("");
        widget_type_field->setText("unknown");

        ModuleWidget::step();
        return;
    }

    // float input = module->inputs[InjectValue::VALUE_INPUT].value;
    float input_value = injectValueModule->param_value;

    // clamp the input to withing input voltage range before scaling it
    float clamped_input = clamp(input_value,
                                voltage_min[injectValueModule->inputRange],
                                voltage_max[injectValueModule->inputRange]);

    // rescale the input CV to whatever the range of the param widget is
    float scaled_value = rescale(clamped_input,
                                 voltage_min[injectValueModule->inputRange],
                                 voltage_max[injectValueModule->inputRange],
                                 pwidget->minValue, pwidget->maxValue);

    /*
        debug("input_value: %f (in_min: %f, in_max:%f) clamped_in: %f out_min: %f, out_max: %f) scaled_value: %f",
            input_value,
            voltage_min[injectValueModule->inputRange],
            voltage_max[injectValueModule->inputRange],
            clamped_input,
            pwidget->minValue,
            pwidget->maxValue,
            scaled_value);
        */

    // Show the value that will be injected
    // TODO: show the original input value and scaled output?

    if (!injectValueModule->enabled || (injectValueModule->enabled == InjectValue::WITH_SHIFT && !windowIsShiftPressed()))
    {
        return;
    }

    param_value_field->setValue(scaled_value);

    min_field->setText(stringf("%#.4g", pwidget->minValue));
    max_field->setText(stringf("%#.4g", pwidget->maxValue));
    default_field->setText(stringf("%#.4g", pwidget->defaultValue));
    widget_type_field->setText("Param");

    // ParamWidgets are-a QuantityWidget, so change it's value
    // but don't inject values into the switch that turns inject on/off
    if (pwidget != enableInjectSwitch)
    {

        // TODO: would be useful to have a light to indicate when values are being injected
        pwidget->setValue(scaled_value);

        // force a step of the param widget to get it to 'animate'
        pwidget->step();

    }

    ModuleWidget::step();
}

void InjectValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

RACK_PLUGIN_MODEL_INIT(Alikins, InjectValue) {
   Model *modelInjectValue = Model::create<InjectValue, InjectValueWidget>(
      "Alikins", "InjectValue", "Inject Value - inject value into param under cursor", UTILITY_TAG, CONTROLLER_TAG);
   return modelInjectValue;
}
