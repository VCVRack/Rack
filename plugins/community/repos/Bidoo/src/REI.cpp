#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/ringbuffer.hpp"
#include "dep/freeverb/revmodel.hpp"
#include "dep/filters/pitchshifter.h"
#include "dsp/digital.hpp"

#define BUFF_SIZE 1024

using namespace std;

namespace rack_plugin_Bidoo {

struct REI : Module {
	enum ParamIds {
		SIZE_PARAM,
		DAMP_PARAM,
		FREEZE_PARAM,
		WIDTH_PARAM,
		DRY_PARAM,
		WET_PARAM,
		SHIMM_PARAM,
		SHIMMPITCH_PARAM,
		CLIPPING_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_L_INPUT,
		IN_R_INPUT,
		SIZE_INPUT,
		DAMP_INPUT,
		FREEZE_INPUT,
		WIDTH_INPUT,
		SHIMM_INPUT,
		SHIMMPITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	DoubleRingBuffer<float,BUFF_SIZE> in_Buffer;
	DoubleRingBuffer<float,2*BUFF_SIZE> pin_Buffer;
	revmodel revprocessor;
	SchmittTrigger freezeTrigger;
	bool freeze = false;
	float sr = engineGetSampleRate();
	PitchShifter *pShifter = NULL;
	int delay=0;

	REI() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		pShifter = new PitchShifter(BUFF_SIZE, 8, engineGetSampleRate());
      memset((void*)in_Buffer.data, 0, sizeof(in_Buffer.data));
      memset((void*)pin_Buffer.data, 0, sizeof(pin_Buffer.data));
	}

	void step() override;
};

void REI::step() {
	float outL = 0.0f, outR = 0.0f;
	float wOutL = 0.0f, wOutR = 0.0f;
  float inL = 0.0f, inR = 0.0f;

	revprocessor.setdamp(clamp(params[DAMP_PARAM].value+inputs[DAMP_INPUT].value,0.0f,1.0f));
	revprocessor.setroomsize(clamp(params[SIZE_PARAM].value+inputs[SIZE_INPUT].value,0.0f,1.0f));
	revprocessor.setwet(clamp(params[WET_PARAM].value,0.0f,1.0f));
	revprocessor.setdry(clamp(params[DRY_PARAM].value,0.0f,1.0f));
	revprocessor.setwidth(clamp(params[WIDTH_PARAM].value+inputs[WIDTH_INPUT].value,0.0f,1.0f));

	if (freezeTrigger.process(params[FREEZE_PARAM].value + inputs[FREEZE_INPUT].value )) freeze = !freeze;

	revprocessor.setmode(freeze?1.0:0.0);

	inL = inputs[IN_L_INPUT].value;
	inR = inputs[IN_R_INPUT].value;

	float fact = clamp(params[SHIMM_PARAM].value+rescale(inputs[SHIMM_INPUT].value,0.0f,10.0f,0.0f,1.0f),0.0f,1.0f);

	if (pin_Buffer.size()>BUFF_SIZE) {
		revprocessor.process(inL, inR, fact*(*pin_Buffer.startData()), outL, outR, wOutL, wOutR);
		pin_Buffer.startIncr(1);
	}
	else {
		revprocessor.process(inL, inR, 0.0f, outL, outR, wOutL, wOutR);
	}

	if (params[CLIPPING_PARAM].value==1.0f) {
		outL = clamp(outL,-7.0f,7.0f);
		outR= clamp(outR,-7.0f,7.0f);
	}
	else {
		outL = tanh(outL/5.0f)*7.0f;
		outR= tanh(outR/5.0f)*7.0f;
	}

	in_Buffer.push((outL + outR)/2.0f);

	if (in_Buffer.full())  {
		pShifter->process(clamp(params[SHIMMPITCH_PARAM].value + inputs[SHIMMPITCH_INPUT].value,0.5f,4.0f), in_Buffer.startData(), pin_Buffer.endData());
		pin_Buffer.endIncr(in_Buffer.size());
		in_Buffer.clear();
	}

	outputs[OUT_L_OUTPUT].value = outL;
	outputs[OUT_R_OUTPUT].value = outR;
}



struct REIWidget : ModuleWidget {
	REIWidget(REI *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/REI.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 45), module, REI::SIZE_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 85), module, REI::DAMP_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 125), module, REI::WIDTH_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 165), module, REI::DRY_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(63, 165), module, REI::WET_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 205), module, REI::SHIMM_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 245), module, REI::SHIMMPITCH_PARAM, 0.5f, 4.0f, 2.0f));
		addParam(ParamWidget::create<BlueCKD6>(Vec(13, 285), module, REI::FREEZE_PARAM, 0.0f, 10.0f, 0.0f));

		addParam(ParamWidget::create<CKSS>(Vec(75.0f, 15.0f), module, REI::CLIPPING_PARAM, 0.0f, 1.0f, 1.0f));

		addInput(Port::create<PJ301MPort>(Vec(65.0f, 47.0f), Port::INPUT, module, REI::SIZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 87.0f), Port::INPUT, module, REI::DAMP_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 127.0f), Port::INPUT, module, REI::WIDTH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 207.0f), Port::INPUT, module, REI::SHIMM_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 247.0f), Port::INPUT, module, REI::SHIMMPITCH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 287.0f), Port::INPUT, module, REI::FREEZE_INPUT));

	 	//Changed ports opposite way around
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 319.0f), Port::INPUT, module, REI::IN_L_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 339.0f), Port::INPUT, module, REI::IN_R_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 319.0f),Port::OUTPUT, module, REI::OUT_L_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 339.0f),Port::OUTPUT, module, REI::OUT_R_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, REI) {
   Model *modelREI = Model::create<REI, REIWidget>("Bidoo", "REI", "REI reverberator", REVERB_TAG, EFFECT_TAG);
   return modelREI;
}
