#include "FrozenWasteland.hpp"
#include "dsp/decimator.hpp"
#include "dsp/digital.hpp"
#include "StateVariableFilter.h"

using namespace std;

#define BANDS 4
#define FREQUENCIES 3
#define numFilters 6

namespace rack_plugin_FrozenWasteland {

struct DamianLillard : Module {
	typedef float T;

	enum ParamIds {
		FREQ_1_CUTOFF_PARAM,
		FREQ_2_CUTOFF_PARAM,
		FREQ_3_CUTOFF_PARAM,
		FREQ_1_CV_ATTENUVERTER_PARAM,
		FREQ_2_CV_ATTENUVERTER_PARAM,
		FREQ_3_CV_ATTENUVERTER_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_IN,
		FREQ_1_CUTOFF_INPUT,		
		FREQ_2_CUTOFF_INPUT,		
		FREQ_3_CUTOFF_INPUT,		
		BAND_1_RETURN_INPUT,		
		BAND_2_RETURN_INPUT,
		BAND_3_RETURN_INPUT,
		BAND_4_RETURN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		BAND_1_OUTPUT,
		BAND_2_OUTPUT,
		BAND_3_OUTPUT,
		BAND_4_OUTPUT,
		MIX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEARN_LIGHT,
		NUM_LIGHTS
	};
	float freq[FREQUENCIES] = {0};
	float lastFreq[FREQUENCIES] = {0};
	float output[BANDS] = {0};

    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];


	int bandOffset = 0;

	DamianLillard() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		filterParams[0].setMode(StateVariableFilterParams<T>::Mode::LowPass);
		filterParams[1].setMode(StateVariableFilterParams<T>::Mode::HiPass);
		filterParams[2].setMode(StateVariableFilterParams<T>::Mode::LowPass);
		filterParams[3].setMode(StateVariableFilterParams<T>::Mode::HiPass);
		filterParams[4].setMode(StateVariableFilterParams<T>::Mode::LowPass);
		filterParams[5].setMode(StateVariableFilterParams<T>::Mode::HiPass);

		for (int i = 0; i < numFilters; ++i) {
	        filterParams[i].setQ(5); 	
	        filterParams[i].setFreq(T(.1));
	    }
	}

	void step() override;
};

void DamianLillard::step() {
	
	float signalIn = inputs[SIGNAL_IN].value/5;
	float out = 0.0;

	const float minCutoff = 15.0;
	const float maxCutoff = 8400.0;
	
	for (int i=0; i<FREQUENCIES;i++) {
		float cutoffExp = params[FREQ_1_CUTOFF_PARAM+i].value + inputs[FREQ_1_CUTOFF_INPUT+i].value * params[FREQ_1_CV_ATTENUVERTER_PARAM+i].value / 10.0f; //I'm reducing range of CV to make it more useful
		cutoffExp = clamp(cutoffExp, 0.0f, 1.0f);
		freq[i] = minCutoff * powf(maxCutoff / minCutoff, cutoffExp);

		//Prevent band overlap
		if(i>0 && freq[i] < lastFreq[i-1]) {
			freq[i] = lastFreq[i-1]+1;
		}
		if(i<FREQUENCIES-1 && freq[i] > lastFreq[i+1]) {
			freq[i] = lastFreq[i+1]-1;
		}

		if(freq[i] != lastFreq[i]) {
			float Fc = freq[i] / engineGetSampleRate();
			filterParams[i*2].setFreq(T(Fc));
			filterParams[i*2 + 1].setFreq(T(Fc));
			lastFreq[i] = freq[i];
		}
	}

	output[0] = StateVariableFilter<T>::run(signalIn, filterStates[0], filterParams[0]) * 5;
	output[1] = StateVariableFilter<T>::run(StateVariableFilter<T>::run(signalIn, filterStates[1], filterParams[1]), filterStates[2], filterParams[2]) * 5;
	output[2] = StateVariableFilter<T>::run(StateVariableFilter<T>::run(signalIn, filterStates[3], filterParams[3]), filterStates[4], filterParams[4]) * 5;
	output[3] = StateVariableFilter<T>::run(signalIn, filterStates[5], filterParams[5]) * 5;

	for(int i=0; i<BANDS; i++) {		
		outputs[BAND_1_OUTPUT+i].value = output[i];

		if(inputs[BAND_1_RETURN_INPUT+i].active) {
			out += inputs[BAND_1_RETURN_INPUT+i].value;
		} else {
			out += output[i];
		}
	}

	outputs[MIX_OUTPUT].value = out / 2.0; 
	
}


