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

struct Pith : Module
{
    enum ParamIds
    {
        DELAY,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        L_OUTPUT,
        R_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Pith() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    std::deque<float> buffer{std::deque<float>(4096, 0.0)};
    unsigned int current_sample = 0;
};

void Pith::step()
{
    float in = inputs[INPUT].value;

    buffer.at(current_sample) = in;

    unsigned int right_sample = (current_sample + (unsigned int)floorf(params[DELAY].value * engineGetSampleRate() * 0.001)) % 4096;

    outputs[L_OUTPUT].value = in;
    outputs[R_OUTPUT].value = buffer.at(right_sample);

    if (++current_sample >= 4096)
    {
        current_sample = 0;
    }
}

struct PithWidget : ModuleWidget
{
    PithWidget(Pith *module);
};

PithWidget::PithWidget(Pith *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 6, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/pith.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(33.3, 66), Port::INPUT, module, Pith::INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(13.2, 266), Port::OUTPUT, module, Pith::L_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(51.2, 266), Port::OUTPUT, module, Pith::R_OUTPUT));

    addParam(ParamWidget::create<SonusBigKnob>(Vec(19, 155), module, Pith::DELAY, 0.0, 10.0, 5.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Pith) {
   Model *modelPith = Model::create<Pith, PithWidget>("Sonus Modular", "Pan in the Haas", "Pan in the Haas | Haas Delay", DELAY_TAG, DUAL_TAG, PANNING_TAG);
   return modelPith;
}
