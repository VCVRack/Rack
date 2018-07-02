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

struct Bymidside : Module
{
    enum ParamIds
    {
        M_GAIN,
        S_GAIN,
        NUM_PARAMS
    };
    enum InputIds
    {
        L_INPUT,
        R_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        M_OUTPUT,
        S_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Bymidside() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void Bymidside::step()
{
    float mid = inputs[L_INPUT].value + inputs[R_INPUT].value;
    float side = inputs[L_INPUT].value - inputs[R_INPUT].value;

    float mid_gain = params[M_GAIN].value;

    float side_gain = params[S_GAIN].value;

    outputs[M_OUTPUT].value = mid * mid_gain;
    outputs[S_OUTPUT].value = side * side_gain;
}

struct BymidsideWidget : ModuleWidget
{
    BymidsideWidget(Bymidside *module);
};

BymidsideWidget::BymidsideWidget(Bymidside *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 6, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/bymidside.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 67), Port::INPUT, module, Bymidside::L_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(52, 67), Port::INPUT, module, Bymidside::R_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(14, 132), Port::OUTPUT, module, Bymidside::M_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(52, 132), Port::OUTPUT, module, Bymidside::S_OUTPUT));

    addParam(ParamWidget::create<SonusBigKnob>(Vec(18, 195), module, Bymidside::M_GAIN, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<SonusBigKnob>(Vec(18, 275), module, Bymidside::S_GAIN, 0.0, 2.0, 1.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Bymidside) {
   Model *modelBymidside = Model::create<Bymidside, BymidsideWidget>("Sonus Modular", "Bymidside", "Bymidside | MS Encoder", DYNAMICS_TAG, UTILITY_TAG);
   return modelBymidside;
}
