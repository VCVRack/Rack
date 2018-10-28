#include "dBiz.hpp"

namespace rack_plugin_dBiz {

struct Remix : Module {
    enum ParamIds
    {
        SCAN_PARAM,
        CV_SCAN_PARAM,
        WIDTH_PARAM,
        CV_WIDTH_PARAM,
        LEVEL_PARAM,
        SLOPE_PARAM,
        CV_LEVEL_PARAM,
        CH1_LEVEL_PARAM,
        CH2_LEVEL_PARAM,
        CH3_LEVEL_PARAM,
        CH4_LEVEL_PARAM,
        CH5_LEVEL_PARAM,
        CH6_LEVEL_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CH1_INPUT,
        CH2_INPUT,
        CH3_INPUT,
        CH4_INPUT,
        CH5_INPUT,
        CH6_INPUT,

        SLOPE_INPUT,
        SCAN_INPUT,
        WIDTH_INPUT,
        LEVEL_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        OUT4_OUTPUT,
        OUT5_OUTPUT,
        OUT6_OUTPUT,
        A_OUTPUT,
        B_OUTPUT,
        C_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        CH1_LIGHT,
        CH2_LIGHT,
        CH3_LIGHT,
        CH4_LIGHT,
        CH5_LIGHT,
        CH6_LIGHT,

        NUM_LIGHTS
    };

    float ins[6] = {};
    float outs[6] = {};
    float inMults[6] = {};
    float widthTable[7] = {0.0f, 0.285f, 0.285f, 0.2608f, 0.23523f, 0.2125f, 0.193f};

    Remix() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    int clampInt(const int _in, const int min = 0, const int max = 5)
    {
        if (_in > max)
            return max;
        if (_in < min)
            return min;
        return _in;
    }

    float triShape(float _in)
    {
        _in = _in - round(_in);
        return std::abs(_in + _in);
    }

    float LERP(const float _amountOfA, const float _inA, const float _inB)
    {
        return ((_amountOfA * _inA) + ((1.0f - _amountOfA) * _inB));
    }

};


void Remix::step()
{
    float allInValue = 0.0f;

    for (int i = 0; i < 6; i++)
    {
        if (!inputs[CH1_INPUT + i].active)
            ins[i] = allInValue;
        else
            ins[i] = inputs[CH1_INPUT + i].value*params[CH1_LEVEL_PARAM+i].value;
    }

    int stages = 6;
    const float invStages = 1.0f / stages;
    const float halfStages = stages * 0.5f;
    const float remainInvStages = 1.0f - invStages;

    float widthControl = params[WIDTH_PARAM].value + inputs[WIDTH_INPUT].value*params[CV_WIDTH_PARAM].value;
    widthControl = clamp(widthControl, 0.0f, 5.0f) * 0.2f;
    widthControl = widthControl * widthControl * widthTable[stages];

    float scanControl = params[SCAN_PARAM].value + inputs[SCAN_INPUT].value*params[CV_SCAN_PARAM].value;
    scanControl = clamp(scanControl, 0.0f, 5.0f) * 0.2f;

    float slopeControl = params[SLOPE_PARAM].value + inputs[SLOPE_INPUT].value;
    slopeControl = clamp(slopeControl, 0.0f, 5.0f) * 0.2f;
    

    float scanFactor1 = LERP(widthControl, halfStages, invStages);
    float scanFactor2 = LERP(widthControl, halfStages + remainInvStages, 1.0f);
    float scanFinal = LERP(scanControl, scanFactor2, scanFactor1);

    float invWidth = 1.0f / (LERP(widthControl, float(stages), invStages + invStages));

    float subStage = 0.0f;
    for (int i = 0; i < 6; i++)
    {
        inMults[i] = (scanFinal + subStage) * invWidth;
        subStage = subStage - invStages;
    }

    for (int i = 0; i < 6; i++)
    {
        inMults[i] = clamp(inMults[i], 0.0f, 1.0f);
        inMults[i] = triShape(inMults[i]);
        inMults[i] = clamp(inMults[i], 0.0f, 1.0f);

        const float shaped = (2.0f - inMults[i]) * inMults[i];
        inMults[i] = LERP(slopeControl, shaped, inMults[i]);
    }

    outputs[A_OUTPUT].value = 0.0f;
    outputs[B_OUTPUT].value = 0.0f;
    outputs[C_OUTPUT].value = 0.0f;

    for (int i = 0; i < 6; i++)
    {
        outputs[i].value = ins[i] * inMults[i];

        lights[CH1_LIGHT + i].setBrightnessSmooth(fmaxf(0.0, inMults[i]));

        outputs[B_OUTPUT].value = outputs[B_OUTPUT].value + outputs[i].value;

        if (i <= 1)
        {
            outputs[A_OUTPUT].value = outputs[A_OUTPUT].value + outputs[i].value;
        }
        else if (i >= 4)
        {
            outputs[C_OUTPUT].value = outputs[C_OUTPUT].value + outputs[i].value;
        }

    outputs[A_OUTPUT].value = crossfade(outputs[A_OUTPUT].value * params[LEVEL_PARAM].value, outputs[A_OUTPUT].value * params[LEVEL_PARAM].value*clamp(inputs[LEVEL_INPUT].normalize(10)/10,0.0f,1.0f),params[CV_LEVEL_PARAM].value);
    outputs[B_OUTPUT].value = crossfade(outputs[B_OUTPUT].value * params[LEVEL_PARAM].value, outputs[B_OUTPUT].value * params[LEVEL_PARAM].value*clamp(inputs[LEVEL_INPUT].normalize(10)/10,0.0f,1.0f),params[CV_LEVEL_PARAM].value);
    outputs[C_OUTPUT].value = crossfade(outputs[C_OUTPUT].value * params[LEVEL_PARAM].value, outputs[C_OUTPUT].value * params[LEVEL_PARAM].value*clamp(inputs[LEVEL_INPUT].normalize(10)/10,0.0f,1.0f),params[CV_LEVEL_PARAM].value);
    }
}


struct RemixWidget : ModuleWidget 
{
RemixWidget(Remix *module) : ModuleWidget(module)
{
	box.size = Vec(14 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Remix.svg")));
		addChild(panel);
	}

    int knob = 32;
    int jack = 27;
    int board =20;
    int light = 15;
    float mid = (14*15)/2;
    float midy= 190; 

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


	addParam(ParamWidget::create<RoundRed>(Vec(board, midy+10), module, Remix::SCAN_PARAM,0.0, 5.0,0.0));
    addParam(ParamWidget::create<RoundWhy>(Vec(board, midy+10+knob+10), module, Remix::CV_SCAN_PARAM, 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<RoundRed>(Vec(mid-15, midy+10), module, Remix::WIDTH_PARAM, 0.0, 5.0, 0.0));
    addParam(ParamWidget::create<RoundWhy>(Vec(mid-15, midy+10+knob+10), module, Remix::CV_WIDTH_PARAM, 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<Trimpot>(Vec(mid - 20, 322.5), module, Remix::SLOPE_PARAM, 0.0, 5.0, 0.0));
    addInput(Port::create<PJ301MIPort>(Vec(mid +10 , 320), Port::INPUT, module, Remix::SLOPE_INPUT));

    addParam(ParamWidget::create<RoundRed>(Vec(box.size.x - board - 32.5, midy+10), module, Remix::LEVEL_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x - board - 32.5, midy+10+knob+10), module, Remix::CV_LEVEL_PARAM, 0.0, 1.0, 0.0));

    addOutput(Port::create<PJ301MIPort>(Vec(board + 7.5, 20), Port::OUTPUT, module, Remix::A_OUTPUT));
    addInput(Port::create<PJ301MIPort>(Vec(board+7.5, 320), Port::INPUT, module, Remix::SCAN_INPUT));

    addOutput(Port::create<PJ301MIPort>(Vec(mid-15 + 7.5, 20), Port::OUTPUT, module, Remix::B_OUTPUT));
    addInput(Port::create<PJ301MIPort>(Vec(mid-15+ 7.5, 290), Port::INPUT, module, Remix::WIDTH_INPUT));

    addOutput(Port::create<PJ301MIPort>(Vec(box.size.x-knob-board + 7.5, 20), Port::OUTPUT, module, Remix::C_OUTPUT));
    addInput(Port::create<PJ301MIPort>(Vec(box.size.x-knob-board + 7.5, 320), Port::INPUT, module, Remix::LEVEL_INPUT));


            addInput(Port::create<PJ301MIPort>(Vec(board +5+ jack*0, 70), Port::INPUT, module, Remix::CH1_INPUT));
            addParam(ParamWidget::create<Trimpot>(Vec(board +10+ jack*0,130),module,Remix::CH1_LEVEL_PARAM,0.0,1.0,0.0));
            addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(board+30+light*0,midy),module,Remix::CH1_LIGHT));

