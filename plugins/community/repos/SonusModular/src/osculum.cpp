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

struct Osculum : Module
{
    enum ParamIds
    {
        OCTAVE,
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH,
        NUM_INPUTS
    };
    enum OutputIds
    {
        WAVE1_OUT,
        WAVE2_OUT,
        WAVE3_OUT,
        WAVE4_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Osculum() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        srand(time(0));

        for (int s = 0; s < 8; s++)
        {
            arand[s] = rand() / (float)RAND_MAX;
        }
    }
    void step() override;

    float phase = 0.0;
    float arand[8];
};


void Osculum::step()
{
    float pitch = params[OCTAVE].value;
    pitch += inputs[PITCH].value;
    pitch = clamp(pitch, -4.0, 4.0);
    float freq = 261.626 * powf(2.0, pitch);

    phase += freq / engineGetSampleRate();
    if (phase >= 1.0)
    {
        phase -= 1.0;
    }

    float wave1 = cosf(powf(M_E, sinf(2.0 * M_PI * phase)));
    float wave2 = 0.45 + (2.0 * phase) * sinf(2.0 * M_PI * phase);
    if (fabs(wave2) > 1.0)
    {
        wave2 = 1.0 * copysign(1.0, wave2);
    }
    float wave3 = cosf(cosh(powf(M_E, sinf(2.0 * M_PI * phase))));
    float wave4 = arand[(unsigned int)floorf(phase * 8.0)] * copysign(1.0, phase - 0.5);

    outputs[WAVE1_OUT].value = wave1 * 5.0;
    outputs[WAVE2_OUT].value = wave2 * 5.0;
    outputs[WAVE3_OUT].value = wave3 * 5.0;
    outputs[WAVE4_OUT].value = wave4 * 5.0;
}

struct OsculumWidget : ModuleWidget
{
    OsculumWidget(Osculum *module);
};

OsculumWidget::OsculumWidget(Osculum *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 6, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/osculum.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(33, 67), Port::INPUT, module, Osculum::PITCH));
    addOutput(Port::create<PJ301MPort>(Vec(14, 132), Port::OUTPUT, module, Osculum::WAVE1_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(52, 132), Port::OUTPUT, module, Osculum::WAVE2_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(14, 197), Port::OUTPUT, module, Osculum::WAVE3_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(52, 197), Port::OUTPUT, module, Osculum::WAVE4_OUT));

    addParam(ParamWidget::create<SonusBigKnob>(Vec(18, 275), module, Osculum::OCTAVE, -3.0, 3.0, 0.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Osculum) {
   Model *modelOsculum = Model::create<Osculum, OsculumWidget>("Sonus Modular", "Osculum", "Osculum | Unusual Oscillator", OSCILLATOR_TAG);
   return modelOsculum;
}