struct DamianLillardBandDisplay : TransparentWidget {
	DamianLillard *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	DamianLillardBandDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/01 Digit.ttf"));
	}

	void drawFrequency(NVGcontext *vg, Vec pos, float cutoffFrequency) {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), " % 4.0f", cutoffFrequency);
		nvgText(vg, pos.x + 8, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		for(int i=0;i<FREQUENCIES;i++) {
			drawFrequency(vg, Vec(i * 46.0, box.size.y - 75), module->freq[i]);
		}
	}
};

struct DamianLillardWidget : ModuleWidget {
	DamianLillardWidget(DamianLillard *module);
};

DamianLillardWidget::DamianLillardWidget(DamianLillard *module) : ModuleWidget(module) {
	box.size = Vec(15*11, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DamianLillard.svg")));
		addChild(panel);
	}
	
	{
		DamianLillardBandDisplay *offsetDisplay = new DamianLillardBandDisplay();
		offsetDisplay->module = module;
		offsetDisplay->box.pos = Vec(15, 10);
		offsetDisplay->box.size = Vec(box.size.x, 140);
		addChild(offsetDisplay);
	}

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(15, 84), module, DamianLillard::FREQ_1_CUTOFF_PARAM, 0, 1.0, .25));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(66, 84), module, DamianLillard::FREQ_2_CUTOFF_PARAM, 0, 1.0, .5));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(117, 84), module, DamianLillard::FREQ_3_CUTOFF_PARAM, 0, 1.0, .75));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(19, 146), module, DamianLillard::FREQ_1_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(70, 146), module, DamianLillard::FREQ_2_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(121, 146), module, DamianLillard::FREQ_3_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));



	addInput(Port::create<PJ301MPort>(Vec(18, 117), Port::INPUT, module, DamianLillard::FREQ_1_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(69, 117), Port::INPUT, module, DamianLillard::FREQ_2_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(120, 117), Port::INPUT, module, DamianLillard::FREQ_3_CUTOFF_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(10, 317), Port::INPUT, module, DamianLillard::SIGNAL_IN));


	addInput(Port::create<PJ301MPort>(Vec(10, 255), Port::INPUT, module, DamianLillard::BAND_1_RETURN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 255), Port::INPUT, module, DamianLillard::BAND_2_RETURN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(90, 255), Port::INPUT, module, DamianLillard::BAND_3_RETURN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(130, 255), Port::INPUT, module, DamianLillard::BAND_4_RETURN_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(10, 215), Port::OUTPUT, module, DamianLillard::BAND_1_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(50, 215), Port::OUTPUT, module, DamianLillard::BAND_2_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(90, 215), Port::OUTPUT, module, DamianLillard::BAND_3_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(130, 215), Port::OUTPUT, module, DamianLillard::BAND_4_OUTPUT));

	addOutput(Port::create<PJ301MPort>(Vec(90, 317), Port::OUTPUT, module, DamianLillard::MIX_OUTPUT));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, DamianLillard) {
   Model *modelDamianLillard = Model::create<DamianLillard, DamianLillardWidget>("Frozen Wasteland", "DamianLillard", "Damian Lillard", FILTER_TAG);
   return modelDamianLillard;
}
