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

struct Harmony : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH,
        NUM_INPUTS
    };
    enum OutputIds
    {
        m2,
        M2,
        m3,
        M3,
        P4,
        TT,
        P5,
        m6,
        M6,
        m7,
        M7,
        P8,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Harmony() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    const float semitone = 1.0 / 12.0;
};


void Harmony::step()
{
    float pitch = inputs[PITCH].value;

    for (int s = 0; s < 12; s++)
    {
        outputs[s].value = pitch + (semitone * (s + 1));
    }
}

struct HarmonyWidget : ModuleWidget
{
    HarmonyWidget(Harmony *module);
};

HarmonyWidget::HarmonyWidget(Harmony *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 12, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/harmony.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(78, 67), Port::INPUT, module, Harmony::PITCH));

    addOutput(Port::create<PJ301MPort>(Vec(20, 132), Port::OUTPUT, module, Harmony::m2));
    addOutput(Port::create<PJ301MPort>(Vec(58, 132), Port::OUTPUT, module, Harmony::M2));
    addOutput(Port::create<PJ301MPort>(Vec(96, 132), Port::OUTPUT, module, Harmony::m3));
    addOutput(Port::create<PJ301MPort>(Vec(134, 132), Port::OUTPUT, module, Harmony::M3));
    addOutput(Port::create<PJ301MPort>(Vec(20, 197), Port::OUTPUT, module, Harmony::P4));
    addOutput(Port::create<PJ301MPort>(Vec(58, 197), Port::OUTPUT, module, Harmony::TT));
    addOutput(Port::create<PJ301MPort>(Vec(96, 197), Port::OUTPUT, module, Harmony::P5));
    addOutput(Port::create<PJ301MPort>(Vec(134, 197), Port::OUTPUT, module, Harmony::m6));
    addOutput(Port::create<PJ301MPort>(Vec(20, 262), Port::OUTPUT, module, Harmony::M6));
    addOutput(Port::create<PJ301MPort>(Vec(58, 262), Port::OUTPUT, module, Harmony::m7));
    addOutput(Port::create<PJ301MPort>(Vec(96, 262), Port::OUTPUT, module, Harmony::M7));
    addOutput(Port::create<PJ301MPort>(Vec(134, 262), Port::OUTPUT, module, Harmony::P8));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Harmony) {
   Model *modelHarmony = Model::create<Harmony, HarmonyWidget>("Sonus Modular", "Harmony", "Harmony | Chord Tool", TUNER_TAG, UTILITY_TAG);
   return modelHarmony;
}
