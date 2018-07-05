#include "NauModular.hpp"

struct Tension : Module{
    enum ParamIds{
	VOLT_PARAM,
	NUM_PARAMS
    };
    enum InputIds{
	NUM_INPUTS
    };
    enum OutputIds{
	VOLT_OUTPUT,
	NUM_OUTPUTS
    };
    enum LightIds{
	BLINK_LIGHT,
	NUM_LIGHTS
    };


    Tension() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS){}
    void step() override;
};

void Tension::step(){
    float v = params[VOLT_PARAM].value;
    outputs[VOLT_OUTPUT].value = v;
    lights[BLINK_LIGHT].value = fabs(v)/12.0;
}

struct TensionWidget : ModuleWidget{
	TensionWidget(Tension *module);
};

TensionWidget::TensionWidget(Tension *module) : ModuleWidget(module){
    box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
	SVGPanel * panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/Tension.svg")));
	addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15,0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30,0)));
    addChild(Widget::create<ScrewSilver>(Vec(15,365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30,365)));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(28, 87), module, Tension::VOLT_PARAM, -12.0, 12.0, 0.0));

    addOutput(Port::create<PJ301MPort>(Vec(33, 275), Port::OUTPUT, module, Tension::VOLT_OUTPUT));

    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(41, 59), module, Tension::BLINK_LIGHT));
}

Model *modelTension = Model::create<Tension, TensionWidget>("NauModular", "Tension", "Tension", FUNCTION_GENERATOR_TAG);
