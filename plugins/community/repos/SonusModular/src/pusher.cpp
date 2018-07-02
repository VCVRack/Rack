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

struct Pusher : Module
{
    enum ParamIds
    {
        PUSH_1,
        CV_1,
        PUSH_2,
        CV_2,
        PUSH_3,
        CV_3,
        PUSH_4,
        CV_4,
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUT_1,
        OUT_2,
        OUT_3,
        OUT_4,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Pusher() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void Pusher::step()
{
    outputs[OUT_1].value = params[CV_1].value * params[PUSH_1].value;
    outputs[OUT_2].value = params[CV_2].value * params[PUSH_2].value;
    outputs[OUT_3].value = params[CV_3].value * params[PUSH_3].value;
    outputs[OUT_4].value = params[CV_4].value * params[PUSH_4].value;
}

struct PusherWidget : ModuleWidget
{
    PusherWidget(Pusher *module);
};

PusherWidget::PusherWidget(Pusher *module) : ModuleWidget(module)
{
    box.size = Vec(15 * 8, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/pusher.svg")));
        addChild(panel);
    }

    addChild(Widget::create<SonusScrew>(Vec(0, 0)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<SonusScrew>(Vec(0, 365)));
    addChild(Widget::create<SonusScrew>(Vec(box.size.x - 15, 365)));

    addOutput(Port::create<PJ301MPort>(Vec(18, 157), Port::OUTPUT, module, Pusher::OUT_1));
    addOutput(Port::create<PJ301MPort>(Vec(18, 292), Port::OUTPUT, module, Pusher::OUT_2));
    addOutput(Port::create<PJ301MPort>(Vec(78, 157), Port::OUTPUT, module, Pusher::OUT_3));
    addOutput(Port::create<PJ301MPort>(Vec(78, 292), Port::OUTPUT, module, Pusher::OUT_4));

    addParam(ParamWidget::create<SonusKnob>(Vec(12, 99), module, Pusher::CV_1, -5.0, 5.0, 5.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(12, 235), module, Pusher::CV_2, -5.0, 5.0, 5.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(72, 99), module, Pusher::CV_3, -5.0, 5.0, 5.0));
    addParam(ParamWidget::create<SonusKnob>(Vec(72, 235), module, Pusher::CV_4, -5.0, 5.0, 5.0));

    addParam(ParamWidget::create<CKD6>(Vec(16,64), module, Pusher::PUSH_1, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<CKD6>(Vec(16,199), module, Pusher::PUSH_2, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<CKD6>(Vec(76,64), module, Pusher::PUSH_3, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<CKD6>(Vec(76,199), module, Pusher::PUSH_4, 0.0, 1.0, 0.0));
}

} // namespace rack_plugin_SonusModular

using namespace rack_plugin_SonusModular;

RACK_PLUGIN_MODEL_INIT(SonusModular, Pusher) {
   Model *modelPusher = Model::create<Pusher, PusherWidget>("Sonus Modular", "Pusher", "Pusher | Buttons Controller", CONTROLLER_TAG, UTILITY_TAG);
   return modelPusher;
}
