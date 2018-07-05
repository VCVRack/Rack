#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"
#include "dsp-noise/noise.hpp"
#include "filters/biquad.h"

using namespace frozenwasteland::dsp;

namespace rack_plugin_FrozenWasteland {

struct EverlastingGlottalStopper : Module {
	enum ParamIds {
		FREQUENCY_PARAM,
		TIME_OPEN_PARAM,
		TIME_CLOSED_PARAM,
		BREATHINESS_PARAM,
		FM_CV_ATTENUVERTER_PARAM,
		TIME_OPEN_CV_ATTENUVERTER_PARAM,
		TIME_CLOSED_CV_ATTENUVERTER_PARAM,
		BREATHINESS_CV_ATTENUVERTER_PARAM,
		DEEMPHASIS_FILTER_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,		
		FM_INPUT,		
		TIME_OPEN_INPUT,		
		TIME_CLOSED_INPUT,	
		BREATHINESS_INPUT,	
		NUM_INPUTS
	};
	enum OutputIds {
		VOICE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEARN_LIGHT,
		NUM_LIGHTS
	};

	Biquad* deemphasisFilter;
	GaussianNoiseGenerator _gauss;
	float phase = 0.0;

	EverlastingGlottalStopper() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		deemphasisFilter = new Biquad(bq_type_lowpass, 2000 / engineGetSampleRate(), 1, 0);
	}


	void step() override;
};



float Rosenburg(float timeOpening, float timeOpen, float phase) {
	float out;
	if(phase < timeOpening) {
		out = 0.5f * (1.0f - cosf(M_PI * phase / timeOpening));
	} else if (phase < timeOpen) {	
		out = cosf(M_PI * (phase - timeOpening) / (timeOpen - timeOpening) / 2);
	} else {
		out = 0.0f;
	}

	return out;
}

float HanningWindow(float phase) {
	return 0.5f * (1 - cosf(2 * M_PI * phase));
}

inline float quadraticBipolar(float x) {
	float x2 = x*x;
	return (x >= 0.f) ? x2 : -x2;
}

void EverlastingGlottalStopper::step() {
	

	float pitch = params[FREQUENCY_PARAM].value;	
	float pitchCv = 12.0f * inputs[PITCH_INPUT].value;
	if (inputs[FM_INPUT].active) {
		pitchCv += quadraticBipolar(params[FM_CV_ATTENUVERTER_PARAM].value) * 12.0f * inputs[FM_INPUT].value;
	}

	pitch += pitchCv;
		// Note C4
	float freq = 261.626f * powf(2.0f, pitch / 12.0f);

	//float pitch = params[FREQUENCY_PARAM].value + inputs[FM_INPUT].value * params[FM_CV_ATTENUVERTER_PARAM].value;	
	//float freq = powf(2.0, pitch);

	float timeOpening = clamp(params[TIME_OPEN_PARAM].value + inputs[TIME_OPEN_INPUT].value * params[TIME_OPEN_CV_ATTENUVERTER_PARAM].value,0.01f,1.0f);
	float timeClosed = clamp(params[TIME_CLOSED_PARAM].value + inputs[TIME_CLOSED_INPUT].value * params[TIME_CLOSED_CV_ATTENUVERTER_PARAM].value,0.0f,1.0f);
	float timeOpen = clamp(1.0-timeClosed,timeOpening,1.0);

	float dt = 1.0 / engineGetSampleRate();
	float deltaPhase = fminf(freq * dt, 0.5);
	phase += deltaPhase;
	if (phase >= 1.0) {
		phase -= 1.0;
	}

	float out = Rosenburg(timeOpening,timeOpen,phase);
	float noiseLevel = clamp(params[BREATHINESS_PARAM].value + inputs[BREATHINESS_INPUT].value * params[BREATHINESS_CV_ATTENUVERTER_PARAM].value,0.0f,1.0f);
 	//Noise level follows glottal wave
	// noise = _gauss.next() * out * noiseLevel;
	float noise = _gauss.next() / 5.0 * noiseLevel * HanningWindow(phase);

	out = out + noise;
	if(params[DEEMPHASIS_FILTER_PARAM].value) {
		out = deemphasisFilter->process(out);
	}

	outputs[VOICE_OUTPUT].value = out * 10.0f  - 5.0f;
	
}




struct EverlastingGlottalStopperWidget : ModuleWidget {
	EverlastingGlottalStopperWidget(EverlastingGlottalStopper *module);
};

EverlastingGlottalStopperWidget::EverlastingGlottalStopperWidget(EverlastingGlottalStopper *module) : ModuleWidget(module) {
	box.size = Vec(15*11, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/EverlastingGlottalStopper.svg")));
		addChild(panel);
	}
	
	

	addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(54, 60), module, EverlastingGlottalStopper::FREQUENCY_PARAM, -54.0f, 54.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(15, 215), module, EverlastingGlottalStopper::TIME_OPEN_PARAM, 0.01f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(68, 215), module, EverlastingGlottalStopper::TIME_CLOSED_PARAM, 0.0f, 0.9f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(120, 215), module, EverlastingGlottalStopper::BREATHINESS_PARAM, 0.0f, 0.9f, 0.0f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(108, 162), module, EverlastingGlottalStopper::FM_CV_ATTENUVERTER_PARAM, 0.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(17, 275), module, EverlastingGlottalStopper::TIME_OPEN_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(70, 275), module, EverlastingGlottalStopper::TIME_CLOSED_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(123, 275), module, EverlastingGlottalStopper::BREATHINESS_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	//addParam(ParamWidget::create<CKSS>(Vec(123, 300), module, EverlastingGlottalStopper::DEEMPHASIS_FILTER_PARAM, 0.0, 1.0, 0));

	addInput(Port::create<PJ301MPort>(Vec(30, 134), Port::INPUT, module, EverlastingGlottalStopper::PITCH_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(108, 134), Port::INPUT, module, EverlastingGlottalStopper::FM_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(17, 247), Port::INPUT, module, EverlastingGlottalStopper::TIME_OPEN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(70, 247), Port::INPUT, module, EverlastingGlottalStopper::TIME_CLOSED_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(123, 247), Port::INPUT, module, EverlastingGlottalStopper::BREATHINESS_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(71, 325), Port::OUTPUT, module, EverlastingGlottalStopper::VOICE_OUTPUT));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, EverlastingGlottalStopper) {
   Model *modelEverlastingGlottalStopper = Model::create<EverlastingGlottalStopper, EverlastingGlottalStopperWidget>("Frozen Wasteland", "EverlastingGlottalStopper", "Everlasting Glottal Stopper", FILTER_TAG);
   return modelEverlastingGlottalStopper;
}
