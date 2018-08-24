#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/ringbuffer.hpp"
#include "dep/freeverb/revmodel.hpp"
#include "dep/filters/smbPitchShift.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct DFUZE : Module {
	enum ParamIds {
		SIZE_PARAM,
		DAMP_PARAM,
		FREEZE_PARAM,
		WIDTH_PARAM,
		DRY_PARAM,
		WET_PARAM,
		SHIMM_PARAM,
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
	DoubleRingBuffer<float,4096> in_L_Buffer, in_R_Buffer;
	DoubleRingBuffer<float,4096> pin_L_Buffer, pin_R_Buffer;
	revmodel revprocessor;
	SchmittTrigger freezeTrigger;
	bool freeze = false;
	float sr = engineGetSampleRate();

	DFUZE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

	}

	void step() override;
};

void DFUZE::step() {
	float outL = 0.0f, outR = 0.0f;
	float wOutL = 0.0f, wOutR = 0.0f;
  float inL = 0.0f, inR = 0.0f;

	// if (sr != engineGetSampleRate()) {
	// 	revprocessor.setsamplerate(engineGetSampleRate());
	// 	in_L_Buffer.clear();
	// 	in_R_Buffer.clear();
	// 	pin_L_Buffer.clear();
	// 	pin_R_Buffer.clear();
	// 	sr = engineGetSampleRate();
	// }

	revprocessor.setdamp(clamp(params[DAMP_PARAM].value+inputs[DAMP_INPUT].value,0.0f,1.0f));
	revprocessor.setroomsize(clamp(params[SIZE_PARAM].value+inputs[SIZE_INPUT].value,0.0f,1.0f));
	revprocessor.setwet(clamp(params[WET_PARAM].value,0.0f,1.0f));
	revprocessor.setdry(clamp(params[DRY_PARAM].value,0.0f,1.0f));
	revprocessor.setwidth(clamp(params[WIDTH_PARAM].value+inputs[WIDTH_INPUT].value,0.0f,1.0f));

	if (freezeTrigger.process(params[FREEZE_PARAM].value + inputs[FREEZE_INPUT].value )) freeze = !freeze;

	revprocessor.setmode(freeze?1.0:0.0);

	inL = inputs[IN_L_INPUT].value;
	inR = inputs[IN_R_INPUT].value;

	if (pin_L_Buffer.size()>0) {
		revprocessor.process(inL, inR, params[SHIMM_PARAM].value * (*pin_L_Buffer.startData()) * 5, clamp(params[SHIMM_PARAM].value+inputs[SHIMM_INPUT].value,0.0f,0.08f) * (*pin_R_Buffer.startData()) * 5, outL, outR, wOutL, wOutR);
		pin_L_Buffer.startIncr(1);
		pin_R_Buffer.startIncr(1);
	}
	else {
		revprocessor.process(inL, inR, 0.0f, 0.0f, outL, outR, wOutL, wOutR);
	}

	in_L_Buffer.push(wOutL/10);
	in_R_Buffer.push(wOutR/10);

	if (in_L_Buffer.full())  {
		smbPitchShift(2.0f, in_L_Buffer.size(), 2048, 4, engineGetSampleRate(), in_L_Buffer.startData(), pin_L_Buffer.endData());
		smbPitchShift(2.0f, in_R_Buffer.size(), 2048, 4, engineGetSampleRate(), in_R_Buffer.startData(), pin_R_Buffer.endData());
		pin_L_Buffer.endIncr(in_L_Buffer.size());
		pin_R_Buffer.endIncr(in_L_Buffer.size());
		in_L_Buffer.clear();
		in_R_Buffer.clear();
	}

	outputs[OUT_L_OUTPUT].value = outL;
	outputs[OUT_R_OUTPUT].value = outR;
}



struct DFUZEWidget : ModuleWidget {
	DFUZEWidget(DFUZE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/DFUZE.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 50), module, DFUZE::SIZE_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 95), module, DFUZE::DAMP_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 140), module, DFUZE::WIDTH_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 185), module, DFUZE::DRY_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(63, 185), module, DFUZE::WET_PARAM, 0.0f, 1.0f, 1.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 230), module, DFUZE::SHIMM_PARAM, 0.0f, 0.08f, 0.0f));
		addParam(ParamWidget::create<BlueCKD6>(Vec(13, 276), module, DFUZE::FREEZE_PARAM, 0.0f, 10.0f, 0.0f));

		addInput(Port::create<PJ301MPort>(Vec(65.0f, 52.0f), Port::INPUT, module, DFUZE::SIZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 97.0f), Port::INPUT, module, DFUZE::DAMP_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 142.0f), Port::INPUT, module, DFUZE::WIDTH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 232.0f), Port::INPUT, module, DFUZE::SHIMM_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 277.0f), Port::INPUT, module, DFUZE::FREEZE_INPUT));

	 	//Changed ports opposite way around
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 319.0f), Port::INPUT, module, DFUZE::IN_L_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 339.0f), Port::INPUT, module, DFUZE::IN_R_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 319.0f),Port::OUTPUT, module, DFUZE::OUT_L_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 339.0f),Port::OUTPUT, module, DFUZE::OUT_R_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, DFUZE) {
   Model *modelDFUZE = Model::create<DFUZE, DFUZEWidget>("Bidoo", "dFUZE", "dFUZE reverberator", REVERB_TAG, EFFECT_TAG);
   return modelDFUZE;
}
