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

struct Oktagon : Module
{
    enum ParamIds
    {
        FREQUENCY,
        RANGE,
        NUM_PARAMS
    };
    enum InputIds
    {
        CV_FREQ,
        NUM_INPUTS
    };
    enum OutputIds
    {
        WAVE0_OUT,
        WAVE45_OUT,
        WAVE90_OUT,
        WAVE135_OUT,
        WAVE180_OUT,
        WAVE225_OUT,
        WAVE270_OUT,
        WAVE315_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        FREQ_LIGHT,
        NUM_LIGHTS
    };

    Oktagon() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    float phase = 0.0;
    float light = 0.0;
    bool audio_range = false;
};


void Oktagon::step()
{
    float freq = 0.0;

    if (params[RANGE].value == 0.0)
    {
        audio_range = false;
    }
    else
    {
        audio_range = true;
    }

    if (!audio_range)
    {
        freq = powf(10.0, params[FREQUENCY].value) * powf(10.0, inputs[CV_FREQ].value / 5.0);
    }
    else
    {
        float pitch = params[FREQUENCY].value;
        pitch += inputs[CV_FREQ].value;
        pitch = clamp(pitch, -4.0, 4.0);
        freq = 261.626 * powf(2.0, pitch);
    }

    phase += freq / engineGetSampleRate();
    if (phase >= 1.0)
    {
        phase -= 1.0;
    }

    for (int w = 0; w < NUM_OUTPUTS; w++)
    {
        outputs[w].value = sinf(2.0 * M_PI * (phase + w * 0.125)) * 5.0;
    }

    lights[FREQ_LIGHT].value = (outputs[WAVE0_OUT].value > 0.0) ? 1.0 : 0.0;
}

struct OktagonWidget : ModuleWidget
{
    OktagonWidget(Oktagon *module);
};

OktagonWidget::OktagonWidget(Oktagon *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 8, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/oktagon.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addParam(ParamWidget::create<CKSS>(Vec(6, 65), module, Oktagon::RANGE, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(48, 60), module, Oktagon::FREQUENCY, -2.0, 2.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(88, 66), Port::INPUT, module, Oktagon::CV_FREQ));

    addOutput(Port::create<PJ301MPort>(Vec(10, 132), Port::OUTPUT, module, Oktagon::WAVE0_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(48, 132), Port::OUTPUT, module, Oktagon::WAVE45_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(86, 132), Port::OUTPUT, module, Oktagon::WAVE90_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(10, 187), Port::OUTPUT, module, Oktagon::WAVE135_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(48, 187), Port::OUTPUT, module, Oktagon::WAVE180_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(86, 187), Port::OUTPUT, module, Oktagon::WAVE225_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(10, 242), Port::OUTPUT, module, Oktagon::WAVE270_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(48, 242), Port::OUTPUT, module, Oktagon::WAVE315_OUT));

    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(58, 310), module, Oktagon::FREQ_LIGHT));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Oktagon) {
   Model *modelOktagon = Model::create<Oktagon, OktagonWidget>("Sonus Modular", "Oktagon", "Oktagon | Quad-Quad LFO/OSC", LFO_TAG, QUAD_TAG, OSCILLATOR_TAG);
   return modelOktagon;
}
