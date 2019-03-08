#include "Bark.hpp"
#include "barkComponents.hpp"
#include "dsp/vumeter.hpp"
#include "dependancies/filt/biquad.cpp"
#include "dependancies/filt/lp24.cpp"

namespace rack_plugin_Bark {

struct OneBand : Module {
	enum ParamIds {
		EQGAIN_PARAM,
		EQFREQ_PARAM,
		EQBANDWIDTH_PARAM,
		EQBYPASS_PARAM,
		PREPOST_PARAM,
		OUTGAIN_PARAM,
		SWAPLR_PARAM,
		LISTEN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GAINMOD_INPUT,
		FREQMOD_INPUT,
		BWMOD_INPUT,
		INL_INPUT,
		INR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		devParamOutGain,
		devParamOutFrq,
		devParamOutQ,
		NUM_OUTPUTS
	};
	enum LightIds {
		FreqParamOn,
		FreqParamOff,
		dBpeak_LIGHT,
		NUM_LIGHTS = dBpeak_LIGHT + 8			//7 + Clipping Light
	};
	
	OneBand() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

	VUMeter volUnitIndicatorPEAK;
	//create object of biquad filter
	Biquad *parametricEQL = new Biquad();
	Biquad *parametricEQR = new Biquad();
	LadderFilter lpf24;		//smoothing
};

void OneBand::step(){
	bool Listen = params[LISTEN_PARAM].value, swapLR = params[SWAPLR_PARAM].value;
	float inL = 0.0f, inR = 0.0f, outLdBVU = 0.0f, outRdBVU = 0.0f, 
		outL = clamp(outputs[OUTL_OUTPUT].value, -9.9f, 9.9f), outR = clamp(outputs[OUTR_OUTPUT].value, -9.9f, 9.9f),
		Gain = params[OUTGAIN_PARAM].value / 2.0f;
	double eqGain = params[EQGAIN_PARAM].value * 5.0 + (clamp(inputs[GAINMOD_INPUT].value, -6.5f, 6.5f) * 5.0),
		modInput, eqFreq = fmax(params[EQFREQ_PARAM].value, fmin(abs(.01375), abs(10.))),
		eqQ = clamp(params[EQBANDWIDTH_PARAM].value, 2.f, 40.f) + (4.f * clamp(inputs[BWMOD_INPUT].value, 2.f, 40.f));
	double sampRate, biquadFreq, biquadQ, biquadGain;
	
	//main gain knob
	inL = inputs[INL_INPUT].value;	//dBmeter on input
	inR = inputs[INR_INPUT].value;
	outLdBVU = outputs[OUTL_OUTPUT].value;	//dBmeter on output
	outRdBVU = outputs[OUTR_OUTPUT].value;
	
	//dB Peak Level Indicator------------------------------------------------------- pre EQ / post EQ
	volUnitIndicatorPEAK.dBInterval = 3.f;
	for (int light = 0; light < 8; light++) {
		//TODO: L/R unbalanced inputs will not engage clip light
		if (params[PREPOST_PARAM].value == 0.0f && !inputs[INR_INPUT].active) {//pre
			volUnitIndicatorPEAK.setValue(inL / 10.f);
			lights[dBpeak_LIGHT + light].setBrightnessSmooth(volUnitIndicatorPEAK.getBrightness(light));
		} else if (params[PREPOST_PARAM].value == 1.0f && !inputs[INR_INPUT].active) {//post
			volUnitIndicatorPEAK.setValue(outLdBVU / 10.f);
			lights[dBpeak_LIGHT + light].setBrightnessSmooth(volUnitIndicatorPEAK.getBrightness(light));
		}
		if (params[PREPOST_PARAM].value == 0.0f && inputs[INR_INPUT].active) {
			volUnitIndicatorPEAK.setValue(((inL / 10.f) + (inR / 10.f)) / 2.f);
			lights[dBpeak_LIGHT + light].setBrightnessSmooth(volUnitIndicatorPEAK.getBrightness(light));
		} else if (params[PREPOST_PARAM].value == 1.0f && inputs[INR_INPUT].active) {
			volUnitIndicatorPEAK.setValue(((outLdBVU / 10.f) + (outRdBVU / 10.f)) / 2.f);
			lights[dBpeak_LIGHT + light].setBrightnessSmooth(volUnitIndicatorPEAK.getBrightness(light));
		}
	}
	//dB Peak Level Indicator-------------------------------------------------------

		if (inputs[FREQMOD_INPUT].active) {
			lights[FreqParamOn].setBrightness(0); lights[FreqParamOff].setBrightness(1);
			modInput = fminf(clamp((inputs[FREQMOD_INPUT].value / 5.f), abs(.01375f), abs(10.f)), fmaxf(abs(.01375f), abs(10.f)));
			lpf24.setCutoff(110.f);
			lpf24.process(modInput, engineGetSampleTime());
			eqFreq = clamp(params[EQFREQ_PARAM].value, .11f, .11f) + ((5.f * lpf24.lowpass) * 2);
		} else if (!inputs[FREQMOD_INPUT].active){
			lights[FreqParamOn].setBrightness(1); lights[FreqParamOff].setBrightness(0);
			eqFreq = fmax(params[EQFREQ_PARAM].value, fmin(abs(.01375), abs(10.)));
		}
		sampRate = engineGetSampleRate();
		biquadFreq = eqFreq * 2000.f;
		biquadQ = eqQ;
		//Bypass EQ/Gain------------------------
		if (params[EQBYPASS_PARAM].value < 1.f) {biquadGain = eqGain;}		//EQ ON
		else if (params[EQBYPASS_PARAM].value > 0.f) {biquadGain = 0.0;}	//EQ OFF
		//Bypass EQ Gain------------------------
		//set Biquad Values
		parametricEQL->setBiquad(bq_type_peak, biquadFreq / sampRate, biquadQ, biquadGain);
		parametricEQR->setBiquad(bq_type_peak, biquadFreq / sampRate, biquadQ, biquadGain);
		//Listen to EQ
		if (Listen == 1) {//invert input
			if (swapLR == 1) {
				outR = parametricEQL->process(inL * Gain) -inL;
				outL = parametricEQR->process(inR * Gain) -inR;
			} else {
				outL = parametricEQL->process(inL * Gain) -inL;
				outR = parametricEQR->process(inR * Gain) -inR;
			}
		} else if (Listen != 1) {
			if (swapLR == 1) {
				outR = parametricEQL->process(inL * Gain);
				outL = parametricEQR->process(inR * Gain);
			} else {
				outL = parametricEQL->process(inL * Gain);
				outR = parametricEQR->process(inR * Gain);
			}
		}
		
		//dev---Check parameter value---------------------------------
		//outputs[devParamOutGain].value = params[EQGAIN_PARAM].value;
		//outputs[devParamOutFrq].value = params[EQFREQ_PARAM].value;
		//outputs[devParamOutQ].value = params[EQBANDWIDTH_PARAM].value;
		//dev---Check parameter value---------------------------------
		
		outputs[OUTL_OUTPUT].value = outL;
		outputs[OUTR_OUTPUT].value = outR;
}

struct OneBandWidget : ModuleWidget{
	OneBandWidget(OneBand *module);
};

OneBandWidget::OneBandWidget(OneBand *module) : ModuleWidget(module) {

	int rackY = 380;
	float lightXpos = 45.5f, offsetKnobs = 0.47f;

	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Bark1Band.svg")));
		panel->box.size = box.size;
		addChild(panel);
	}

