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

struct Multimulti : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        A1_INPUT,
        A2_INPUT,
        B1_INPUT,
        B2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        A1_OUTPUT,
        A2_OUTPUT,
        A3_OUTPUT,
        A4_OUTPUT,
        A5_OUTPUT,
        A6_OUTPUT,
        A7_OUTPUT,
        A8_OUTPUT,
        B1_OUTPUT,
        B2_OUTPUT,
        B3_OUTPUT,
        B4_OUTPUT,
        B5_OUTPUT,
        B6_OUTPUT,
        B7_OUTPUT,
        B8_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Multimulti() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void Multimulti::step()
{
    float in1 = inputs[A1_INPUT].value + inputs[A2_INPUT].value;
    float in2 = inputs[B1_INPUT].value + inputs[B2_INPUT].value;

    outputs[A1_OUTPUT].value = in1;
    outputs[A2_OUTPUT].value = in1;
    outputs[A3_OUTPUT].value = in1;
    outputs[A4_OUTPUT].value = in1;
    outputs[A5_OUTPUT].value = in1;
    outputs[A6_OUTPUT].value = in1;
    outputs[A7_OUTPUT].value = in1;
    outputs[A8_OUTPUT].value = in1;
    outputs[B1_OUTPUT].value = in2;
    outputs[B2_OUTPUT].value = in2;
    outputs[B3_OUTPUT].value = in2;
    outputs[B4_OUTPUT].value = in2;
    outputs[B5_OUTPUT].value = in2;
    outputs[B6_OUTPUT].value = in2;
    outputs[B7_OUTPUT].value = in2;
    outputs[B8_OUTPUT].value = in2;
}

struct MultimultiWidget : ModuleWidget
{
    MultimultiWidget(Multimulti *module);
};


MultimultiWidget::MultimultiWidget(Multimulti *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 8, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/multimulti.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(18, 67), Port::INPUT, module, Multimulti::A1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(78, 67), Port::INPUT, module, Multimulti::A2_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(3, 125), Port::OUTPUT, module, Multimulti::A1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(33, 125), Port::OUTPUT, module, Multimulti::A2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(63, 125), Port::OUTPUT, module, Multimulti::A3_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(93, 125), Port::OUTPUT, module, Multimulti::A4_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(3, 155), Port::OUTPUT, module, Multimulti::A5_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(33, 155), Port::OUTPUT, module, Multimulti::A6_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(63, 155), Port::OUTPUT, module, Multimulti::A7_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(93, 155), Port::OUTPUT, module, Multimulti::A8_OUTPUT));

    addInput(Port::create<PJ301MPort>(Vec(18, 227), Port::INPUT, module, Multimulti::B1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(78, 227), Port::INPUT, module, Multimulti::B2_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(3, 285), Port::OUTPUT, module, Multimulti::B1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(33, 285), Port::OUTPUT, module, Multimulti::B2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(63, 285), Port::OUTPUT, module, Multimulti::B3_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(93, 285), Port::OUTPUT, module, Multimulti::B4_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(3, 315), Port::OUTPUT, module, Multimulti::B5_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(33, 315), Port::OUTPUT, module, Multimulti::B6_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(63, 315), Port::OUTPUT, module, Multimulti::B7_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(93, 315), Port::OUTPUT, module, Multimulti::B8_OUTPUT));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Multimulti) {
   Model *modelMultimulti = Model::create<Multimulti, MultimultiWidget>("Sonus Modular", "Multimulti", "Multimulti | 2x8 Multiples", MULTIPLE_TAG, UTILITY_TAG);
   return modelMultimulti;
}
