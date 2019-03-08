#include "Bark.hpp"
#include "dsp/digital.hpp"
#include "util/math.hpp"
#include "barkComponents.hpp"
#include "dsp/functions.hpp"
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>

namespace rack_plugin_Bark {

struct LowFrequencyOscillator {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {

	}

	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0f);
		freq = powf(2.0f, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01f;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset / 0.01f)) {
			phase = 0.0f;
		}
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}
	float sin() {
		if (offset)
			return 1.0f - cosf(2 * M_PI * phase) * (invert ? -1.0f : 1.0f);
		else
			return sinf(2 * M_PI * phase) * (invert ? -1.0f : 1.0f);
	}
	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5f : phase);
		else
			return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
	}
	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
		else
			return saw(phase) * (invert ? -1.0f : 1.0f);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0f : -1.0f;
		return offset ? sqr + 1.0f : sqr;
	}
	float light() {
		return sinf(2 * M_PI * phase);
	}
};

struct TrimLFO : Module {
	enum ParamIds
	{
		//Offset
		OFFSET1_PARAM,
		OFFSET2_PARAM,
		//LFO
		OFFSET_PARAM,
		INVERT_PARAM,
		FREQ_PARAM,
		FINE_PARAM,
		FM1_PARAM,
		FM2_PARAM,
		PW_PARAM,
		PWM_PARAM,
		WAVEMIX_PARAM,
		RESET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		//TODO mod source for ±1v sin/sqr Offset out
		//MODSRC_INPUT,		//Modulates Offset Params
		FM1_INPUT,
		FM2_INPUT,
		RESET_INPUT,
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT1a_OUTPUT,
		OUT2a_OUTPUT,
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		trimLFO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator oscillator;
	float volts1 = 0.000f;
	float volts2 = 0.000f;
	float freqHz = 1.f;

	TrimLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
	

};


void TrimLFO::step() {

	volts1 = clamp(params[OFFSET1_PARAM].value, -10.0f, 10.0f);
	volts2 = clamp(params[OFFSET2_PARAM].value, -10.0f, 10.0f);
	
	float out1 = params[OFFSET1_PARAM].value + oscillator.sin(); //Attenuvert??? osc.sin
	float out2 = params[OFFSET2_PARAM].value + oscillator.sqr();
	float out1a = params[OFFSET1_PARAM].value;	//Normal +-10v
	float out2a = params[OFFSET2_PARAM].value;	//Normal +-10v
	float fineTune = 3.0f * quadraticBipolar(params[FINE_PARAM].value);		//finetune

	outputs[OUT1_OUTPUT].value = clamp(out1, -10.0f, 10.0f);
	outputs[OUT2_OUTPUT].value = clamp(out2, -10.0f, 10.0f);
	outputs[OUT1a_OUTPUT].value = clamp(out1a, -10.0f, 10.0f);
	outputs[OUT2a_OUTPUT].value = clamp(out2a, -10.0f, 10.0f);
	oscillator.setPitch(params[FREQ_PARAM].value + fineTune + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	oscillator.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0f);
	oscillator.offset = (params[OFFSET_PARAM].value > 0.0f);
	oscillator.invert = (params[INVERT_PARAM].value <= 0.0f);
	oscillator.step(engineGetSampleTime());
	oscillator.setReset(inputs[RESET_INPUT].value || params[RESET_PARAM].value);

	//----------------	DISPLAY Hz	--------------------------
	float frq = params[FREQ_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value;
	float fine = fineTune;
	frq = clamp(frq, -32.0f, 32.0f);
	freqHz = 1.0f * powf(2.0f, frq + fine);
	//float isBPM = params[FREQHZplus_PARAM].value;
	//--------------------------------------------------------
	///TRIM LFO----
	//initilise oscillators
	float sinValue = 5.0f * oscillator.sin(), sawValue = 5.0f * oscillator.saw(),
		  triValue = 5.0f * oscillator.tri(), sqrValue = 5.0f * oscillator.sqr();
	float waveMixParam = clamp(params[WAVEMIX_PARAM].value, 0.0f, 3.0f), xFade;
	///sin.saw----
	if (waveMixParam < 1.0f) {// 0.0f sin
		xFade = crossfade(sinValue, sawValue, waveMixParam);
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, xFade));
	}
	///saw.tri----
	else if (waveMixParam < 2.0f) {	//1.0f saw
		xFade = crossfade(sawValue, triValue, waveMixParam - 1.0f); ///some of the higher voltages get lost TODO: fix that - 1.2 maybe
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, xFade));
	}
	///tri.sqr----
	else if (waveMixParam < 3.0f) { //2.0f tri
		xFade = crossfade(triValue, sqrValue, waveMixParam - 1.95f); 
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, xFade));
	}
	///sqr----
	else if (waveMixParam == 3.0f) { //3.0f sqr
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, sqrValue));
		
	}
	///LFO----
	outputs[SIN_OUTPUT].value = sinValue;
	outputs[SAW_OUTPUT].value = sawValue;
	outputs[TRI_OUTPUT].value = triValue;
	outputs[SQR_OUTPUT].value = sqrValue;
	lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator.light()));
	lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator.light()));
}

