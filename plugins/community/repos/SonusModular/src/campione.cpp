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

struct Campione : Module
{
    enum ParamIds
    {
        RECORD,
        PLAY,
        SPEED,
        DIRECTION,
        CLEAR,
        START,
        END,
        LOOP,
        ONE_SHOT,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT,
        CV_RECORD,
        CV_PLAY,
        CV_SPEED,
        CV_DIRECTION,
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

    Campione() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    std::deque<float> sample;
    bool loop = false;
    bool forward = true;
    bool recording = false;
    bool one_shot = true;
    bool playing = false;
    float pos = 0.0;
    unsigned int count = 0;
    float play_last_value[2] = {0.0, 0.0};
    float rec_last_value[2] = {0.0, 0.0};
    float clear_last_value[2] = {0.0, 0.0};
    float dir_last_value[2] = {0.0, 0.0};
    float increase = 0.0;
    int start = 0;
    int end = 0;
};


void Campione::step()
{
    float in = inputs[INPUT].value;
    float out = 0.0;

    if (!sample.empty())
    {
        start = (unsigned int)(floorf(params[START].value * sample.size()));
        end = (unsigned int)(ceilf(params[END].value * sample.size()));

        if (start == (int)sample.size())
        {
            --start;

            if (start < 0)
            {
                start = 0;
            }
        }

        if (end < start)
        {
            end = start;
        }
    }
    else
    {
        start = 0;
        end = 0;
    }

    params[ONE_SHOT].value == 0.0 ? one_shot = false : one_shot = true;
    params[LOOP].value == 0.0 ? loop = false : loop = true;

    increase = (((inputs[CV_SPEED].value + 5.0) / 5.0) + params[SPEED].value) / 2.0;

    if (((inputs[CV_RECORD].value != rec_last_value[0]) && (rec_last_value[0] == 0.0)) || ((params[RECORD].value != rec_last_value[1]) && (rec_last_value[1] == 0.0)))
    {
        if (!recording)
        {
            sample.clear();
            pos = 0;
        }

        recording = !recording;
    }

    rec_last_value[0] = inputs[CV_RECORD].value;
    rec_last_value[1] = params[RECORD].value;

    if (((inputs[CV_DIRECTION].value != dir_last_value[0]) && (dir_last_value[0] == 0.0)) || ((params[DIRECTION].value != dir_last_value[1]) && (dir_last_value[1] == 0.0)))
    {
        forward = !forward;
    }

    dir_last_value[0] = inputs[CV_DIRECTION].value;
    dir_last_value[1] = params[DIRECTION].value;

    if (one_shot)
    {
        if (((inputs[CV_PLAY].value != play_last_value[0]) && (play_last_value[0] == 0.0)) || ((params[PLAY].value != play_last_value[1]) && (play_last_value[1] == 0.0)))
        {
            playing = !playing;
            forward ? pos = (float)start : pos = (float)end - 1.0;
            count = 0;
        }
    }
    else
    {
        if ((inputs[CV_PLAY].value != 0.0) || (params[PLAY].value != 0.0))
        {
            playing = true;
        }
        else
        {
            playing = false;
            forward ? pos = (float)start : pos = (float)end - 1.0;
            count = 0;
        }
    }

    play_last_value[0] = inputs[CV_PLAY].value;
    play_last_value[1] = params[PLAY].value;

    if (count > 0 && !loop)
    {
        playing = false;
        forward ? pos = (float)start : pos = (float)end - 1.0;
    }

    if (recording)
    {
        out = in;
        sample.push_back(in);
    }
    else
    {
        if (!sample.empty() && playing)
        {
            out = sample.at((int)floorf(pos));
        }
        else
        {
            out = 0.0;
        }

        if (forward)
        {
            pos += increase;
            if (pos >= (float)end)
            {
                ++count;
                pos = (float)start;
            }
        }
        else
        {
            pos -= increase;
            if (pos < (float)start)
            {
                ++count;
                pos = (float)end - 1.0;
            }
        }
    }

    outputs[OUTPUT].value = out;

    if (((inputs[CV_CLEAR].value != clear_last_value[0]) && (clear_last_value[0] == 0.0)) || ((params[CLEAR].value != clear_last_value[1]) && (clear_last_value[1] == 0.0)))
    {
        sample.clear();
        pos = 0.0;
    }

    clear_last_value[0] = inputs[CV_CLEAR].value;
    clear_last_value[1] = params[CLEAR].value;

    if (recording)
    {
        lights[REC_LIGHT].value = 1.0;
    }
    else
    {
        lights[REC_LIGHT].value = 0.0;
    }

    if (playing && !recording)
    {
        lights[PLAY_LIGHT].value = 1.0;
    }
    else
    {
        lights[PLAY_LIGHT].value = 0.0;
    }
}

struct CampioneWidget : ModuleWidget
{
    CampioneWidget(Campione *module);
};

CampioneWidget::CampioneWidget(Campione *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 12, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/campione.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 67), Port::INPUT, module, Campione::INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(52, 67), Port::OUTPUT, module, Campione::OUTPUT));

    addInput(Port::create<PJ301MPort>(Vec(14, 152), Port::INPUT, module, Campione::CV_RECORD));
    addParam(ParamWidget::create<CKD6>(Vec(50,150), module, Campione::RECORD, 0.0, 1.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(14, 212), Port::INPUT, module, Campione::CV_PLAY));
    addParam(ParamWidget::create<CKD6>(Vec(50,210), module, Campione::PLAY, 0.0, 1.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(14, 272), Port::INPUT, module, Campione::CV_CLEAR));
    addParam(ParamWidget::create<CKD6>(Vec(50,270), module, Campione::CLEAR, 0.0, 1.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(104, 152), Port::INPUT, module, Campione::CV_DIRECTION));
    addParam(ParamWidget::create<CKD6>(Vec(140,150), module, Campione::DIRECTION, 0.0, 1.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(104, 212), Port::INPUT, module, Campione::CV_SPEED));
    addParam(ParamWidget::create<SonusKnob>(Vec(140, 206), module, Campione::SPEED, 0.0, 2.0, 1.0));

    addParam(ParamWidget::create<CKSS>(Vec(153, 53), module, Campione::ONE_SHOT, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<CKSS>(Vec(153, 85), module, Campione::LOOP, 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<SonusKnob>(Vec(94, 266), module, Campione::START, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(140, 266), module, Campione::END, 0.0, 1.0, 1.0));

    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(71, 127), module, Campione::REC_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(104, 127), module, Campione::PLAY_LIGHT));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Campione) {
   Model *modelCampione = Model::create<Campione, CampioneWidget>("Sonus Modular", "Campione", "Campione | Live Sampler", SAMPLER_TAG);
   return modelCampione;
}
