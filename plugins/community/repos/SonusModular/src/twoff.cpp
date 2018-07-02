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

struct Twoff : Module
{
    enum ParamIds
    {
        OFF_A,
        OFF_B,
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
        OUT_A,
        OUT_B,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Twoff() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void Twoff::step()
{
    float in_a = inputs[IN_A].value;
    float in_b = inputs[IN_B].value;

    outputs[OUT_A].value = in_a + params[OFF_A].value;
    outputs[OUT_B].value = in_b + params[OFF_B].value;
}

struct TwoffWidget : ModuleWidget
{
    TwoffWidget(Twoff *module);
};

TwoffWidget::TwoffWidget(Twoff *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 4, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/twoff.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(18, 67), Port::INPUT, module, Twoff::IN_A));
    addInput(Port::create<PJ301MPort>(Vec(18, 202), Port::INPUT, module, Twoff::IN_B));
    addOutput(Port::create<PJ301MPort>(Vec(18, 157), Port::OUTPUT, module, Twoff::OUT_A));
    addOutput(Port::create<PJ301MPort>(Vec(18, 292), Port::OUTPUT, module, Twoff::OUT_B));

    addParam(ParamWidget::create<SonusKnob>(Vec(12, 99), module, Twoff::OFF_A, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(12, 235), module, Twoff::OFF_B, -5.0, 5.0, 0.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Twoff) {
   Model *modelTwoff = Model::create<Twoff, TwoffWidget>("Sonus Modular", "Twoff", "Twoff | CV Offset",  UTILITY_TAG);
   return modelTwoff;
}
