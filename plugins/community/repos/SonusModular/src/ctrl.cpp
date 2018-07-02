/******************************************************************************
 * Copyright 2017-2018 Valerio Orlandini / Sonus Dept. <sonusdept@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/


#include "sonusmodular.hpp"

namespace rack_plugin_SonusModular {

struct Ctrl : Module
{
    enum ParamIds
    {
        KNOB_1,
        KNOB_2,
        KNOB_3,
        KNOB_4,
        KNOB_5,
        KNOB_6,
        KNOB_7,
        KNOB_8,
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        KNOB_1_OUT,
        KNOB_2_OUT,
        KNOB_3_OUT,
        KNOB_4_OUT,
        KNOB_5_OUT,
        KNOB_6_OUT,
        KNOB_7_OUT,
        KNOB_8_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Ctrl() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void Ctrl::step()
{
    outputs[KNOB_1_OUT].value = params[KNOB_1].value;
    outputs[KNOB_2_OUT].value = params[KNOB_2].value;
    outputs[KNOB_3_OUT].value = params[KNOB_3].value;
    outputs[KNOB_4_OUT].value = params[KNOB_4].value;
    outputs[KNOB_5_OUT].value = params[KNOB_5].value;
    outputs[KNOB_6_OUT].value = params[KNOB_6].value;
    outputs[KNOB_7_OUT].value = params[KNOB_7].value;
    outputs[KNOB_8_OUT].value = params[KNOB_8].value;
}

struct CtrlWidget : ModuleWidget
{
    CtrlWidget(Ctrl *module);

    TextField *ctrlLabel[8];
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};

json_t *CtrlWidget::toJson()
{
    json_t *rootJ = ModuleWidget::toJson();

    json_object_set_new(rootJ, "knob_1_label", json_string(ctrlLabel[0]->text.c_str()));
    json_object_set_new(rootJ, "knob_2_label", json_string(ctrlLabel[1]->text.c_str()));
    json_object_set_new(rootJ, "knob_3_label", json_string(ctrlLabel[2]->text.c_str()));
    json_object_set_new(rootJ, "knob_4_label", json_string(ctrlLabel[3]->text.c_str()));
    json_object_set_new(rootJ, "knob_5_label", json_string(ctrlLabel[4]->text.c_str()));
    json_object_set_new(rootJ, "knob_6_label", json_string(ctrlLabel[5]->text.c_str()));
    json_object_set_new(rootJ, "knob_7_label", json_string(ctrlLabel[6]->text.c_str()));
    json_object_set_new(rootJ, "knob_8_label", json_string(ctrlLabel[7]->text.c_str()));

    return rootJ;
}


void CtrlWidget::fromJson(json_t *rootJ)
{
    ModuleWidget::fromJson(rootJ);

    json_t *knob_1_label = json_object_get(rootJ, "knob_1_label");
    if (knob_1_label)
    {
        ctrlLabel[0]->text = json_string_value(knob_1_label);
    }
    json_t *knob_2_label = json_object_get(rootJ, "knob_2_label");
    if (knob_2_label)
    {
        ctrlLabel[1]->text = json_string_value(knob_2_label);
    }
    json_t *knob_3_label = json_object_get(rootJ, "knob_3_label");
    if (knob_3_label)
    {
        ctrlLabel[2]->text = json_string_value(knob_3_label);
    }
    json_t *knob_4_label = json_object_get(rootJ, "knob_4_label");
    if (knob_4_label)
    {
        ctrlLabel[3]->text = json_string_value(knob_4_label);
    }
    json_t *knob_5_label = json_object_get(rootJ, "knob_5_label");
    if (knob_5_label)
    {
        ctrlLabel[4]->text = json_string_value(knob_5_label);
    }
    json_t *knob_6_label = json_object_get(rootJ, "knob_6_label");
    if (knob_6_label)
    {
        ctrlLabel[5]->text = json_string_value(knob_6_label);
    }
    json_t *knob_7_label = json_object_get(rootJ, "knob_7_label");
    if (knob_7_label)
    {
        ctrlLabel[6]->text = json_string_value(knob_7_label);
    }
    json_t *knob_8_label = json_object_get(rootJ, "knob_8_label");
    if (knob_8_label)
    {
        ctrlLabel[7]->text = json_string_value(knob_8_label);
    }
}

CtrlWidget::CtrlWidget(Ctrl *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 16, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/ctrl.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addOutput(Port::create<PJ301MPort>(Vec(18, 80), Port::OUTPUT, module, Ctrl::KNOB_1_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(78, 80), Port::OUTPUT, module, Ctrl::KNOB_2_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(138, 80), Port::OUTPUT, module, Ctrl::KNOB_3_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(198, 80), Port::OUTPUT, module, Ctrl::KNOB_4_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(18, 210), Port::OUTPUT, module, Ctrl::KNOB_5_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(78, 210), Port::OUTPUT, module, Ctrl::KNOB_6_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(138, 210), Port::OUTPUT, module, Ctrl::KNOB_7_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(198, 210), Port::OUTPUT, module, Ctrl::KNOB_8_OUT));

    addParam(ParamWidget::create<SonusKnob>(Vec(12, 120), module, Ctrl::KNOB_1, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(72, 120), module, Ctrl::KNOB_2, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(132, 120), module, Ctrl::KNOB_3, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(192, 120), module, Ctrl::KNOB_4, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(12, 250), module, Ctrl::KNOB_5, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(72, 250), module, Ctrl::KNOB_6, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(132, 250), module, Ctrl::KNOB_7, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(192, 250), module, Ctrl::KNOB_8, -5.0, 5.0, 0.0));

    for (unsigned int f = 0; f < 8; f++)
    {
        ctrlLabel[f] = new TextField();
        ctrlLabel[f]->box.pos = Vec(8 + (60 * (f % 4)), 160 + (130 * (int)floorf(f / 4.0)));
        ctrlLabel[f]->box.size = Vec(44, 36);
        ctrlLabel[f]->multiline = true;
        addChild(ctrlLabel[f]);
    }
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Ctrl) {
   Model *modelCtrl = Model::create<Ctrl, CtrlWidget>("Sonus Modular", "Ctrl", "Ctrl | Customizable Controller", CONTROLLER_TAG, UTILITY_TAG);
   return modelCtrl;
}
