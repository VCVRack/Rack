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

struct Addiction : Module
{
    enum ParamIds
    {
        OCTAVE,
        H1_I1,
        H1_I2,
        H1_I3,
        H1_I4,
        H2_I1,
        H2_I2,
        H2_I3,
        H2_I4,
        H3_I1,
        H3_I2,
        H3_I3,
        H3_I4,
        H4_I1,
        H4_I2,
        H4_I3,
        H4_I4,
        NUM_PARAMS
    };
    enum InputIds
    {
        CV_FREQ,
        CV_H1_I1,
        CV_H1_I2,
        CV_H1_I3,
        CV_H1_I4,
        CV_H2_I1,
        CV_H2_I2,
        CV_H2_I3,
        CV_H2_I4,
        CV_H3_I1,
        CV_H3_I2,
        CV_H3_I3,
        CV_H3_I4,
        CV_H4_I1,
        CV_H4_I2,
        CV_H4_I3,
        CV_H4_I4,
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

    Addiction() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    float amp_sum = 0.0f;
    float ramps[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float coeff[16] = {1.0f, 1.2f, 1.25f, 1.5f, 2.0f, 2.4f, 2.5f, 3.0f, 3.0f, 3.6f, 3.75f, 4.5f, 4.0f, 4.8f, 5.0f, 6.0f};
    
};


void Addiction::step()
{
    float pitch = params[OCTAVE].value;
    pitch += inputs[CV_FREQ].value;
    pitch = clamp(pitch, -4.0f, 4.0f);
    float freq = 261.626 * powf(2.0f, pitch);

    float inv_sample_rate = 1.0 / engineGetSampleRate();

    outputs[OUTPUT].value = 0.0;
    amp_sum = 0.0;

    for (unsigned int i = 0; i < 16; i++)
    {
        ramps[i] += freq * inv_sample_rate * coeff[i];
    
        if (ramps[i] > 1.0)
        {
            ramps[i] = -1.0;
        }

        outputs[OUTPUT].value += sin(ramps[i] * M_PI) * (params[i + 1].value + (inputs[i + 1].value * 0.2));
        amp_sum += (params[i + 1].value + (inputs[i + 1].value * 0.2));
    }

    amp_sum > 0.0 ? outputs[OUTPUT].value /= (amp_sum * 0.2) : outputs[OUTPUT].value = 0.0;
}

struct AddictionWidget : ModuleWidget
{
    AddictionWidget(Addiction *module);
    float def_amps[4] = {1.0f, 0.7f, 0.0f, 0.8f};
};

AddictionWidget::AddictionWidget(Addiction *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 24, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/addiction.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addParam(ParamWidget::create<SonusKnob>(Vec(20, 64), module, Addiction::OCTAVE, -3.0, 3.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(25.5, 137), Port::INPUT, module, Addiction::CV_FREQ));

    addOutput(Port::create<PJ301MPort>(Vec(25.5, 277), Port::OUTPUT, module, Addiction::OUTPUT));

    for (unsigned int a = 0; a < Addiction::NUM_PARAMS - 1; a++)
    {
        addParam(ParamWidget::create<SonusKnob>(Vec(150 + (50 * (a % 4)), 64 + (70 * floorf(a * 0.25))), module, a + 1, 0.0, 1.0, def_amps[(int)(a % 4)]));
        addInput(Port::create<PJ301MPort>(Vec(155.5 + (50 * (a % 4)), 102 + (70 * floorf(a * 0.25))), Port::INPUT, module, a + 1));
    }    
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Addiction) {
   Model *modelAddiction = Model::create<Addiction, AddictionWidget>("Sonus Modular", "Addiction", "Addiction | Additive Oscillator", OSCILLATOR_TAG);
   return modelAddiction;
}
