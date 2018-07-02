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

struct Bitter : Module
{
    enum ParamIds
    {
        BIT_1,
        BIT_2,
        BIT_3,
        BIT_4,
        BIT_5,
        BIT_6,
        BIT_7,
        BIT_8,
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

    Bitter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

inline void clear_bit(unsigned char &number, unsigned int bit)
{
    number &= ~(1 << bit);
}

inline void toggle_bit(unsigned char &number, unsigned int bit)
{
    number ^= 1 << bit;
}

void Bitter::step()
{
    unsigned char in = (unsigned char)round((1.0 + (inputs[INPUT].value / 5.0)) * 0.5 * 255.0);

    for (int b = 0; b < 8; b++)
    {
        if ((int)roundf(params[b].value) == 1)
        {
            clear_bit(in, b);
        }
        if ((int)roundf(params[b].value) == 0)
        {
            toggle_bit(in, b);
        }
    }

    float out = (((float)in / 255.0) * 2.0) - 1.0;

    outputs[OUTPUT].value = out * 5.0;
}

struct BitterWidget : ModuleWidget
{
    BitterWidget(Bitter *module);
};

BitterWidget::BitterWidget(Bitter *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 8, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/bitter.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(16, 67), Port::INPUT, module, Bitter::INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(80, 67), Port::OUTPUT, module, Bitter::OUTPUT));

    addParam(ParamWidget::create<NKK>(Vec(12, 133), module, Bitter::BIT_1, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<NKK>(Vec(12, 183), module, Bitter::BIT_2, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<NKK>(Vec(12, 233), module, Bitter::BIT_3, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<NKK>(Vec(12, 283), module, Bitter::BIT_4, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<NKK>(Vec(76, 133), module, Bitter::BIT_5, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<NKK>(Vec(76, 183), module, Bitter::BIT_6, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<NKK>(Vec(76, 233), module, Bitter::BIT_7, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<NKK>(Vec(76, 283), module, Bitter::BIT_8, 0.0, 2.0, 2.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Bitter) {
   Model *modelBitter = Model::create<Bitter, BitterWidget>("Sonus Modular", "Bitter", "Bitter | Bit Manipulator", DISTORTION_TAG, EFFECT_TAG);
   return modelBitter;
}
