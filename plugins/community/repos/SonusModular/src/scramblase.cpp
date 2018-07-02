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

struct Scramblase : Module
{
    enum ParamIds
    {
        THRESHOLD,
        NUM_PARAMS
    };
    enum InputIds
    {
        IN_A,
        IN_B,
        IN_C,
        THRESHOLD_CV,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUT_A1,
        OUT_A2,
        OUT_A3,
        OUT_A4,
        OUT_B1,
        OUT_B2,
        OUT_B3,
        OUT_B4,
        OUT_C1,
        OUT_C2,
        OUT_C3,
        OUT_C4,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Scramblase() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    float threshold = 0.0;
};


void Scramblase::step()
{
    float in_a = inputs[IN_A].value / 5.0;
    float in_b = inputs[IN_B].value / 5.0;
    float in_c = inputs[IN_C].value / 5.0;

    float out_a1, out_a2, out_a3, out_a4;
    float out_b1, out_b2, out_b3, out_b4;
    float out_c1, out_c2, out_c3, out_c4;

    threshold = params[THRESHOLD].value + inputs[THRESHOLD_CV].value / 5.0;
    if (threshold > 1.0)
    {
        threshold = 1.0;
    }
    if (threshold < 0.0)
    {
        threshold = 0.0;
    }

    if (fabs(in_a) > threshold)
    {
        out_a1 = (fabs(in_a) - 2.0 * (fabs(in_a) - threshold)) * copysign(1.0, in_a);
        out_a2 = threshold * copysign (1.0, in_a);
        out_a3 = 1.0 * copysign (1.0, in_a);
        out_a4 = (fabs(in_a) - 2.0 * (fabs(in_a) - threshold)) * copysign(1.0, in_a);
    }
    else
    {
        out_a1 = in_a;
        out_a2 = in_a;
        out_a3 = in_a;
        out_a4 = 1.0 * copysign(1.0, in_a);
    }

    if (fabs(in_b) > threshold)
    {
        out_b1 = (fabs(in_b) - 2.0 * (fabs(in_b) - threshold)) * copysign(1.0, in_b);
        out_b2 = threshold * copysign (1.0, in_b);
        out_b3 = 1.0 * copysign (1.0, in_b);
        out_b4 = (fabs(in_b) - 2.0 * (fabs(in_b) - threshold)) * copysign(1.0, in_b);
    }
    else
    {
        out_b1 = in_b;
        out_b2 = in_b;
        out_b3 = in_b;
        out_b4 = 1.0 * copysign(1.0, in_b);
    }

    if (fabs(in_c) > threshold)
    {
        out_c1 = (fabs(in_c) - 2.0 * (fabs(in_c) - threshold)) * copysign(1.0, in_c);
        out_c2 = threshold * copysign (1.0, in_c);
        out_c3 = 1.0 * copysign (1.0, in_c);
        out_c4 = (fabs(in_c) - 2.0 * (fabs(in_c) - threshold)) * copysign(1.0, in_c);
    }
    else
    {
        out_c1 = in_c;
        out_c2 = in_c;
        out_c3 = in_c;
        out_c4 = 1.0 * copysign(1.0, in_c);
    }

    outputs[OUT_A1].value = out_a1 * 5.0;
    outputs[OUT_A2].value = out_a2 * 5.0;
    outputs[OUT_A3].value = out_a3 * 5.0;
    outputs[OUT_A4].value = out_a4 * 5.0;

    outputs[OUT_B1].value = out_b1 * 5.0;
    outputs[OUT_B2].value = out_b2 * 5.0;
    outputs[OUT_B3].value = out_b3 * 5.0;
    outputs[OUT_B4].value = out_b4 * 5.0;

    outputs[OUT_C1].value = out_c1 * 5.0;
    outputs[OUT_C2].value = out_c2 * 5.0;
    outputs[OUT_C3].value = out_c3 * 5.0;
    outputs[OUT_C4].value = out_c4 * 5.0;
}

struct ScramblaseWidget : ModuleWidget
{
    ScramblaseWidget(Scramblase *module);
};

ScramblaseWidget::ScramblaseWidget(Scramblase *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 8, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/scramblase.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(12, 67), Port::INPUT, module, Scramblase::IN_A));
    addOutput(Port::create<PJ301MPort>(Vec(12, 121), Port::OUTPUT, module, Scramblase::OUT_A1));
    addOutput(Port::create<PJ301MPort>(Vec(12, 150), Port::OUTPUT, module, Scramblase::OUT_A2));
    addOutput(Port::create<PJ301MPort>(Vec(12, 179), Port::OUTPUT, module, Scramblase::OUT_A3));
    addOutput(Port::create<PJ301MPort>(Vec(12, 208), Port::OUTPUT, module, Scramblase::OUT_A4));

    addInput(Port::create<PJ301MPort>(Vec(47, 67), Port::INPUT, module, Scramblase::IN_B));
    addOutput(Port::create<PJ301MPort>(Vec(47, 120), Port::OUTPUT, module, Scramblase::OUT_B1));
    addOutput(Port::create<PJ301MPort>(Vec(47, 150), Port::OUTPUT, module, Scramblase::OUT_B2));
    addOutput(Port::create<PJ301MPort>(Vec(47, 179), Port::OUTPUT, module, Scramblase::OUT_B3));
    addOutput(Port::create<PJ301MPort>(Vec(47, 208), Port::OUTPUT, module, Scramblase::OUT_B4));

    addInput(Port::create<PJ301MPort>(Vec(83, 67), Port::INPUT, module, Scramblase::IN_C));
    addOutput(Port::create<PJ301MPort>(Vec(83, 121), Port::OUTPUT, module, Scramblase::OUT_C1));
    addOutput(Port::create<PJ301MPort>(Vec(83, 150), Port::OUTPUT, module, Scramblase::OUT_C2));
    addOutput(Port::create<PJ301MPort>(Vec(83, 179), Port::OUTPUT, module, Scramblase::OUT_C3));
    addOutput(Port::create<PJ301MPort>(Vec(83, 208), Port::OUTPUT, module, Scramblase::OUT_C4));

    addInput(Port::create<PJ301MPort>(Vec(12, 290), Port::INPUT, module, Scramblase::THRESHOLD_CV));

    addParam(ParamWidget::create<SonusBigKnob>(Vec(53, 275), module, Scramblase::THRESHOLD, 0.0, 1.0, 1.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Scramblase) {
   Model *modelScramblase = Model::create<Scramblase, ScramblaseWidget>("Sonus Modular", "Scramblase", "Scramblase | Waveshaper", WAVESHAPER_TAG, EFFECT_TAG);
   return modelScramblase;
}
