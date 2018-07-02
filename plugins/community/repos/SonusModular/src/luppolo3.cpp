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

struct Luppolo3 : Module
{
    enum ParamIds
    {
        DIRECTION,
        ERASE,
        CLEAR_M,
        CLEAR_S1,
        CLEAR_S2,
        GAIN_M,
        GAIN_S1,
        GAIN_S2,
        TRIGGER_M,
        TRIGGER_S1,
        TRIGGER_S2,
        OVERDUB_M,
        OVERDUB_S1,
        OVERDUB_S2,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        TRIGGER_DIR,
        CV_TRIGGER_M,
        CV_TRIGGER_S1,
        CV_TRIGGER_S2,
        CV_OVERDUB_M,
        CV_OVERDUB_S1,
        CV_OVERDUB_S2,
        CV_ERASE,
        CV_CLEAR_M,
        CV_CLEAR_S1,
        CV_CLEAR_S2,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        OUTPUT_ML,
        OUTPUT_MR,
        OUTPUT_S1L,
        OUTPUT_S1R,
        OUTPUT_S2L,
        OUTPUT_S2R,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        REC_LIGHT_M,
        PLAY_LIGHT_M,
        REC_LIGHT_S1,
        PLAY_LIGHT_S1,
        REC_LIGHT_S2,
        PLAY_LIGHT_S2,
        NUM_LIGHTS
    };

    Luppolo3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    ~Luppolo3()
    {
        for (int c = 0; c < 2; c++)
        {
            master_loop[c].clear();
            slave_loop_1[c].clear();
            slave_loop_2[c].clear();
        }
    }
    void step() override;

    std::deque<float> master_loop[2];
    std::deque<float> slave_loop_1[2];
    std::deque<float> slave_loop_2[2];
    bool is_recording[3] = {false, false, false};
    bool master_rec = false;
    bool overdubbing[3] = {false, false, false};
    int sample = 0;
    float trig_last_value[3][2] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
    float overdub_last_value[3][2] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
    float clear_last_value[3][2] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
    float erase_last_value[2] = {0.0, 0.0};
    float dir_last_value[2] = {0.0, 0.0};
    bool forward = true;
    int loop_size = 0;
};


