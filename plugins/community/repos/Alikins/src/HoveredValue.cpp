#include <math.h>

#include "alikins.hpp"
#include "ParamFloatField.hpp"

#include "ui.hpp"
#include "window.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_Alikins {

/* Notes:

Most of this is implemented in HoveredValueWidget.step(). Each
time the gui thread steps the scene graph, .step() is checking
if gHoveredWidget is a ParamWidget or Port. It is more or less
polling.

Is there any way to get an event when any ParamWidget or Port is
hovered? (onMouseMove or onMouseEnter? but for other widgets). May
be better implemented as responding to hypothetical onHoverEnter
or onHoverLeave instead of polling.

*/

/* This module was inspired by the question and discussion at
    https://www.facebook.com/groups/vcvrack/permalink/286752278651590/

    "Hi Folks, Just wondering, is it possible to see control parameter values in VCVRack?"
*/
struct HoveredValue : Module {
    enum ParamIds {
        HOVERED_PARAM_VALUE_PARAM,
        HOVER_ENABLED_PARAM,
        OUTPUT_RANGE_PARAM,
        HOVERED_SCALED_PARAM_VALUE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        PARAM_VALUE_OUTPUT,
        SCALED_PARAM_VALUE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    enum HoverEnabled {OFF, WITH_SHIFT, ALWAYS};

    HoveredValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    json_t* toJson() override;
    void fromJson(json_t *rootJ) override;

    HoverEnabled enabled = WITH_SHIFT;

    VoltageRange outputRange = MINUS_PLUS_FIVE;

    bool useTooltip = true;

};

void HoveredValue::step() {
    outputs[PARAM_VALUE_OUTPUT].value = params[HOVERED_PARAM_VALUE_PARAM].value;
    outputs[SCALED_PARAM_VALUE_OUTPUT].value = params[HOVERED_SCALED_PARAM_VALUE_PARAM].value;
}

json_t* HoveredValue::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "useTooltip", json_boolean(useTooltip));
    return rootJ;
}

void HoveredValue::fromJson(json_t *rootJ) {
    json_t *useTooltipJ = json_object_get(rootJ, "useTooltip");
    if (useTooltipJ) {
        useTooltip = json_boolean_value(useTooltipJ);
    }
}

struct HoveredValueWidget : ModuleWidget {
    HoveredValueWidget(HoveredValue *module);

    void step() override;
    void onChange(EventChange &e) override;
    void tooltipHide();
    void tooltipShow(std::string tooltipText, Widget *hoveredWidget);

    Menu *createContextMenu() override;

    ParamWidget *enableHoverSwitch;
    ParamWidget *outputRangeSwitch;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;

    Tooltip *tooltip = NULL;
};