struct FreqDisplayWidget : TransparentWidget {

	float *freqHz;
	
	std::shared_ptr<Font> font;

	FreqDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/segoescb.ttf"));
	};

	void draw(NVGcontext *vg) override {
		
		float spacer = 40.f;
		
		// Background
		NVGcolor backgroundColor = nvgRGB(97, 54, 57);		//CreamyRed
		NVGcolor borderColor = nvgRGB(0, 0, 0);				//Black
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 0.75);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		nvgTextAlign(vg, 1 << 1);
		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1.0);
		Vec textPos = Vec(85.798f / 2.0f - 8.0f, 10.673f);
		//----- "Hz"
		NVGcolor textColor = nvgRGB(93, 193, 57);		//97, 193, 57 == Green
		nvgFillColor(vg, nvgTransRGBA(textColor, 255));
		char display_stringHz[10];
		snprintf(display_stringHz, sizeof(display_stringHz), "%0.52f", *freqHz);
		nvgText(vg, textPos.x + spacer, textPos.y, "Hz", NULL);
		nvgText(vg, textPos.x, textPos.y, display_stringHz, NULL);
		//----- Background
		/*
		NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~~~~~", NULL);

		textColor = nvgRGB(0xda, 0xe9, 0x29);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "//////////", NULL);
		*/
		textColor = nvgRGB(93, 193, 57);
		nvgFillColor(vg, textColor);
	}
};

struct VoltsDisplayWidget : TransparentWidget {

	float *value;
	
	std::shared_ptr<Font> font;

	VoltsDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/segoescb.ttf"));	//segoescb
	};

	void draw(NVGcontext *vg) override {
		// Background
		NVGcolor backgroundColor = nvgRGB(97, 54, 57);
		NVGcolor borderColor = nvgRGB(0, 0, 0);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 0.75);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		// text 
		nvgTextAlign(vg, 1 << 1);
		nvgFontSize(vg, 18);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 0.75);

		char display_string[10];
		sprintf(display_string, "%5.2f", *value);

		Vec textPos = Vec(25.0f, 10.0f);		//		85.798f, 13.673f 

		NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "$$$$$", NULL);

		textColor = nvgRGB(0xda, 0xe9, 0x29);
		nvgFillColor(vg, nvgTransRGBA(textColor, 11));
		nvgText(vg, textPos.x, textPos.y, "-00.00", NULL);

		textColor = nvgRGB(93, 193, 57);
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, display_string, NULL);
	}
};


struct TrimLFOWidget : ModuleWidget {
	TrimLFOWidget(TrimLFO *module);
};

