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

struct Ringo : Module
{
    enum ParamIds
    {
        SOURCE,
        FREQUENCY,
        SHAPE,
        NUM_PARAMS
    };
    enum InputIds
    {
        CARRIER,
        MODULATOR,
        CV_FREQ,
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

    Ringo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    float phase = 0.0;
    bool audio_modulated = false;
};


void Ringo::step()
{
    float freq = 0.0;
    float sine_out = 0.0;
    float saw_out = 0.0;
    float shape = params[SHAPE].value;
    float carrier = inputs[CARRIER].value / 5.0;
    float modulator = inputs[MODULATOR].value / 5.0;

    if (params[SOURCE].value == 0.0)
    {
        audio_modulated = false;
    }
    else
    {
        audio_modulated = true;
    }

    float pitch = params[FREQUENCY].value;
    pitch += inputs[CV_FREQ].value;
    pitch = clamp(pitch, -4.0, 4.0);
    freq = 440.0 * powf(2.0, pitch);

    phase += freq / engineGetSampleRate();
    if (phase >= 1.0)
    {
        phase -= 1.0;
    }

    sine_out = sinf(2.0 * M_PI * phase);
    saw_out = 2.0 * (phase - 0.5);

    if (audio_modulated)
    {
        outputs[OUTPUT].value = carrier * modulator * 5.0;
    }
    else
    {
        outputs[OUTPUT].value = carrier * (((1.0 - shape) * sine_out) + (shape * saw_out)) * 5.0;
    }
}

struct RingoWidget : ModuleWidget
{
    RingoWidget(Ringo *module);
};

RingoWidget::RingoWidget(Ringo *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 6, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/ringo.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 67), Port::INPUT, module, Ringo::MODULATOR));
    addInput(Port::create<PJ301MPort>(Vec(52, 67), Port::INPUT, module, Ringo::CARRIER));
    addOutput(Port::create<PJ301MPort>(Vec(33, 132), Port::OUTPUT, module, Ringo::OUTPUT));

    addParam(ParamWidget::create<CKSS>(Vec(28, 197), module, Ringo::SOURCE, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(27, 243), module, Ringo::SHAPE, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(48, 293), module, Ringo::FREQUENCY, -2.0, 2.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(14, 300), Port::INPUT, module, Ringo::CV_FREQ));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Ringo) {
   Model *modelRingo = Model::create<Ringo, RingoWidget>("Sonus Modular", "Ringo", "Ringo | Ring Modulator", RING_MODULATOR_TAG, EFFECT_TAG);
   return modelRingo;
}
