#include "NauModular.hpp"

struct S_h_it : Module{
    enum ParamIds{
        TIME_PARAM,
        MULT_PARAM,
        NUM_PARAMS
    };
    enum InputIds{
        VOLT_INPUT,
        NUM_INPUTS
    };
    enum OutputIds{
        VOLT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds{
        NUM_LIGHTS
    };

    S_h_it() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS){}
    void step() override;

    float curTime = 0.0;
    float lastTime = 0.0;
    float outVoltage = 0.0;
};

void S_h_it::step(){
    float deltaTime = 1.0 / engineGetSampleRate();
    curTime += deltaTime;
    if(!std::isfinite(curTime)) curTime = 0.0;

    float timeElapsed = fabs(curTime-lastTime);
    float timeInterval = params[TIME_PARAM].value / params[MULT_PARAM].value;
    if(timeElapsed >= timeInterval){
        outVoltage = inputs[VOLT_INPUT].value;
        lastTime = curTime;
    }

    outputs[VOLT_OUTPUT].value = outVoltage;
}

struct S_h_itWidget : ModuleWidget{
    S_h_itWidget(S_h_it *module);
};

S_h_itWidget::S_h_itWidget(S_h_it *module) : ModuleWidget(module){
    box.size = Vec(3*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel * panel = new SVGPanel();
	    panel->box.size = box.size;
	    panel->setBackground(SVG::load(assetPlugin(plugin, "res/sh.svg")));
	    addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(box.size.x/2,0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x/2,365)));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(5,150), module, S_h_it::TIME_PARAM, 0.2, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(9,200), module, S_h_it::MULT_PARAM, 1.0, 50.0, 5.0));
    addInput(Port::create<PJ301MPort>(Vec(11,235), Port::INPUT, module, S_h_it::VOLT_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(11,275), Port::OUTPUT, module, S_h_it::VOLT_OUTPUT));
}

Model *modelS_h_it = Model::create<S_h_it, S_h_itWidget>("NauModular", "S&h(it)", "S&h(it)", SAMPLE_AND_HOLD_TAG);
