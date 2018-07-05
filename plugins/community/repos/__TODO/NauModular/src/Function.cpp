#include "NauModular.hpp"

struct Function : Module{
   enum ParamIds {
	A_PARAM,
	NUM_PARAMS
    };
    enum InputIds {
	X_INPUT,
    	NUM_INPUTS
    };
    enum OutputIds{
	ELLIPSE_OUTPUT,
	PARABOLA_OUTPUT,
	HYPERBOLA_OUTPUT,
	NUM_OUTPUTS
    };
    enum LightIds {
    	NUM_LIGHTS
    };

    Function() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS){}

    void step() override;
};

void Function::step(){
    float v = inputs[X_INPUT].value;

    float a = params[A_PARAM].value;

    outputs[ELLIPSE_OUTPUT].value = sqrt((a*a)-(v*v));
    outputs[HYPERBOLA_OUTPUT].value = a/v;
    outputs[PARABOLA_OUTPUT].value = v*v*a;
}

struct FunctionWidget : ModuleWidget{
	FunctionWidget(Function *module);
};

FunctionWidget::FunctionWidget(Function *module) : ModuleWidget(module){
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/Function.svg")));
	addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(50, 87), module, Function::A_PARAM, -5.0, 5.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(15, 87), Port::INPUT, module, Function::X_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(15,150), Port::OUTPUT, module, Function::HYPERBOLA_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(15,220), Port::OUTPUT, module, Function::PARABOLA_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(15,300), Port::OUTPUT, module, Function::ELLIPSE_OUTPUT));
};

Model *modelFunction = Model::create<Function, FunctionWidget>("NauModular", "Function", "Function", FUNCTION_GENERATOR_TAG);