TrimLFOWidget::TrimLFOWidget(TrimLFO *module) : ModuleWidget(module) {
	int rackY = 380;

	setPanel(SVG::load(assetPlugin(plugin, "res/BarkTrimLFO.svg")));
	///Ports---
	//Out---
	addOutput(Port::create<BarkOutPort350>(Vec(13.28f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::SIN_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(46.58f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::SAW_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(79.68f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::TRI_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(113.245f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::SQR_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(14.57f, rackY - 275.08f), Port::OUTPUT, module, TrimLFO::OUT1_OUTPUT));				//2v sin
	addOutput(Port::create<BarkOutPort350>(Vec(112.09f, rackY - 275.08f), Port::OUTPUT, module, TrimLFO::OUT2_OUTPUT));				//2v sqr
	addOutput(Port::create<BarkOutPort350>(Vec(42.11f + 0.35f, rackY - 275.08f), Port::OUTPUT, module, TrimLFO::OUT1a_OUTPUT));		//Offset1
	addOutput(Port::create<BarkOutPort350>(Vec(84.18f, rackY - 275.08f), Port::OUTPUT, module, TrimLFO::OUT2a_OUTPUT));				//Offset2
	addOutput(Port::create<BarkPatchPortOut>(Vec(63.35f, rackY - 332.02f), Port::OUTPUT, module, TrimLFO::trimLFO_OUTPUT));	//trimmed offset output 
	//In---
	addInput(Port::create<BarkInPort350>(Vec(27.06f, rackY - 82.70f), Port::INPUT, module, TrimLFO::FM1_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(63.25f, rackY - 82.70f), Port::INPUT, module, TrimLFO::FM2_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(99.66f, rackY - 82.70f), Port::INPUT, module, TrimLFO::PW_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(119.89f, rackY - 164.05f), Port::INPUT, module, TrimLFO::RESET_INPUT));
	//addInput(Port::create<BarkPatchPortIn>(Vec(63.35f, rackY - 332.02f), Port::INPUT, module, TrimLFO::MODSRC_INPUT));
	//Knobs---
	addParam(ParamWidget::create<BarkKnob70>(Vec(39.66f, rackY - 217.01f), module, TrimLFO::FREQ_PARAM, -16.0f, 4.0f, 1.0f));//-6
	addParam(ParamWidget::create<BarkScrew01>(Vec(box.size.x - 13, 367.2f), module, TrimLFO::FINE_PARAM, -0.06798301f, 0.06798301f, 0.0f));
	addParam(ParamWidget::create<BarkKnob40>(Vec(20.48f, rackY - 329.58f), module, TrimLFO::OFFSET1_PARAM, -10.0f, 10.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob40>(Vec(89.7f, rackY - 329.58f), module, TrimLFO::OFFSET2_PARAM, -10.0f, 10.0f, 10.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(5.19f, rackY - 167.6f), module, TrimLFO::PW_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(25.32f, rackY - 122.3f), module, TrimLFO::FM1_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(61.65f, rackY - 122.3f), module, TrimLFO::FM2_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(98.06f, rackY - 122.3f), module, TrimLFO::PWM_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkSlide1>(Vec(25.41f, rackY - 57.f), module, TrimLFO::WAVEMIX_PARAM, 0.0f, 3.0f, 0.0f));
	//Switch---
	addParam(ParamWidget::create<BarkSwitch>(Vec(8.67f, rackY - 217.06f), module, TrimLFO::OFFSET_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<BarkSwitch>(Vec(117.57f, rackY - 217.06f), module, TrimLFO::INVERT_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<BarkButton1>(Vec(122.72f, rackY - 138.20f), module, TrimLFO::RESET_PARAM, 0.0f, 1.0f, 1.0f));
	//Screw---
	addChild(Widget::create<BarkScrew3>(Vec(2, 3)));	//pos1
	//Light---
	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(71.93f, rackY - 230.22f), module, TrimLFO::PHASE_POS_LIGHT));
	//------------------------------
	///ref, VOLTS DISPLAY : AS-TriggersMKI
	VoltsDisplayWidget *display1 = new VoltsDisplayWidget();
	display1->box.pos = Vec(15.009f, 33.05f);
	display1->box.size = Vec(50.728f, 13.152f);
	display1->value = &module->volts1;
	addChild(display1);
	VoltsDisplayWidget *display2 = new VoltsDisplayWidget();
	display2->box.pos = Vec(84.228f, 33.05f);
	display2->box.size = Vec(50.728f, 13.152f);
	display2->value = &module->volts2;
	addChild(display2);
	//------------------------------
	FreqDisplayWidget *display3 = new FreqDisplayWidget();
	display3->box.pos = Vec(32.23f, 237.31f);	//== -L/+R, +==D/-==U
	display3->box.size = Vec(85.798f, 13.673f);	//x, y == Lenght, Height
	display3->freqHz = &module->freqHz;
	addChild(display3);
	//------------------------------
}

struct bpmTrimLFO : Module {
	enum ParamIds {
		//Offset
		OFFSET1_PARAM,
		OFFSET2_PARAM,
		//LFO
		OFFSET_PARAM,
		INVERT_PARAM,
		FREQ_PARAM,
		FINE_PARAM,
		FM1_PARAM,
		FM2_PARAM,
		PW_PARAM,
		PWM_PARAM,
		WAVEMIX_PARAM,
		BPM_PARAM,
		FREQHZplus_PARAM,	// +divide by 0.5f
		FREQHZminus_PARAM,	// -multiply by 0.5f
		RESET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		//TODO mod source for ±1v sin/sqr Offset out
		//MODSRC_INPUT,		//Modulates Offset Params
		FM1_INPUT,
		FM2_INPUT,
		RESET_INPUT,
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT1a_OUTPUT,
		OUT2a_OUTPUT,
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		trimLFO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator oscillator;
	float volts1bpm = 0.000f;
	float volts2bpm = 0.000f;
	float freqHz = 1.f;

	bpmTrimLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void bpmTrimLFO::step() {
	

	volts1bpm = clamp(params[OFFSET1_PARAM].value, -10.0f, 10.0f);
	volts2bpm = clamp(params[OFFSET2_PARAM].value, -10.0f, 10.0f);

	float out1 = params[OFFSET1_PARAM].value + oscillator.sin(); //Attenuvert??? osc.sin
	float out2 = params[OFFSET2_PARAM].value + oscillator.sqr();
	float out1a = params[OFFSET1_PARAM].value;	//Normal +-10v
	float out2a = params[OFFSET2_PARAM].value;	//Normal +-10v
	float fineTune = 3.0f * quadraticBipolar(params[FINE_PARAM].value),
		  bpmAdjust = 3.0f * quadraticBipolar(params[BPM_PARAM].value);		//finetune


	outputs[OUT1_OUTPUT].value = clamp(out1, -10.0f, 10.0f);
	outputs[OUT2_OUTPUT].value = clamp(out2, -10.0f, 10.0f);
	outputs[OUT1a_OUTPUT].value = clamp(out1a, -10.0f, 10.0f);
	outputs[OUT2a_OUTPUT].value = clamp(out2a, -10.0f, 10.0f);
	oscillator.setPitch(params[FREQ_PARAM].value + fineTune + bpmAdjust + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	oscillator.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0f);
	oscillator.offset = (params[OFFSET_PARAM].value > 0.0f);
	oscillator.invert = (params[INVERT_PARAM].value <= 0.0f);
	oscillator.step(engineGetSampleTime());
	oscillator.setReset(inputs[RESET_INPUT].value || params[RESET_PARAM].value);

	//----------------	DISPLAY Hz	--------------------------
	float frq = params[FREQ_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value;
	float fine = fineTune + bpmAdjust;
	frq = clamp(frq, -32.0f, 32.0f);
	freqHz = 1.0f * powf(2.0f, frq + fine);
	//float isBPM = params[FREQHZplus_PARAM].value;
	//--------------------------------------------------------
	///TRIM LFO----
	//initilise oscillators
	float sinValue = 5.0f * oscillator.sin(), sawValue = 5.0f * oscillator.saw(),
		triValue = 5.0f * oscillator.tri(), sqrValue = 5.0f * oscillator.sqr();
	float waveMixParam = clamp(params[WAVEMIX_PARAM].value, 0.0f, 3.0f), xFade;
	///sin.saw----
	if (waveMixParam < 1.0f) {// 0.0f sin
		xFade = crossfade(sinValue, sawValue, waveMixParam);
		outputs[trimLFO_OUTPUT].value = fmaxf (params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, xFade));
	}
	///saw.tri----
	else if (waveMixParam < 2.0f) {	//1.0f saw
		xFade = crossfade(sawValue, triValue, waveMixParam - 1.0f); ///some of the higher voltages get lost TODO: fix that - 1.2 maybe
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, xFade));
	}
	///tri.sqr----
	else if (waveMixParam < 3.0f) { //2.0f tri
		xFade = crossfade(triValue, sqrValue, waveMixParam - 1.95f);
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, xFade));
	}
	///sqr----
	else if (waveMixParam == 3.0f) { //3.0f sqr
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET1_PARAM].value, fminf(params[OFFSET2_PARAM].value, sqrValue));

	}
	///LFO----
	outputs[SIN_OUTPUT].value = sinValue;
	outputs[SAW_OUTPUT].value = sawValue;
	outputs[TRI_OUTPUT].value = triValue;
	outputs[SQR_OUTPUT].value = sqrValue;
	lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator.light()));
	lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator.light()));
}