void Luppolo3::step()
{
    float in_l = inputs[INPUT_L].value;
    float in_r = inputs[INPUT_R].value;
    float out_l = 0.0;
    float out_r = 0.0;

    if (((inputs[TRIGGER_DIR].value != 0.0) && dir_last_value[0] == 0.0) || ((params[DIRECTION].value != 0.0) && (dir_last_value[1] == 0.0)))
    {
        forward = !forward;
    }

    dir_last_value[0] = inputs[TRIGGER_DIR].value;
    dir_last_value[1] = params[DIRECTION].value;

    // Master track
    if (((inputs[CV_TRIGGER_M].value != trig_last_value[0][0]) && (trig_last_value[0][0] == 0.0)) || ((params[TRIGGER_M].value != trig_last_value[0][1]) && (trig_last_value[0][1] == 0.0)))
    {
        if (!is_recording[0])
        {
            for (int c = 0; c < 2; c++)
            {
                master_loop[c].clear();
                slave_loop_1[c].clear();
                slave_loop_2[c].clear();
            }
            sample = 0;
            loop_size = 0;
            master_rec = false;
            overdubbing[0] = false;
        }
        else
        {
            master_rec = true;
        }

        is_recording[0] = !is_recording[0];
    }

    trig_last_value[0][0] = inputs[CV_TRIGGER_M].value;
    trig_last_value[0][1] = params[TRIGGER_M].value;

    if (((inputs[CV_OVERDUB_M].value != overdub_last_value[0][0]) && (overdub_last_value[0][0] == 0.0)) || ((params[OVERDUB_M].value != overdub_last_value[0][1]) && (overdub_last_value[0][1] == 0.0)))
    {
        if (!overdubbing[0] && master_rec)
        {
            overdubbing[0] = true;
        }
        else if (overdubbing && master_rec)
        {
            overdubbing[0] = false;
        }
    }

    overdub_last_value[0][0] = inputs[CV_OVERDUB_M].value;
    overdub_last_value[0][1] = params[OVERDUB_M].value;

    if (((inputs[CV_CLEAR_M].value != 0.0) && clear_last_value[0][0] == 0.0) || ((params[CLEAR_M].value != 0.0) && (clear_last_value[0][1] == 0.0)))
    {
        master_rec = false;
        is_recording[0] = false;
        overdubbing[0] = false;
        for (int c = 0; c < 2; c++)
        {
            master_loop[c].assign(master_loop[c].size(), 0.0);
        }
    }

    clear_last_value[0][0] = inputs[CV_CLEAR_M].value;
    clear_last_value[0][1] = params[CLEAR_M].value;

    if (is_recording[0])
    {
        ++loop_size;

        if (loop_size < INT_MAX)
        {
            master_loop[0].push_back(in_l);
            master_loop[1].push_back(in_r);
            slave_loop_1[0].push_back(0.0);
            slave_loop_1[1].push_back(0.0);
            slave_loop_2[0].push_back(0.0);
            slave_loop_2[1].push_back(0.0);
        }
        else
        {
            --loop_size;
            is_recording[0] = false;
            master_rec = true;
        }
    }
    else
    {
        if (!master_loop[0].empty())
        {
            if (overdubbing[0])
            {
                master_loop[0].at(sample) += in_l;
                master_loop[1].at(sample) += in_r;
            }

            out_l += master_loop[0].at(sample) * params[GAIN_M].value;
            out_r += master_loop[1].at(sample) * params[GAIN_M].value;
        }
        else
        {
            out_l += 0.0;
            out_r += 0.0;
        }
    }

    // Slave tracks
    if (master_rec)
    {
        for (int t = 1; t < 3; t++)
        {

            if (((inputs[CV_TRIGGER_M + t].value != trig_last_value[t][0]) && (trig_last_value[t][0] == 0.0)) || ((params[TRIGGER_M + t].value != trig_last_value[t][1]) && (trig_last_value[t][1] == 0.0)))
            {
                if (!is_recording[t])
                {
                    for (int c = 0; c < 2; c++)
                    {
                        t < 2 ? slave_loop_1[c].assign(slave_loop_1[c].size(), 0.0) : slave_loop_2[c].assign(slave_loop_2[c].size(), 0.0);
                    }
                    overdubbing[t] = false;
                }

                is_recording[t] = !is_recording[t];
            }

            trig_last_value[t][0] = inputs[CV_TRIGGER_M + t].value;
            trig_last_value[t][1] = params[TRIGGER_M + t].value;

            if (((inputs[CV_OVERDUB_M + t].value != overdub_last_value[t][0]) && (overdub_last_value[t][0] == 0.0)) || ((params[OVERDUB_M + t].value != overdub_last_value[t][1]) && (overdub_last_value[t][1] == 0.0)))
            {
                if (!overdubbing[t] && master_rec)
                {
                    overdubbing[t] = true;
                }
                else if (overdubbing && master_rec)
                {
                    overdubbing[t] = false;
                }
            }

            overdub_last_value[t][0] = inputs[CV_OVERDUB_M + t].value;
            overdub_last_value[t][1] = params[OVERDUB_M + t].value;


            if (((inputs[CV_CLEAR_M + t].value != 0.0) && clear_last_value[t][0] == 0.0) || ((params[CLEAR_M].value != 0.0) && (clear_last_value[t][1] == 0.0)))
            {
                is_recording[t] = false;
                overdubbing[t] = false;
                for (int c = 0; c < 2; c++)
                {
                    t < 2 ? slave_loop_1[c].assign(slave_loop_1[c].size(), 0.0) : slave_loop_2[c].assign(slave_loop_2[c].size(), 0.0);
                }
                sample = 0;
            }

            clear_last_value[t][0] = inputs[CV_CLEAR_M + t].value;
            clear_last_value[t][1] = params[CLEAR_M + t].value;

            if (is_recording[t])
            {
                t < 2 ? slave_loop_1[0].at(sample) = in_l : slave_loop_2[0].at(sample) = in_l;
                t < 2 ? slave_loop_1[1].at(sample) = in_r : slave_loop_2[1].at(sample) = in_r;
            }
            else
            {
                if (!master_loop[0].empty())
                {
                    if (overdubbing[t])
                    {
                        t < 2 ? slave_loop_1[0].at(sample) += in_l : slave_loop_2[0].at(sample) += in_l;
                        t < 2 ? slave_loop_1[1].at(sample) += in_r : slave_loop_2[1].at(sample) += in_r;
                    }

                    t < 2 ? out_l += slave_loop_1[0].at(sample) * params[GAIN_S1].value : out_l += slave_loop_2[0].at(sample) * params[GAIN_S2].value;
                    t < 2 ? out_r += slave_loop_1[1].at(sample) * params[GAIN_S1].value : out_r += slave_loop_2[1].at(sample) * params[GAIN_S2].value;
                }
                else
                {
                    out_l += 0.0;
                    out_r += 0.0;
                }
            }
        }
    }

    (is_recording[0] || overdubbing[0]) ? lights[REC_LIGHT_M].value = 1.0 : lights[REC_LIGHT_M].value = 0.0;
    (is_recording[1] || overdubbing[1]) ? lights[REC_LIGHT_S1].value = 1.0 : lights[REC_LIGHT_S1].value = 0.0;
    (is_recording[2] || overdubbing[2]) ? lights[REC_LIGHT_S2].value = 1.0 : lights[REC_LIGHT_S2].value = 0.0;
    master_rec ? lights[PLAY_LIGHT_M].value = 1.0 : lights[PLAY_LIGHT_M].value = 0.0;
    master_rec ? lights[PLAY_LIGHT_S1].value = 1.0 : lights[PLAY_LIGHT_S1].value = 0.0;
    master_rec ? lights[PLAY_LIGHT_S2].value = 1.0 : lights[PLAY_LIGHT_S2].value = 0.0;

    if (is_recording[0] || is_recording[1] || is_recording[2])
    {
        out_l += in_l;
        out_r += in_r;
    }

    outputs[OUTPUT_L].value = out_l;
    outputs[OUTPUT_R].value = out_r;

    if (master_rec)
    {
        outputs[OUTPUT_ML].value = master_loop[0].at(sample) * params[GAIN_M].value;
        outputs[OUTPUT_MR].value = master_loop[1].at(sample) * params[GAIN_M].value;
        outputs[OUTPUT_S1L].value = slave_loop_1[0].at(sample) * params[GAIN_S1].value;
        outputs[OUTPUT_S1R].value = slave_loop_1[1].at(sample) * params[GAIN_S1].value;
        outputs[OUTPUT_S2L].value = slave_loop_2[0].at(sample) * params[GAIN_S2].value;
        outputs[OUTPUT_S2R].value = slave_loop_2[1].at(sample) * params[GAIN_S2].value;
    }

    if (!is_recording[0])
    {
        if (forward)
        {
            if (++sample >= (int)master_loop[0].size())
            {
                sample = 0;
            }
        }
        else
        {
            if (--sample < 0)
            {
                sample = master_loop[0].size() - 1;
            }
        }
    }

    if (((inputs[CV_ERASE].value != 0.0) && erase_last_value[0] == 0.0) || ((params[ERASE].value != 0.0) && (erase_last_value[1] == 0.0)))
    {
        master_rec = false;
        for (int t = 0; t < 3; t++)
        {
            is_recording[t] = false;
            overdubbing[t] = false;
        }
        for (int c = 0; c < 2; c++)
        {
            master_loop[c].clear();
            slave_loop_1[c].clear();
            slave_loop_2[c].clear();
        }
        sample = 0;
        loop_size = 0;
    }

    erase_last_value[0] = inputs[CV_ERASE].value;
    erase_last_value[1] = params[ERASE].value;
}

