////////////////////////////////////////////////
///
// Dual digital oscillator
// based on Fundamental OSC by Andrew Belt
//	still some fix to do;)
//
//
//////////////////////////////////


#include "dBiz.hpp"
#include "dsp/decimator.hpp"
#include "dsp/filter.hpp"

//// Based on Fundamental OSC

namespace rack_plugin_dBiz {

extern float sawTable[2048];
extern float triTable[2048];


template <int OVERSAMPLE, int QUALITY>
struct VoltageControlledOscillator {
	bool analog = false;
	bool soft = false;
	float lastSyncValue = 0.0;
	float phase = 0.0;
	float freq;
	float pw = 0.5;
	float pitch;
	bool syncEnabled = false;
	bool syncDirection = false;

	Decimator<OVERSAMPLE, QUALITY> sinDecimator;
	Decimator<OVERSAMPLE, QUALITY> triDecimator;
	Decimator<OVERSAMPLE, QUALITY> sawDecimator;
	Decimator<OVERSAMPLE, QUALITY> sqrDecimator;
	RCFilter sqrFilter;

	// For analog detuning effect
	float pitchSlew = 0.0;
	int pitchSlewIndex = 0;

	float sinBuffer[OVERSAMPLE] = {};
	float triBuffer[OVERSAMPLE] = {};
	float sawBuffer[OVERSAMPLE] = {};
	float sqrBuffer[OVERSAMPLE] = {};

	void setPitch(float pitchKnob, float pitchCv) {
		// Compute frequency
		pitch = pitchKnob;
		if (analog) {
			// Apply pitch slew
			const float pitchSlewAmount = 2.0;
			pitch += pitchSlew * pitchSlewAmount;
		}
		else {
			// Quantize coarse knob if digital mode
			pitch = roundf(pitch);
		}
		pitch += pitchCv;
		// Note C3
		freq = 261.626 * powf(2.0, pitch / 12.0);
	}
	void setPulseWidth(float pulseWidth) {
		const float pwMin = 0.01;
		pw = clamp(pulseWidth, pwMin, 1.0 - pwMin);
	}

	void process(float deltaTime, float syncValue) {
		if (analog) {
			// Adjust pitch slew
			if (++pitchSlewIndex > 32) {
				const float pitchSlewTau = 100.0; // Time constant for leaky integrator in seconds
				pitchSlew += (randomNormal() - pitchSlew / pitchSlewTau) / engineGetSampleRate();
				pitchSlewIndex = 0;
			}
		}

		// Advance phase
		float deltaPhase = clamp(freq * deltaTime, 1e-6, 0.5);

		// Detect sync
		int syncIndex = -1; // Index in the oversample loop where sync occurs [0, OVERSAMPLE)
		float syncCrossing = 0.0; // Offset that sync occurs [0.0, 1.0)
		if (syncEnabled) {
			syncValue -= 0.01;
			if (syncValue > 0.0 && lastSyncValue <= 0.0) {
				float deltaSync = syncValue - lastSyncValue;
				syncCrossing = 1.0 - syncValue / deltaSync;
				syncCrossing *= OVERSAMPLE;
				syncIndex = (int)syncCrossing;
				syncCrossing -= syncIndex;
			}
			lastSyncValue = syncValue;
		}

		if (syncDirection)
			deltaPhase *= -1.0;

		sqrFilter.setCutoff(40.0 * deltaTime);

		for (int i = 0; i < OVERSAMPLE; i++) {
			if (syncIndex == i) {
				if (soft) {
					syncDirection = !syncDirection;
					deltaPhase *= -1.0;
				}
				else {
					// phase = syncCrossing * deltaPhase / OVERSAMPLE;
					phase = 0.0;
				}
			}

			if (analog) {
				// Quadratic approximation of sine, slightly richer harmonics
				if (phase < 0.5f)
					sinBuffer[i] = 1.f - 16.f * powf(phase - 0.25f, 2);
				else
					sinBuffer[i] = -1.f + 16.f * powf(phase - 0.75f, 2);
				sinBuffer[i] *= 1.08f;
			}
			else {
				sinBuffer[i] = sinf(2.f*M_PI * phase);
			}
			if (analog) {
				triBuffer[i] = 1.25f * interpolateLinear(triTable, phase * 2047.f);
			}
			else {
				if (phase < 0.25f)
					triBuffer[i] = 4.f * phase;
				else if (phase < 0.75f)
					triBuffer[i] = 2.f - 4.f * phase;
				else
					triBuffer[i] = -4.f + 4.f * phase;
			}
			if (analog) {
				sawBuffer[i] = 1.66f * interpolateLinear(sawTable, phase * 2047.f);
			}
			else {
				if (phase < 0.5f)
					sawBuffer[i] = 2.f * phase;
				else
					sawBuffer[i] = -2.f + 2.f * phase;
			}
			sqrBuffer[i] = (phase < pw) ? 1.f : -1.f;
			if (analog) {
				// Simply filter here
				sqrFilter.process(sqrBuffer[i]);
				sqrBuffer[i] = 0.71f * sqrFilter.highpass();
			}

			// Advance phase
			phase += deltaPhase / OVERSAMPLE;
			phase = eucmod(phase, 1.0);
		}
	}

