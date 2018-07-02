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

struct Chainsaw : Module
{
    enum ParamIds
    {
        OCTAVE,
        OSC1_SHAPE,
        OSC1_TUNE,
        OSC2_SHAPE,
        OSC2_TUNE,
        OSC3_SHAPE,
        OSC3_TUNE,
        ALIGN,
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH,
        CV_OSC1_SHAPE,
        CV_OSC1_TUNE,
        CV_OSC2_SHAPE,
        CV_OSC2_TUNE,
        CV_OSC3_SHAPE,
        CV_OSC3_TUNE,
        CV_ALIGN,
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

    Chainsaw() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    float osc1_phase = 0.0;
    float osc2_phase = 0.0;
    float osc3_phase = 0.0;

    float osc_wave1_mix[3] = {0.0, 0.0, 0.0};
    float osc_wave2_mix[3] = {0.0, 0.0, 0.0};
    float osc_wave3_mix[3] = {0.0, 0.0, 0.0};
    float osc[3] = {0.0, 0.0, 0.0};
    float osc_freq[3] = {0.0, 0.0, 0.0};
    float osc_phase[3] = {0.0, 0.0, 0.0};
};


void Chainsaw::step()
{
    float pitch = params[OCTAVE].value;
    pitch += inputs[PITCH].value;
    pitch = clamp(pitch, -4.0, 4.0);

    for (unsigned int f = 0; f < 3; f++)
    {
        osc_freq[f] = 261.626 * powf(2.0, pitch + (inputs[(f * 2) + 2].value / 5.0) + params[(f * 2) + 2].value);
    }

    for (unsigned int p = 0; p < 3; p++)
    {
        osc_phase[p] += 2.0 * (osc_freq[p] / engineGetSampleRate());
        if (osc_phase[p] >= 1.0)
        {
            osc_phase[p] -= 2.0;
        }
    }

    for (unsigned int o = 0; o < 3; o++)
    {
        osc_wave1_mix[o] = (-1.0 * params[(o * 2) + 1].value) + (inputs[(o * 2) + 1].value / -5.0);
        osc_wave1_mix[o] = clamp(osc_wave1_mix[o], 0.0, 1.0);

        osc_wave3_mix[o] = params[(o * 2) + 1].value + (inputs[(o * 2) + 1].value / 5.0);
        osc_wave3_mix[o] = clamp(osc_wave3_mix[o], 0.0, 1.0);

        osc_wave2_mix[o] = (2.0 - osc_wave1_mix[o] - osc_wave3_mix[o]) * 0.5;
    }

    for (unsigned int i = 0; i < 3; i++)
    {
        osc[i] = ((osc_wave1_mix[i] * powf(osc_phase[i], 4.0)) - (0.2 * osc_wave1_mix[i])) + (osc_wave2_mix[i] * osc_phase[i]) + (osc_wave3_mix[i] * 0.8 * cbrt(cbrt(osc_phase[i])));
    }

    outputs[OUTPUT].value = (osc[0] + osc[1] + osc[2]) * (5.0 / 3.0);

    if (params[ALIGN].value != 0.0 || inputs[CV_ALIGN].value != 0)
    {
        osc_phase[1] = osc_phase[0];
        osc_phase[2] = osc_phase[0];
    }
}

struct ChainsawWidget : ModuleWidget
{
    ChainsawWidget(Chainsaw *module);
};

ChainsawWidget::ChainsawWidget(Chainsaw *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 12, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/chainsaw.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 67), Port::INPUT, module, Chainsaw::PITCH));
    addInput(Port::create<PJ301MPort>(Vec(14, 132), Port::INPUT, module, Chainsaw::CV_OSC1_SHAPE));
    addInput(Port::create<PJ301MPort>(Vec(143, 132), Port::INPUT, module, Chainsaw::CV_OSC1_TUNE));
    addInput(Port::create<PJ301MPort>(Vec(14, 197), Port::INPUT, module, Chainsaw::CV_OSC2_SHAPE));
    addInput(Port::create<PJ301MPort>(Vec(143, 197), Port::INPUT, module, Chainsaw::CV_OSC2_TUNE));
    addInput(Port::create<PJ301MPort>(Vec(14, 262), Port::INPUT, module, Chainsaw::CV_OSC3_SHAPE));
    addInput(Port::create<PJ301MPort>(Vec(143, 262), Port::INPUT, module, Chainsaw::CV_OSC3_TUNE));
    addInput(Port::create<PJ301MPort>(Vec(100, 324), Port::INPUT, module, Chainsaw::CV_ALIGN));

    addOutput(Port::create<PJ301MPort>(Vec(143, 67), Port::OUTPUT, module, Chainsaw::OUTPUT));

    addParam(ParamWidget::create<SonusKnob>(Vec(72, 65), module, Chainsaw::OCTAVE, -3.0, 3.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(50, 128), module, Chainsaw::OSC1_SHAPE, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(94, 128), module, Chainsaw::OSC1_TUNE, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(50, 193), module, Chainsaw::OSC2_SHAPE, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(94, 193), module, Chainsaw::OSC2_TUNE, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(50, 258), module, Chainsaw::OSC3_SHAPE, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(94, 258), module, Chainsaw::OSC3_TUNE, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<CKD6>(Vec(67,322), module, Chainsaw::ALIGN, 0.0, 1.0, 0.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Chainsaw) {
   Model *modelChainsaw = Model::create<Chainsaw, ChainsawWidget>("Sonus Modular", "Chainsaw", "Chainsaw | Fat Sawish Osc", OSCILLATOR_TAG);
   return modelChainsaw;
}

 