HoveredValueWidget::HoveredValueWidget(HoveredValue *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/HoveredValue.svg")));

    float y_baseline = 45.0f;

    Vec text_field_size = Vec(70.0f, 22.0f);

    float x_pos = 10.0f;

    y_baseline = 38.0f;

    tooltip = new Tooltip();

    param_value_field = new ParamFloatField(module);
    param_value_field->box.pos = Vec(x_pos, y_baseline);
    param_value_field->box.size = text_field_size;

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

    // Scaled output and scaled output range
    // y_baseline = y_baseline + 25.0f;
    y_baseline = box.size.y - 128.0f;

    outputRangeSwitch = ParamWidget::create<CKSSThree>(Vec(5, y_baseline), module,
        HoveredValue::OUTPUT_RANGE_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(outputRangeSwitch);

    // Scaled output port
    Port *scaled_value_out_port = Port::create<PJ301MPort>(
        Vec(60.0f, y_baseline - 2.0f),
        Port::OUTPUT,
        module,
        HoveredValue::SCALED_PARAM_VALUE_OUTPUT);

    outputs.push_back(scaled_value_out_port);

    addChild(scaled_value_out_port);

    // enabled/disable switch
    y_baseline = box.size.y - 65.0f;

    enableHoverSwitch = ParamWidget::create<CKSSThree>(Vec(5, box.size.y - 62.0f), module,
        HoveredValue::HOVER_ENABLED_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(enableHoverSwitch);

    Port *raw_value_out_port = Port::create<PJ301MPort>(
        Vec(60.0f, box.size.y - 67.0f),
        Port::OUTPUT,
        module,
        HoveredValue::PARAM_VALUE_OUTPUT);

    outputs.push_back(raw_value_out_port);

    addChild(raw_value_out_port);

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

    // fire off an event to refresh all the widgets
    EventChange e;
    onChange(e);
}

void HoveredValueWidget::tooltipHide() {
    // assert?
    if (!tooltip) {
        // debug("tooltip was null");
        return;
    }

    if (!tooltip->parent) { return; }

    // debug("tooltipHide[res=%s]: tt->text:%s", reason.c_str(), tooltip->text.c_str());

    if (!RACK_PLUGIN_UI_RACKSCENE) {
        // debug("gScene was null");
        return;
    }
    if (tooltip->parent) {
        RACK_PLUGIN_UI_RACKSCENE->removeChild(tooltip);
    }

}

void HoveredValueWidget::tooltipShow(std::string tooltipText, Widget *hoveredWidget) {

    ModuleWidget *hoveredParent = hoveredWidget->getAncestorOfType<ModuleWidget>();
    if (!hoveredParent) {
        return;
    }

    if (!reinterpret_cast<HoveredValue *>(module)->useTooltip) {
        return;
    }

    // debug("hoveredParent pos (%f, %f)", hoveredParent->box.pos.x, hoveredParent->box.pos.y);

    Vec offsetFromHovered = Vec(20.0, -20.0f);
    Vec absHovered = hoveredParent->getAbsoluteOffset(hoveredWidget->box.pos);
    Vec hoveredOffset = absHovered.plus(offsetFromHovered);

    tooltip->box.pos = hoveredOffset;

    // debug("tooltip show: (%f, %f) pos: (%f, %f)", tooltip->box.pos.x, tooltip->box.pos.y,
    // hoveredWidget->box.pos.x, hoveredWidget->box.pos.y);

    tooltip->text = tooltipText;
    if (!tooltip->parent) {
        RACK_PLUGIN_UI_RACKSCENE->addChild(tooltip);
    }
}

void HoveredValueWidget::step() {
    /* TODO: would be useful to be more explicit about the state here,
        ie, states indicating if hovered widget is not injectable or not,
        and if we are actively injecting or not. And transitions for when
        we start hovering and stop hovering over a widget.
    */

    float display_min = -5.0f;
    float display_max = 5.0f;
    float display_default = 0.0f;

    std::string display_type = "";
    std::string tooltipText;

    float raw_value = 0.0f;

    ModuleWidget::step();

    bool shift_pressed = windowIsShiftPressed();

    if (!RACK_PLUGIN_UI_HOVERED_WIDGET) {
        tooltipHide();
        return;
    }

    if (module->params[HoveredValue::HOVER_ENABLED_PARAM].value == HoveredValue::OFF) {
        tooltipHide();
        return;
    }

    if (module->params[HoveredValue::HOVER_ENABLED_PARAM].value == HoveredValue::WITH_SHIFT &&!shift_pressed) {
        tooltipHide();
        return;
    }

    VoltageRange outputRange = (VoltageRange) int32_t(round(module->params[HoveredValue::OUTPUT_RANGE_PARAM].value));

    ParamWidget *pwidget = dynamic_cast<ParamWidget *>(RACK_PLUGIN_UI_HOVERED_WIDGET);

    if (pwidget) {
        // TODO: show value of original and scaled?

        raw_value = pwidget->value;
        display_min = pwidget->minValue;
        display_max = pwidget->maxValue;
        display_default = pwidget->defaultValue;
        display_type = "param";

        // TODO: if we use type name detection stuff (cxxabi/typeinfo/etc) we could possibly
        //       also show the name of the hovered widget as a hint on mystery meat params
        // TODO: anyway to get the orig literal name of an enum value (ie, LFO_VC_OUTPUT etc)
        //       at runtime? might also be hint

    }

    Port *port = dynamic_cast<Port *>(RACK_PLUGIN_UI_HOVERED_WIDGET);
    if (port) {
        if (port->type == port->INPUT) {
            raw_value = port->module->inputs[port->portId].value;
            display_type = "input";
        }
        if (port->type == port->OUTPUT) {
            raw_value = port->module->outputs[port->portId].value;
            display_type = "output";
        }

        // inputs/outputs dont have variable min/max, so just use the -10/+10 and
        // 0 for the default to get the point across.
        display_min = -10.0f;
        display_max = 10.0f;
        display_default = 0.0f;
    }

    if (!pwidget && !port) {
        tooltipHide();
    } else {
        // TODO build fancier tool tip text
        // TODO maybe just draw a widget like a tooltip, would be cool to draw a pop up a mini 'scope'
        tooltipShow(stringf("%s\n%#.4g", display_type.c_str(), raw_value), RACK_PLUGIN_UI_HOVERED_WIDGET);
    }

    float scaled_value = rescale(raw_value, display_min, display_max,
                                 voltage_min[outputRange],
                                 voltage_max[outputRange]);

    engineSetParam(module, HoveredValue::HOVERED_PARAM_VALUE_PARAM, raw_value);
    engineSetParam(module, HoveredValue::HOVERED_SCALED_PARAM_VALUE_PARAM, scaled_value);

    param_value_field->setValue(raw_value);
    min_field->setText(stringf("%#.4g", display_min));
    max_field->setText(stringf("%#.4g", display_max));
    default_field->setText(stringf("%#.4g", display_default));
    widget_type_field->setText(display_type);

    // TODO: if a WireWidget, can we figure out it's in/out and current value? That would be cool,
    //       though it doesn't look like WireWidgets are ever hovered (or gHoveredWidget never
    //       seems to be a WireWidget).
}

void HoveredValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

struct UseTooltipMenuItem : MenuItem {
    HoveredValue *module;

    void onAction(EventAction &e) override {
        if (module->useTooltip) {
            module->useTooltip = false;
        } else {
            module->useTooltip = true;
        }
    }

    void step() override {
        rightText = CHECKMARK(module->useTooltip);
    }
};


Menu *HoveredValueWidget::createContextMenu() {
    Menu *menu = ModuleWidget::createContextMenu();

    MenuLabel *spacerLabel = new MenuLabel();
    menu->addChild(spacerLabel);

    HoveredValue *hoveredValue = dynamic_cast<HoveredValue*>(module);
    assert(hoveredValue);

    UseTooltipMenuItem *useTooltipMenuItem =
        MenuItem::create<UseTooltipMenuItem>("Show Tooltip",
                                             CHECKMARK(hoveredValue->useTooltip > 2.0f));

    useTooltipMenuItem->module = hoveredValue;
    menu->addChild(useTooltipMenuItem);

    return menu;
}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

RACK_PLUGIN_MODEL_INIT(Alikins, HoveredValue) {
   Model *modelHoveredValue = Model::create<HoveredValue, HoveredValueWidget>(
      "Alikins", "HoveredValue", "Hovered Value - get value under cursor", UTILITY_TAG, CONTROLLER_TAG);
   return modelHoveredValue;
}