	float sin() {
		return sinDecimator.process(sinBuffer);
	}
	float tri() {
		return triDecimator.process(triBuffer);
	}
	float saw() {
		return sawDecimator.process(sawBuffer);
	}
	float sqr() {
		return sqrDecimator.process(sqrBuffer);
	}
	float light() {
		return sinf(2*M_PI * phase);
	}
};


struct DVCO : Module {
	enum ParamIds {
		MODE_A_PARAM,
		MODE_B_PARAM,
		SYNC_A_PARAM,
		SYNC_B_PARAM,
		FREQ_A_PARAM,
		FREQ_B_PARAM,
		FINE_A_PARAM,
		FINE_B_PARAM,
		FM_A_PARAM,
		FM_B_PARAM,
		PW_A_PARAM,
		PW_B_PARAM,
		PWM_A_PARAM,
		PWM_B_PARAM,
		WAVE_A_PARAM,
		WAVE_B_PARAM,
		LFO_A_MODE_PARAM,
		LFO_B_MODE_PARAM,
		OSC_SYNC_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_A_INPUT,
		PITCH_B_INPUT,
		FM_A_INPUT,
		FM_B_INPUT,
		SYNC_A_INPUT,
		SYNC_B_INPUT,
		PW_A_INPUT,
		PW_B_INPUT,
		WAVE_A_INPUT,
		WAVE_B_INPUT,
		CARRIER_INPUT,
		MODULATOR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OSC_A_OUTPUT,
		OSC_AN_OUTPUT,
		OSC_B_OUTPUT,
		OSC_BN_OUTPUT,
		RING_OUTPUT,
		SUM_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	VoltageControlledOscillator<16, 16> oscillator_a;
	VoltageControlledOscillator<16, 16> oscillator_b;