struct BPMDisplayWidget : TransparentWidget {

	float *freqHz;
	std::shared_ptr<Font> font;

	BPMDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/segoescb.ttf"));
	};

	void draw(NVGcontext *vg) override {
		float spacer = 40.f;
		// Background
		NVGcolor backgroundColor = nvgRGB(97, 54, 57);		//CreamyRed
		NVGcolor borderColor = nvgRGB(0, 0, 0);				//Black
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 0.75);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		nvgTextAlign(vg, 1 << 1);
		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1.0);
		Vec textPos = Vec(85.798f / 2.0f - 8.0f, 10.673f);
		//format char length
		NVGcolor textColor = nvgRGB(93, 193, 57);		//97, 193, 57 == Green
		nvgFillColor(vg, nvgTransRGBA(textColor, 255));
		char display_stringBPM[8];
		//TODO: format decimal places <10 to .xxx <100 to .xxx
		snprintf(display_stringBPM, sizeof(display_stringBPM), "%0.52f", *freqHz * 60);
		//display_string.precision(3);
		nvgText(vg, textPos.x + spacer - 5, textPos.y, "BPM", NULL);
		//nvgFontSize(vg, 15);		//numbers only
		nvgText(vg, textPos.x - 5, textPos.y, display_stringBPM, NULL);	//moved from line 274
		textColor = nvgRGB(93, 193, 57);
		nvgFillColor(vg, textColor);
	}
};

