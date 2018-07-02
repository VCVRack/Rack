#include "AS.hpp"


struct AtNuVrTr : Module {
	enum ParamIds {
		ATEN1_PARAM,
		OFFSET1_PARAM,
		ATEN2_PARAM,
		OFFSET2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV_ATEN_1,
		CV_ATEN_2,
		CV_OFFSET_1,
		CV_OFFSET_2,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		OUT1_POS_LIGHT,
		OUT1_NEG_LIGHT,
		OUT2_POS_LIGHT,
		OUT2_NEG_LIGHT,
		NUM_LIGHTS
	};

	AtNuVrTr() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void AtNuVrTr::step() {
	float cv_at1 = 0.0f;
	if(inputs[CV_ATEN_1].active){
		cv_at1 = rescale(inputs[CV_ATEN_1].value, -10.0f,10.0f, -1.0f, 1.0f);
	}
	float cv_off1 = 0.0f;
	if(inputs[CV_OFFSET_1].active){
		cv_off1 = rescale(inputs[CV_OFFSET_1].value, -10.0f,10.0f, -10.0f, 10.0f);
	}
	float atten1 = params[ATEN1_PARAM].value + cv_at1;
	float offset1 = params[OFFSET1_PARAM].value + cv_off1;
	float out1 = inputs[IN1_INPUT].value * atten1 + offset1;

	float cv_at2 = 0.0f;
	if(inputs[CV_ATEN_2].active){
		cv_at2 = rescale(inputs[CV_ATEN_2].value, -10.0f,10.0f, -1.0f, 1.0f);
	}
	float cv_off2 = 0.0f;
	if(inputs[CV_OFFSET_2].active){
		cv_off2 = rescale(inputs[CV_OFFSET_2].value, -10.0f,10.0f, -10.0f, 10.0f);
	}
	float atten2 = params[ATEN2_PARAM].value + cv_at2;
	float offset2 = params[OFFSET2_PARAM].value + cv_off2;
	float out2 = inputs[IN2_INPUT].value * atten2 + offset2;


	out1 = clamp(out1, -10.0f, 10.0f);
	out2 = clamp(out2, -10.0f, 10.0f);

	outputs[OUT1_OUTPUT].value = out1;
	outputs[OUT2_OUTPUT].value = out2;
	lights[OUT1_POS_LIGHT].value = fmaxf(0.0f, out1 / 5.0f);
	lights[OUT1_NEG_LIGHT].value = fmaxf(0.0f, -out1 / 5.0f);
	lights[OUT2_POS_LIGHT].value = fmaxf(0.0f, out2 / 5.0f);
	lights[OUT2_NEG_LIGHT].value = fmaxf(0.0f, -out2 / 5.0f);
}


struct AtNuVrTrWidget : ModuleWidget {
	AtNuVrTrWidget(AtNuVrTr *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/AtNuVrTr.svg")));

	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

const int group_offset = 160;
	//ATTN 1
	addParam(ParamWidget::create<as_KnobBlack>(Vec(34, 45), module, AtNuVrTr::ATEN1_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<as_Knob>(Vec(34, 100), module, AtNuVrTr::OFFSET1_PARAM, -10.0f, 10.0f, 0.0f));

	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(65, 95), module, AtNuVrTr::OUT1_POS_LIGHT));

	addInput(Port::create<as_PJ301MPort>(Vec(4, 51), Port::INPUT, module, AtNuVrTr::CV_ATEN_1));
	addInput(Port::create<as_PJ301MPort>(Vec(4, 106), Port::INPUT, module, AtNuVrTr::CV_OFFSET_1));

	addInput(Port::create<as_PJ301MPort>(Vec(8, 165), Port::INPUT, module, AtNuVrTr::IN1_INPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 165), Port::OUTPUT, module, AtNuVrTr::OUT1_OUTPUT));
	//ATTN 2
	addParam(ParamWidget::create<as_KnobBlack>(Vec(34, 45+group_offset), module, AtNuVrTr::ATEN2_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<as_Knob>(Vec(34, 100+group_offset), module, AtNuVrTr::OFFSET2_PARAM, -10.0f, 10.0f, 0.0f));

	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(65, 95+group_offset), module, AtNuVrTr::OUT2_POS_LIGHT));

	addInput(Port::create<as_PJ301MPort>(Vec(4, 51+group_offset), Port::INPUT, module, AtNuVrTr::CV_ATEN_2));
	addInput(Port::create<as_PJ301MPort>(Vec(4, 106+group_offset), Port::INPUT, module, AtNuVrTr::CV_OFFSET_2));

	addInput(Port::create<as_PJ301MPort>(Vec(8, 165+group_offset), Port::INPUT, module, AtNuVrTr::IN2_INPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 165+group_offset), Port::OUTPUT, module, AtNuVrTr::OUT2_OUTPUT));

	}
};


RACK_PLUGIN_MODEL_INIT(AS, AtNuVrTr) {
   Model *modelAtNuVrTr = Model::create<AtNuVrTr, AtNuVrTrWidget>("AS", "AtNuVrTr", "AtNuVrTr Attenuverter", ATTENUATOR_TAG, DUAL_TAG);
   return modelAtNuVrTr;
}