	DVCO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void DVCO::step() {

	oscillator_a.analog = params[MODE_A_PARAM].value > 0.0;
	oscillator_a.soft = params[SYNC_A_PARAM].value <= 0.0;
	oscillator_b.analog = params[MODE_B_PARAM].value > 0.0;
	oscillator_b.soft = params[SYNC_B_PARAM].value <= 0.0;

	float carrier = inputs[CARRIER_INPUT].value / 5.0;
    float modulator = inputs[MODULATOR_INPUT].value / 5.0;

	float pitchFine_a = 3.0 * quadraticBipolar(params[FINE_A_PARAM].value);
	float pitchCv_a = 12.0 * inputs[PITCH_A_INPUT].value;
	float pitchFine_b = 3.0 * quadraticBipolar(params[FINE_B_PARAM].value);
	float pitchCv_b = 12.0 * inputs[PITCH_B_INPUT].value;

	if (inputs[FM_A_INPUT].active) {
		pitchCv_a += quadraticBipolar(params[FM_A_PARAM].value) * 12.0 * inputs[FM_A_INPUT].value;
	}
	if (inputs[FM_B_INPUT].active) {
		pitchCv_b += quadraticBipolar(params[FM_B_PARAM].value) * 12.0 * inputs[FM_B_INPUT].value;
	}
	else
		pitchCv_b += quadraticBipolar(params[FM_B_PARAM].value) * 12.0 * outputs[OSC_A_OUTPUT].value;

	if(params[LFO_A_MODE_PARAM].value==0.0){
	oscillator_a.setPitch(params[FREQ_A_PARAM].value, pitchFine_a + pitchCv_a);
	oscillator_a.freq=oscillator_a.freq/100;
	}
	else
	oscillator_a.setPitch(params[FREQ_A_PARAM].value, pitchFine_a + pitchCv_a);

	oscillator_a.setPulseWidth(params[PW_A_PARAM].value + params[PWM_A_PARAM].value * inputs[PW_A_INPUT].value / 10.0);
	oscillator_a.syncEnabled = inputs[SYNC_A_INPUT].active;
	oscillator_a.process(1.0 / engineGetSampleRate(), inputs[SYNC_A_INPUT].value);

	if(params[LFO_B_MODE_PARAM].value==0.0){
	oscillator_b.setPitch(params[FREQ_B_PARAM].value, pitchFine_b + pitchCv_b);
	oscillator_b.freq=oscillator_b.freq/100;
	}
	else
	oscillator_b.setPitch(params[FREQ_B_PARAM].value, pitchFine_b + pitchCv_b);
	oscillator_b.setPulseWidth(params[PW_B_PARAM].value + params[PWM_B_PARAM].value * inputs[PW_B_INPUT].value / 10.0);

	if(params[OSC_SYNC_PARAM].value==0.0){
	oscillator_b.syncEnabled = true;
	oscillator_b.process(1.0 / engineGetSampleRate(), outputs[OSC_A_OUTPUT].value);
	}

else {
	oscillator_b.syncEnabled = inputs[SYNC_B_INPUT].active;
	oscillator_b.process(1.0 / engineGetSampleRate(), inputs[SYNC_B_INPUT].value);
	}

	float wave_a = clamp(params[WAVE_A_PARAM].value + inputs[WAVE_A_INPUT].value, 0.0f, 3.0f);
	float out_a;
	if (wave_a < 1.0)
		out_a = crossfade(oscillator_a.sin(), oscillator_a.tri(), wave_a);
	else if (wave_a < 2.0)
		out_a = crossfade(oscillator_a.tri(), oscillator_a.saw(), wave_a - 1.0);
	else
		out_a = crossfade(oscillator_a.saw(), oscillator_a.sqr(), wave_a - 2.0);

	float wave_b = clamp(params[WAVE_B_PARAM].value + inputs[WAVE_B_INPUT].value, 0.0f, 3.0f);
	float out_b;
	if (wave_b < 1.0)
		out_b = crossfade(oscillator_b.sin(), oscillator_b.tri(), wave_b);
	else if (wave_b < 2.0)
		out_b = crossfade(oscillator_b.tri(), oscillator_b.saw(), wave_b - 1.0);
	else
		out_b = crossfade(oscillator_b.saw(), oscillator_b.sqr(), wave_b - 2.0);

	

	if (inputs[CARRIER_INPUT].active && inputs[MODULATOR_INPUT].value == 0.0)
	{
	outputs[RING_OUTPUT].value=5.0*carrier*out_b;
	outputs[SUM_OUTPUT].value = 5.0 * (carrier + out_b);
	}
	else if (inputs[MODULATOR_INPUT].active && inputs[CARRIER_INPUT].value == 0.0)
	{
		outputs[RING_OUTPUT].value = 5.0 * out_a * modulator;
		outputs[SUM_OUTPUT].value = 5.0 * (out_a + modulator);
	}
	else if (inputs[MODULATOR_INPUT].active && inputs[CARRIER_INPUT].active)
	{
	outputs[RING_OUTPUT].value=5.0*carrier*modulator;
	outputs[SUM_OUTPUT].value=5.0*(carrier+modulator);
	}
	else 
	{
	outputs[RING_OUTPUT].value=5.0*out_a*out_b;
	outputs[SUM_OUTPUT].value = 2.5*(out_a + out_b);
	}

	outputs[OSC_AN_OUTPUT].value = -5.0 * out_a;
	outputs[OSC_BN_OUTPUT].value = -5.0 * out_b;
	outputs[OSC_A_OUTPUT].value = 5.0 * out_a;
	outputs[OSC_B_OUTPUT].value = 5.0 * out_b;
	
	
	
}


struct DVCOWidget : ModuleWidget 
{
DVCOWidget(DVCO *module) : ModuleWidget(module)
{
	box.size = Vec(15*13, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DVCO.svg")));
		addChild(panel);
	}

	
	float mid = (15*13)/2;
	int jacks = 27;
	int knobs = 38;
	int border=10;



	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


///////////////////////////////////////port//////////////////////////////////////////////////
	addParam(ParamWidget::create<CKSS>(Vec(mid/2-10, 20), module, DVCO::LFO_A_MODE_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(mid-8, 20), module, DVCO::OSC_SYNC_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(mid+(mid/2-5), 20), module,DVCO::LFO_B_MODE_PARAM, 0.0, 1.0, 1.0));

	addParam(ParamWidget::create<CKSS>(Vec(border, 260), module, DVCO::MODE_A_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(border+jacks, 260), module, DVCO::SYNC_A_PARAM, 0.0, 1.0, 1.0));

	addParam(ParamWidget::create<CKSS>(Vec(box.size.x - 26, 260), module, DVCO::MODE_B_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(box.size.x - jacks - 26, 260), module, DVCO::SYNC_B_PARAM, 0.0, 1.0, 1.0));

	///////////////////////////////////////params////////////////////////////////////////


	addParam(ParamWidget::create<LRoundWhy>(Vec(10, 50), module, DVCO::FREQ_A_PARAM, -54.0, 54.0, 0.0));
	addParam(ParamWidget::create<RoundWhy>(Vec(55, 40), module, DVCO::FINE_A_PARAM, -1.5, 1.5, 0.0));

	addParam(ParamWidget::create<LRoundWhy>(Vec(box.size.x -45-10, 50), module, DVCO::FREQ_B_PARAM, -54.0, 54.0, 0.0));
	addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x-95, 40), module, DVCO::FINE_B_PARAM, -1.5, 1.5, 0.0));


