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

struct Piconoise : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
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
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Piconoise() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        srand(time(0));
    }
    void step() override;
};


void Piconoise::step()
{
    float out = (2.0 * (rand() / (float)RAND_MAX)) - 1.0;

    outputs[A1_OUTPUT].value = 5.0 * out;
    outputs[A2_OUTPUT].value = 5.0 * out;
    outputs[A3_OUTPUT].value = 5.0 * out;
    outputs[A4_OUTPUT].value = 5.0 * out;
    outputs[A5_OUTPUT].value = 5.0 * out;
    outputs[A6_OUTPUT].value = 5.0 * out;
}

struct PiconoiseWidget : ModuleWidget
{
    PiconoiseWidget(Piconoise *module);
};

PiconoiseWidget::PiconoiseWidget(Piconoise *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 4, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/piconoise.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addOutput(Port::create<PJ301MPort>(Vec(18, 67), Port::OUTPUT, module, Piconoise::A1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(18, 112), Port::OUTPUT, module, Piconoise::A2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(18, 157), Port::OUTPUT, module, Piconoise::A3_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(18, 202), Port::OUTPUT, module, Piconoise::A4_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(18, 247), Port::OUTPUT, module, Piconoise::A5_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(18, 292), Port::OUTPUT, module, Piconoise::A6_OUTPUT));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Piconoise) {
   Model *modelPiconoise = Model::create<Piconoise, PiconoiseWidget>("Sonus Modular", "Piconoise", "Piconoise | Noise Generator", NOISE_TAG);
   return modelPiconoise;
}