struct bpmVoltsDisplayWidget : TransparentWidget {

	float *value;

	std::shared_ptr<Font> font;

	bpmVoltsDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/segoescb.ttf"));	//segoescb
	};

	void draw(NVGcontext *vg) override {
		// Background
		NVGcolor backgroundColor = nvgRGB(97, 54, 57);
		NVGcolor borderColor = nvgRGB(0, 0, 0);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 0.75);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		// text 
		nvgTextAlign(vg, 1 << 1);
		nvgFontSize(vg, 18);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 0.75);

		char display_string[10];
		sprintf(display_string, "%5.2f", *value);

		Vec textPos = Vec(25.0f, 10.0f);		//		85.798f, 13.673f 

		NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "$$$$$", NULL);

		textColor = nvgRGB(0xda, 0xe9, 0x29);
		nvgFillColor(vg, nvgTransRGBA(textColor, 11));
		nvgText(vg, textPos.x, textPos.y, "-00.00", NULL);

		textColor = nvgRGB(93, 193, 57);
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, display_string, NULL);
	}
};


struct bpmTrimLFOWidget : ModuleWidget {
	bpmTrimLFOWidget(bpmTrimLFO *module);

};

bpmTrimLFOWidget::bpmTrimLFOWidget(bpmTrimLFO *module) : ModuleWidget(module) {
	int rackY = 380;

	setPanel(SVG::load(assetPlugin(plugin, "res/BarkTrimLFO.svg")));
	///Ports---
	//Out---
	addOutput(Port::create<BarkOutPort350>(Vec(13.28f, rackY - 52.35f), Port::OUTPUT, module, bpmTrimLFO::SIN_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(46.58f, rackY - 52.35f), Port::OUTPUT, module, bpmTrimLFO::SAW_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(79.68f, rackY - 52.35f), Port::OUTPUT, module, bpmTrimLFO::TRI_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(113.245f, rackY - 52.35f), Port::OUTPUT, module, bpmTrimLFO::SQR_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(14.57f, rackY - 275.08f), Port::OUTPUT, module, bpmTrimLFO::OUT1_OUTPUT));				//2v sin
	addOutput(Port::create<BarkOutPort350>(Vec(112.09f, rackY - 275.08f), Port::OUTPUT, module, bpmTrimLFO::OUT2_OUTPUT));				//2v sqr
	addOutput(Port::create<BarkOutPort350>(Vec(42.11f + 0.35f, rackY - 275.08f), Port::OUTPUT, module, bpmTrimLFO::OUT1a_OUTPUT));		//Offset1
	addOutput(Port::create<BarkOutPort350>(Vec(84.18f, rackY - 275.08f), Port::OUTPUT, module, bpmTrimLFO::OUT2a_OUTPUT));				//Offset2
	addOutput(Port::create<BarkPatchPortOut>(Vec(63.35f, rackY - 332.02f), Port::OUTPUT, module, bpmTrimLFO::trimLFO_OUTPUT));	//trimmed offset output 
	//In---
	addInput(Port::create<BarkInPort350>(Vec(27.06f, rackY - 82.70f), Port::INPUT, module, bpmTrimLFO::FM1_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(63.25f, rackY - 82.70f), Port::INPUT, module, bpmTrimLFO::FM2_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(99.66f, rackY - 82.70f), Port::INPUT, module, bpmTrimLFO::PW_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(119.89f, rackY - 164.05f), Port::INPUT, module, bpmTrimLFO::RESET_INPUT));
	//addInput(Port::create<BarkPatchPortIn>(Vec(63.35f, rackY - 332.02f), Port::INPUT, module, bpmTrimLFO::MODSRC_INPUT));
	//Knobs---
	addParam(ParamWidget::create<BarkKnob70Snap>(Vec(39.66f, rackY - 217.01f), module, bpmTrimLFO::FREQ_PARAM, -16.0f, 4.0f, 1.0f));
	addParam(ParamWidget::create<BarkScrew01>(Vec(box.size.x - 13, 367.2f), module, bpmTrimLFO::FINE_PARAM, -0.06798301f, 0.06798301f, 0.0f));
	addParam(ParamWidget::create<BarkScrew02>(Vec(2, 3), module, bpmTrimLFO::BPM_PARAM, -1.0136f / 2, 0.952f / 2, 0.0f));//-0.2534 * 4, .2380 * 4
	addParam(ParamWidget::create<BarkKnob40>(Vec(20.48f, rackY - 329.58f), module, bpmTrimLFO::OFFSET1_PARAM, -10.0f, 10.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob40>(Vec(89.7f, rackY - 329.58f), module, bpmTrimLFO::OFFSET2_PARAM, -10.0f, 10.0f, 10.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(5.19f, rackY - 167.6f), module, bpmTrimLFO::PW_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(25.32f, rackY - 122.3f), module, bpmTrimLFO::FM1_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(61.65f, rackY - 122.3f), module, bpmTrimLFO::FM2_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(98.06f, rackY - 122.3f), module, bpmTrimLFO::PWM_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkSlide1>(Vec(25.41f, rackY - 57.f), module, bpmTrimLFO::WAVEMIX_PARAM, 0.0f, 3.0f, 0.0f));
	//Switch---
	addParam(ParamWidget::create<BarkSwitch>(Vec(8.67f, rackY - 217.06f), module, bpmTrimLFO::OFFSET_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<BarkSwitch>(Vec(117.57f, rackY - 217.06f), module, bpmTrimLFO::INVERT_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<BarkButton1>(Vec(122.72f, rackY - 138.20f), module, bpmTrimLFO::RESET_PARAM, 0.0f, 1.0f, 1.0f));
	//Screw---
	//addChild(Widget::create<BarkScrew3>(Vec(2, 3)));	//pos1
	//Light---
	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(71.93f, rackY - 230.22f), module, bpmTrimLFO::PHASE_POS_LIGHT));
	//------------------------------
	///ref, VOLTS DISPLAY : AS-TriggersMKI
	bpmVoltsDisplayWidget *display4 = new bpmVoltsDisplayWidget();
	display4->box.pos = Vec(15.009f, 33.05f);
	display4->box.size = Vec(50.728f, 13.152f);
	display4->value = &module->volts1bpm;
	addChild(display4);
	bpmVoltsDisplayWidget *display5 = new bpmVoltsDisplayWidget();
	display5->box.pos = Vec(84.228f, 33.05f);
	display5->box.size = Vec(50.728f, 13.152f);
	display5->value = &module->volts2bpm;
	addChild(display5);
	//------------------------------
	BPMDisplayWidget *display6 = new BPMDisplayWidget();
	display6->box.pos = Vec(32.23f, 237.31f);	//== -L/+R, +==D/-==U
	display6->box.size = Vec(85.798f, 13.673f);	//x, y == Lenght, Height
	display6->freqHz = &module->freqHz;
	addChild(display6);
	//------------------------------bpmTrimLFO
}

} // namespace rack_plugin_Bark

using namespace rack_plugin_Bark;

RACK_PLUGIN_MODEL_INIT(Bark, TrimLFO) {
   Model *modelTrimLFO = Model::create<TrimLFO, TrimLFOWidget>("Bark", "TrimLFO", "Trim LFO", LFO_TAG, UTILITY_TAG, LOGIC_TAG, DUAL_TAG);
   return modelTrimLFO;
}

RACK_PLUGIN_MODEL_INIT(Bark, TrimLFObpm) {
   Model *modelTrimLFObpm = Model::create<bpmTrimLFO, bpmTrimLFOWidget>("Bark", "bpmTrimLFO", "Trim LFO BPM", LFO_TAG, UTILITY_TAG, LOGIC_TAG, DUAL_TAG);
   return modelTrimLFObpm;
}