	addParam(ParamWidget::create<RoundAzz>(Vec(15, 110), module, DVCO::PW_A_PARAM, 0.0, 1.0, 0.5));
	addParam(ParamWidget::create<RoundWhy>(Vec(15+knobs+5, 100), module, DVCO::FM_A_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundAzz>(Vec(5, 160), module, DVCO::PWM_A_PARAM, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<RoundAzz>(Vec(box.size.x - knobs-15, 110), module, DVCO::PW_B_PARAM, 0.0, 1.0, 0.5));
	addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x - (knobs*2)-15-5, 100), module, DVCO::FM_B_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundAzz>(Vec(box.size.x - knobs-5, 160), module, DVCO::PWM_B_PARAM, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<RoundRed>(Vec(15+knobs, 150), module, DVCO::WAVE_A_PARAM, 0.0, 3.0, 1.5));
	addParam(ParamWidget::create<RoundRed>(Vec(box.size.x - (knobs * 2) - 15 , 150), module, DVCO::WAVE_B_PARAM, 0.0, 3.0, 1.5));

	////////////////////////////////jacks//////////////////////////////////////////////////////////


	addInput(Port::create<PJ301MIPort>(Vec(border, 290), Port::INPUT, module, DVCO::PITCH_A_INPUT));
	addInput(Port::create<PJ301MCPort>(Vec(border+jacks, 290), Port::INPUT, module, DVCO::FM_A_INPUT));
	addInput(Port::create<PJ301MIPort>(Vec(border + jacks*2, 290), Port::INPUT, module, DVCO::WAVE_A_INPUT));

	addInput(Port::create<PJ301MIPort>(Vec(border+jacks, 330), Port::INPUT, module, DVCO::SYNC_A_INPUT));
	addInput(Port::create<PJ301MIPort>(Vec(border, 330), Port::INPUT, module, DVCO::PW_A_INPUT));

	addInput(Port::create<PJ301MIPort>(Vec(box.size.x-8-jacks, 290), Port::INPUT, module, DVCO::PITCH_B_INPUT));
	addInput(Port::create<PJ301MCPort>(Vec(box.size.x-8-jacks*2, 290), Port::INPUT, module, DVCO::FM_B_INPUT));
	addInput(Port::create<PJ301MIPort>(Vec(box.size.x-8-jacks*3, 290), Port::INPUT, module, DVCO::WAVE_B_INPUT));

	addInput(Port::create<PJ301MIPort>(Vec(box.size.x-(jacks*2)-8, 330), Port::INPUT, module, DVCO::SYNC_B_INPUT));
	addInput(Port::create<PJ301MIPort>(Vec(box.size.x-jacks-8, 330), Port::INPUT, module, DVCO::PW_B_INPUT));

	addInput(Port::create<PJ301MOrPort>(Vec(mid-jacks-15,245), Port::INPUT, module, DVCO::CARRIER_INPUT));
	addInput(Port::create<PJ301MOrPort>(Vec(mid+3+10, 245), Port::INPUT, module, DVCO::MODULATOR_INPUT));
	
//////////////////////////////OUTPUTS////////////////////////////////////////////////////////////////

	addOutput(Port::create<PJ301MOPort>(Vec(border, 225), Port::OUTPUT, module, DVCO::OSC_A_OUTPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(border+jacks, 225), Port::OUTPUT, module, DVCO::OSC_AN_OUTPUT));

	addOutput(Port::create<PJ301MOPort>(Vec(box.size.x-jacks-8 , 225), Port::OUTPUT, module, DVCO::OSC_B_OUTPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(box.size.x-jacks*2-8 , 225), Port::OUTPUT, module, DVCO::OSC_BN_OUTPUT));


	addOutput(Port::create<PJ301MOPort>(Vec(mid-14, 255), Port::OUTPUT, module, DVCO::SUM_OUTPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(mid-14, 225), Port::OUTPUT, module, DVCO::RING_OUTPUT));

	
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, DVCO) {
   Model *modelDVCO = Model::create<DVCO, DVCOWidget>("dBiz", "DVCO", "DVCO", OSCILLATOR_TAG);
   return modelDVCO;
}