	///Ports---
	//Out---
	addOutput(Port::create<BarkOutPort350>(Vec(4.05f, rackY - 174.04f), Port::OUTPUT, module, OneBand::OUTL_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(31.38f, rackY - 174.04f), Port::OUTPUT, module, OneBand::OUTR_OUTPUT));
	//In---
	//Audio--
	addInput(Port::create<BarkInPort350>(Vec(4.05f, rackY - 46.16f), Port::INPUT, module, OneBand::INL_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(31.38f, rackY - 46.16f), Port::INPUT, module, OneBand::INR_INPUT));
	//Mod--
	addInput(Port::create<BarkPatchPortIn>(Vec(34.16f, rackY - 324.73f), Port::INPUT, module, OneBand::GAINMOD_INPUT));
	addInput(Port::create<BarkPatchPortIn>(Vec(2.16f, rackY - 307.59f), Port::INPUT, module, OneBand::FREQMOD_INPUT));
	addInput(Port::create<BarkPatchPortIn>(Vec(33.78f, rackY - 248.26f), Port::INPUT, module, OneBand::BWMOD_INPUT));
	//dev test param values
	//addOutput(Port::create<BarkPatchPortIn>(Vec(34.16f, rackY - 324.73f), Port::OUTPUT, module, OneBand::devParamOutGain));
	//addOutput(Port::create<BarkPatchPortIn>(Vec(2.16f, rackY - 307.59f), Port::OUTPUT, module, OneBand::devParamOutFrq));
	//addOutput(Port::create<BarkPatchPortIn>(Vec(33.78f, rackY - 248.26f), Port::OUTPUT, module, OneBand::devParamOutQ));
	//Knobs---
	addParam(ParamWidget::create<BarkKnob26>(Vec(10.2f - offsetKnobs, rackY - 349.73f), module, OneBand::EQGAIN_PARAM, -6.5f, 6.5f, 0.f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(24.95f - offsetKnobs, rackY - 291.2f), module, OneBand::EQFREQ_PARAM, .01375f, 10.f, 0.11f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(10.68f - offsetKnobs, rackY - 231.51f), module, OneBand::EQBANDWIDTH_PARAM, 1.f, 40.f, 4.f));
	addParam(ParamWidget::create<BarkKnob30>(Vec(8.29f, rackY - 94.67f), module, OneBand::OUTGAIN_PARAM, 0.f, 7.f, 2.f));
	//Switch---
	addParam(ParamWidget::create<BarkSwitchSmall>(Vec(41.29f, rackY - 355.97f), module, OneBand::EQBYPASS_PARAM, 0.f, 1.f, 0.f));
	addParam(ParamWidget::create<BarkSwitchSmall>(Vec(11.26f, rackY - 123.78f), module, OneBand::PREPOST_PARAM, 0.f, 1.f, 0.f));
	addParam(ParamWidget::create<BarkSwitchSmallSide>(Vec(21.89f, rackY - 148.45f), module, OneBand::SWAPLR_PARAM, 0.f, 1.f, 0.f));//swapL/R
	addParam(ParamWidget::create<BarkSwitchSmall>(Vec(40.4f, rackY - 200.21f), module, OneBand::LISTEN_PARAM, 0.f, 1.f, 0.f));
	//TODO: Screw Positions
	addChild(Widget::create<BarkScrew1>(Vec(box.size.x - 13, 3)));		//pos2
	addChild(Widget::create<BarkScrew2>(Vec(2, 367.2f)));			//pos3
	//Light---
	float floatyMcFloatFace = 16.11f; //18.04f
	addChild(ModuleLightWidget::create<SmallerLightFA<ParamInLight>>(Vec(floatyMcFloatFace, rackY - 280.05f), module, OneBand::FreqParamOn));
	addChild(ModuleLightWidget::create<SmallerLightFA<ParamInLight>>(Vec(floatyMcFloatFace, rackY - 261.72f), module, OneBand::FreqParamOff));
	addChild(ModuleLightWidget::create<BiggerLight<clipLight>>(Vec(lightXpos - 1.f, rackY - 134.65f), module, OneBand::dBpeak_LIGHT + 0));
	addChild(ModuleLightWidget::create<BigLight<redLight>>(Vec(lightXpos, rackY - 133.66f), module, OneBand::dBpeak_LIGHT + 1));
	addChild(ModuleLightWidget::create<BigLight<orangeLight>>(Vec(lightXpos, rackY - 122.11f), module, OneBand::dBpeak_LIGHT + 2));
	addChild(ModuleLightWidget::create<BigLight<yellowLight2>>(Vec(lightXpos, rackY - 110.55f), module, OneBand::dBpeak_LIGHT + 3));
	addChild(ModuleLightWidget::create<BigLight<yellowLight1>>(Vec(lightXpos, rackY - 99.f), module, OneBand::dBpeak_LIGHT + 4));
	addChild(ModuleLightWidget::create<BigLight<greenLight>>(Vec(lightXpos, rackY - 87.45f), module, OneBand::dBpeak_LIGHT + 5));
	addChild(ModuleLightWidget::create<BigLight<greenLight>>(Vec(lightXpos, rackY - 75.9f), module, OneBand::dBpeak_LIGHT + 6));
	addChild(ModuleLightWidget::create<BigLight<greenLight>>(Vec(lightXpos, rackY - 64.35f), module, OneBand::dBpeak_LIGHT + 7));
	

}

} // namespace rack_plugin_Bark

using namespace rack_plugin_Bark;

RACK_PLUGIN_MODEL_INIT(Bark, OneBand) {
   Model *modelOneBand = Model::create<OneBand, OneBandWidget>("Bark", "1Band", "One Band", EQUALIZER_TAG, AMPLIFIER_TAG);
   return modelOneBand;
}