            addInput(Port::create<PJ301MIPort>(Vec(board + 5 + jack * 1, 70), Port::INPUT, module, Remix::CH2_INPUT));
            addParam(ParamWidget::create<Trimpot>(Vec(board + 10 + jack * 1, 130), module, Remix::CH2_LEVEL_PARAM, 0.0, 1.0, 0.0));
            addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(board + 30 + light * 1, midy), module, Remix::CH2_LIGHT));

            addInput(Port::create<PJ301MIPort>(Vec(board + 5 + jack * 2, 70), Port::INPUT, module, Remix::CH3_INPUT));
            addParam(ParamWidget::create<Trimpot>(Vec(board + 10 + jack * 2, 130), module, Remix::CH3_LEVEL_PARAM, 0.0, 1.0, 0.0));
            addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(board + 30 + light * 2, midy), module, Remix::CH3_LIGHT));

            

            addInput(Port::create<PJ301MIPort>(Vec(board +10+ jack*3+7.5, 70), Port::INPUT, module, Remix::CH4_INPUT));
            addParam(ParamWidget::create<Trimpot>(Vec(board +10+ jack*3+9,130),module,Remix::CH4_LEVEL_PARAM,0.0,1.0,0.0));
            addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(board+60+light*3,midy),module,Remix::CH4_LIGHT));

            addInput(Port::create<PJ301MIPort>(Vec(board + 10 + jack * 4 + 7.5, 70), Port::INPUT, module, Remix::CH5_INPUT));
            addParam(ParamWidget::create<Trimpot>(Vec(board + 10 + jack * 4 + 9, 130), module, Remix::CH5_LEVEL_PARAM, 0.0, 1.0, 0.0));
            addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(board + 60 + light * 4, midy), module, Remix::CH5_LIGHT));

            addInput(Port::create<PJ301MIPort>(Vec(board + 10 + jack * 5 + 7.5, 70), Port::INPUT, module, Remix::CH6_INPUT));
            addParam(ParamWidget::create<Trimpot>(Vec(board + 10 + jack * 5 +9, 130), module, Remix::CH6_LEVEL_PARAM , 0.0, 1.0, 0.0));
            addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(board + 60 + light * 5, midy), module, Remix::CH6_LIGHT));

            // addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(41, 59), module, Remix::CH_LIGHT));

}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Remix) {
   Model *modelRemix = Model::create<Remix, RemixWidget>("dBiz", "Remix", "Remix", MIXER_TAG);
   return modelRemix;
}