struct Luppolo3Widget : ModuleWidget
{
    Luppolo3Widget(Luppolo3 *module);
};

Luppolo3Widget::Luppolo3Widget(Luppolo3 *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 30, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/luppolo3.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addInput(Port::create<PJ301MPort>(Vec(14, 92), Port::INPUT, module, Luppolo3::INPUT_L));
    addInput(Port::create<PJ301MPort>(Vec(52, 92), Port::INPUT, module, Luppolo3::INPUT_R));
    addInput(Port::create<PJ301MPort>(Vec(14, 215), Port::INPUT, module, Luppolo3::TRIGGER_DIR));
    addParam(ParamWidget::create<CKD6>(Vec(50, 213), module, Luppolo3::DIRECTION, 0.0, 1.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(14,272), Port::INPUT, module, Luppolo3::CV_ERASE));
    addParam(ParamWidget::create<CKD6>(Vec(50,270), module, Luppolo3::ERASE, 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<SonusKnob>(Vec(117, 85), module, Luppolo3::GAIN_M, 0.0, 2.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(104, 155), Port::INPUT, module, Luppolo3::CV_TRIGGER_M));
    addParam(ParamWidget::create<CKD6>(Vec(140, 153), module, Luppolo3::TRIGGER_M, 0.0, 1.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(104, 215), Port::INPUT, module, Luppolo3::CV_OVERDUB_M));
    addParam(ParamWidget::create<CKD6>(Vec(140, 213), module, Luppolo3::OVERDUB_M, 0.0, 1.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(104,272), Port::INPUT, module, Luppolo3::CV_CLEAR_M));
    addParam(ParamWidget::create<CKD6>(Vec(140,270), module, Luppolo3::CLEAR_M, 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<SonusKnob>(Vec(207, 85), module, Luppolo3::GAIN_S1, 0.0, 2.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(194, 155), Port::INPUT, module, Luppolo3::CV_TRIGGER_S1));
    addParam(ParamWidget::create<CKD6>(Vec(230, 153), module, Luppolo3::TRIGGER_S1, 0.0, 1.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(194, 215), Port::INPUT, module, Luppolo3::CV_OVERDUB_S1));
    addParam(ParamWidget::create<CKD6>(Vec(230, 213), module, Luppolo3::OVERDUB_S1, 0.0, 1.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(194, 272), Port::INPUT, module, Luppolo3::CV_CLEAR_S1));
    addParam(ParamWidget::create<CKD6>(Vec(230,270), module, Luppolo3::CLEAR_S1, 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<SonusKnob>(Vec(297, 85), module, Luppolo3::GAIN_S2, 0.0, 2.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(284, 155), Port::INPUT, module, Luppolo3::CV_TRIGGER_S2));
    addParam(ParamWidget::create<CKD6>(Vec(320, 153), module, Luppolo3::TRIGGER_S2, 0.0, 1.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(284, 215), Port::INPUT, module, Luppolo3::CV_OVERDUB_S2));
    addParam(ParamWidget::create<CKD6>(Vec(320, 213), module, Luppolo3::OVERDUB_S2, 0.0, 1.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(284,272), Port::INPUT, module, Luppolo3::CV_CLEAR_S2));
    addParam(ParamWidget::create<CKD6>(Vec(320,270), module, Luppolo3::CLEAR_S2, 0.0, 1.0, 0.0));

    addOutput(Port::create<PJ301MPort>(Vec(374, 92), Port::OUTPUT, module, Luppolo3::OUTPUT_L));
    addOutput(Port::create<PJ301MPort>(Vec(412, 92), Port::OUTPUT, module, Luppolo3::OUTPUT_R));
    addOutput(Port::create<PJ301MPort>(Vec(374, 175), Port::OUTPUT, module, Luppolo3::OUTPUT_ML));
    addOutput(Port::create<PJ301MPort>(Vec(412, 175), Port::OUTPUT, module, Luppolo3::OUTPUT_MR));
    addOutput(Port::create<PJ301MPort>(Vec(374, 230), Port::OUTPUT, module, Luppolo3::OUTPUT_S1L));
    addOutput(Port::create<PJ301MPort>(Vec(412, 230), Port::OUTPUT, module, Luppolo3::OUTPUT_S1R));
    addOutput(Port::create<PJ301MPort>(Vec(374, 286), Port::OUTPUT, module, Luppolo3::OUTPUT_S2L));
    addOutput(Port::create<PJ301MPort>(Vec(412, 286), Port::OUTPUT, module, Luppolo3::OUTPUT_S2R));

    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(113, 65), module, Luppolo3::REC_LIGHT_M));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(148, 65), module, Luppolo3::PLAY_LIGHT_M));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(203, 65), module, Luppolo3::REC_LIGHT_S1));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(238, 65), module, Luppolo3::PLAY_LIGHT_S1));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(293, 65), module, Luppolo3::REC_LIGHT_S2));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(328, 65), module, Luppolo3::PLAY_LIGHT_S2));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Luppolo3) {
   Model *modelLuppolo3 = Model::create<Luppolo3, Luppolo3Widget>("Sonus Modular", "Luppolo3", "Luppolo3 | Loop Station", SAMPLER_TAG);
   return modelLuppolo3;
}
