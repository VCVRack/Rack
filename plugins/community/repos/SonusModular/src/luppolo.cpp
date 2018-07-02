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

struct Luppolo : Module
{
    enum ParamIds
    {
        CLEAR,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT,
        TRIGGER,
        OVERDUB,
        CV_CLEAR,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        REC_LIGHT,
        PLAY_LIGHT,
        NUM_LIGHTS
    };

    Luppolo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    std::vector<float> loop;
    bool is_recording = false;
    bool master_rec = false;
    bool overdubbing = false;
    unsigned int sample = 0;
    float trig_last_value = 0.0;
    float overdub_last_value = 0.0;
};


void Luppolo::step()
{
    float in = inputs[INPUT].value;
    float out = 0.0;

    if ((inputs[TRIGGER].value != trig_last_value) && (trig_last_value == 0.0))
    {
        if (!is_recording)
        {
            loop.clear();
            sample = 0;
            master_rec = false;
            overdubbing = false;
        }
        else
        {
            master_rec = true;
        }

        is_recording = !is_recording;
    }

    trig_last_value = inputs[TRIGGER].value;

    if ((inputs[OVERDUB].value != overdub_last_value) && (overdub_last_value == 0.0))
    {
        if (!overdubbing && master_rec)
        {
            overdubbing = true;
        }
        else if (overdubbing && master_rec)
        {
            overdubbing = false;
        }
    }

    overdub_last_value = inputs[OVERDUB].value;

    if ((params[CLEAR].value != 0.0) || (inputs[CV_CLEAR].value != 0.0))
    {
        master_rec = false;
        is_recording = false;
        overdubbing = false;
        loop.clear();
        sample = 0;
    }

    if (is_recording)
    {
        out = in;
        loop.push_back(in);
    }
    else
    {
        if (!loop.empty())
        {
            if (overdubbing)
            {
                loop.at(sample) += in;
            }

            out = loop.at(sample);
        }
        else
        {
            out = 0.0;
        }

        if (++sample >= loop.size())
        {
            sample = 0;
        }
    }

    outputs[OUTPUT].value = out;

    if (is_recording || overdubbing)
    {
        lights[REC_LIGHT].value = 1.0;
    }
    else
    {
        lights[REC_LIGHT].value = 0.0;
    }

    if (master_rec)
    {
        lights[PLAY_LIGHT].value = 1.0;
    }
    else
    {
        lights[PLAY_LIGHT].value = 0.0;
    }
}

struct LuppoloWidget : ModuleWidget
{
    LuppoloWidget(Luppolo *module);
};

LuppoloWidget::LuppoloWidget(Luppolo *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 6, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/luppolo.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 67), Port::INPUT, module, Luppolo::INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(52, 67), Port::OUTPUT, module, Luppolo::OUTPUT));
    addInput(Port::create<PJ301MPort>(Vec(33, 155), Port::INPUT, module, Luppolo::TRIGGER));
    addInput(Port::create<PJ301MPort>(Vec(33, 215), Port::INPUT, module, Luppolo::OVERDUB));

    addInput(Port::create<PJ301MPort>(Vec(14,272), Port::INPUT, module, Luppolo::CV_CLEAR));
    addParam(ParamWidget::create<CKD6>(Vec(50,270), module, Luppolo::CLEAR, 0.0, 1.0, 0.0));

    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(22, 127), module, Luppolo::REC_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(61, 127), module, Luppolo::PLAY_LIGHT));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Luppolo) {
   Model *modelLuppolo = Model::create<Luppolo, LuppoloWidget>("Sonus Modular", "Luppolo", "Luppolo | Simple Looper", SAMPLER_TAG);
   return modelLuppolo;
}
