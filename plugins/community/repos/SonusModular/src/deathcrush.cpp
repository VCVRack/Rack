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

struct Deathcrush : Module
{
    enum ParamIds
    {
        DRIVE1,
        DRIVE2,
        BITS,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Deathcrush() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void Deathcrush::step()
{
    float in = inputs[INPUT].value / 5.0;
    float out = 0.0;
    float drive1_amount = params[DRIVE1].value;
    float drive2_amount = params[DRIVE2].value;
    float bits = params[BITS].value;

    out = (in * (1.0 - drive1_amount)) + ((copysign(1.0, in) * tan(powf(fabs(in), 0.25)) * drive1_amount * 0.75));

    if (fabs(in) > (1.0 - drive2_amount))
    {
        out = (out * (1.0 - drive2_amount)) + (copysign(1.0, in) * drive2_amount);
    }

    out = out * powf(2.0, bits - 1.0);
    out = round(out);
    out = out / powf(2.0, bits - 1.0);

    outputs[OUTPUT].value = out * 5.0;
}

struct DeathcrushWidget : ModuleWidget
{
    DeathcrushWidget(Deathcrush *module);
};

DeathcrushWidget::DeathcrushWidget(Deathcrush *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 6, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/deathcrush.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 67), Port::INPUT, module, Deathcrush::INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(52, 67), Port::OUTPUT, module, Deathcrush::OUTPUT));

    addParam(ParamWidget::create<SonusKnob>(Vec(27, 150), module, Deathcrush::DRIVE1, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(27, 210), module, Deathcrush::DRIVE2, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusBigKnob>(Vec(18, 275), module, Deathcrush::BITS, 1.0, 12.0, 12.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Deathcrush) {
   Model *modelDeathcrush = Model::create<Deathcrush, DeathcrushWidget>("Sonus Modular", "Deathcrush", "Deathcrush | Driver and Crusher", DISTORTION_TAG, EFFECT_TAG);
   return modelDeathcrush;
}

  
