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

struct Paramath : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        IN_A,
        IN_B,
        NUM_INPUTS
    };
    enum OutputIds
    {
        A_GEQ_B,
        A_EQ_B,
        MIN,
        MAX,
        A_MUL_B,
        PYTHAGORAS,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Paramath() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void Paramath::step()
{
    float in_a = inputs[IN_A].value;
    float in_b = inputs[IN_B].value;

    if (in_a >= in_b)
    {
        outputs[A_GEQ_B].value = 5.0;
        outputs[MIN].value = in_b;
        outputs[MAX].value = in_a;
    }
    else
    {
        outputs[A_GEQ_B].value = 0.0;
        outputs[MIN].value = in_a;
        outputs[MAX].value = in_b;
    }

    if (in_a == in_b)
    {
        outputs[A_EQ_B].value = 5.0;
    }
    else
    {
        outputs[A_EQ_B].value = 0.0;
    }

    // These two value are computed on normalized [-1.0; 1.0]
    outputs[A_MUL_B].value = (in_a * in_b) * 5.0;
    outputs[PYTHAGORAS].value = sqrt(powf(in_a / 5.0, 2.0) + powf(in_b / 5.0, 2.0)) * 5.0;
}

struct ParamathWidget : ModuleWidget
{
    ParamathWidget(Paramath *module);
};

ParamathWidget::ParamathWidget(Paramath *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 6, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/paramath.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 67), Port::INPUT, module, Paramath::IN_A));
    addInput(Port::create<PJ301MPort>(Vec(52, 67), Port::INPUT, module, Paramath::IN_B));

    addOutput(Port::create<PJ301MPort>(Vec(14, 132), Port::OUTPUT, module, Paramath::A_GEQ_B));
    addOutput(Port::create<PJ301MPort>(Vec(52, 132), Port::OUTPUT, module, Paramath::A_EQ_B));
    addOutput(Port::create<PJ301MPort>(Vec(14, 197), Port::OUTPUT, module, Paramath::MIN));
    addOutput(Port::create<PJ301MPort>(Vec(52, 197), Port::OUTPUT, module, Paramath::MAX));
    addOutput(Port::create<PJ301MPort>(Vec(14, 262), Port::OUTPUT, module, Paramath::A_MUL_B));
    addOutput(Port::create<PJ301MPort>(Vec(52, 262), Port::OUTPUT, module, Paramath::PYTHAGORAS));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Paramath) {
   Model *modelParamath = Model::create<Paramath, ParamathWidget>("Sonus Modular", "Paramath", "Paramath | Comparing and Maths", LOGIC_TAG, UTILITY_TAG);
   return modelParamath;
}
