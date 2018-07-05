#include "FrozenWasteland.hpp"
#include "dsp/decimator.hpp"
#include "dsp/digital.hpp"
#include "filters/biquad.h"

using namespace std;

#define BANDS 16

namespace rack_plugin_FrozenWasteland {

struct MrBlueSky : Module {
	enum ParamIds {
		BG_PARAM,
		ATTACK_PARAM = BG_PARAM + BANDS,
		DECAY_PARAM,
		CARRIER_Q_PARAM,
		MOD_Q_PARAM,
		BAND_OFFSET_PARAM,
		GMOD_PARAM,
		GCARR_PARAM,
		G_PARAM,
		SHAPE_PARAM,
		ATTACK_CV_ATTENUVERTER_PARAM,
		DECAY_CV_ATTENUVERTER_PARAM,
		CARRIER_Q_CV_ATTENUVERTER_PARAM,
		MODIFER_Q_CV_ATTENUVERTER_PARAM,
		SHIFT_BAND_OFFSET_CV_ATTENUVERTER_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CARRIER_IN,
		IN_MOD = CARRIER_IN + BANDS,
		IN_CARR,
		ATTACK_INPUT,
		DECAY_INPUT,
		CARRIER_Q_INPUT,
		MOD_Q_INPUT,
		SHIFT_BAND_OFFSET_LEFT_INPUT,
		SHIFT_BAND_OFFSET_RIGHT_INPUT,
		SHIFT_BAND_OFFSET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MOD_OUT,
		OUT = MOD_OUT + BANDS,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEARN_LIGHT,
		NUM_LIGHTS
	};
	Biquad* iFilter[2*BANDS];
	Biquad* cFilter[2*BANDS];
	float mem[BANDS] = {0};
	float freq[BANDS] = {125,185,270,350,430,530,630,780,950,1150,1380,1680,2070,2780,3800,6400};
	float peaks[BANDS] = {0};
	float lastCarrierQ = 0;
	float lastModQ = 0;

	int bandOffset = 0;
	int shiftIndex = 0;
	int lastBandOffset = 0;
	SchmittTrigger shiftLeftTrigger,shiftRightTrigger;

	MrBlueSky() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for(int i=0; i<2*BANDS; i++) {
			iFilter[i] = new Biquad(bq_type_bandpass, freq[i%BANDS] / engineGetSampleRate(), 5, 6);
			cFilter[i] = new Biquad(bq_type_bandpass, freq[i%BANDS] / engineGetSampleRate(), 5, 6);
		};
	}

	void step() override;

	void reset() override {
		bandOffset =0;
	}

};

void MrBlueSky::step() {
	// Band Offset Processing
	bandOffset = params[BAND_OFFSET_PARAM].value;
	if(inputs[SHIFT_BAND_OFFSET_INPUT].active) {
		bandOffset += inputs[SHIFT_BAND_OFFSET_INPUT].value * params[SHIFT_BAND_OFFSET_CV_ATTENUVERTER_PARAM].value;
	}
	if(bandOffset != lastBandOffset) {
		shiftIndex = 0;
		lastBandOffset = bandOffset;
	}

	if(inputs[SHIFT_BAND_OFFSET_LEFT_INPUT].active) {
		if (shiftLeftTrigger.process(inputs[SHIFT_BAND_OFFSET_LEFT_INPUT].value)) {
			shiftIndex -= 1;
			if(shiftIndex <= -BANDS) {
				shiftIndex = BANDS -1;
			}
		}
	}

	if(inputs[SHIFT_BAND_OFFSET_RIGHT_INPUT].active) {
		if (shiftRightTrigger.process(inputs[SHIFT_BAND_OFFSET_RIGHT_INPUT].value)) {
			shiftIndex += 1;
			if(shiftIndex >= BANDS) {
				shiftIndex = (-BANDS) + 1;
			}
		}
	}

	bandOffset +=shiftIndex;
	//Hack until I can do int clamping
	if(bandOffset <= -BANDS) {
		bandOffset += (BANDS*2) - 1;
	}
	if(bandOffset >= BANDS) {
		bandOffset -= (BANDS*2) + 1;
	}


	//So some vocoding!
	float inM = inputs[IN_MOD].value/5;
	float inC = inputs[IN_CARR].value/5;
	const float slewMin = 0.001;
	const float slewMax = 500.0;
	const float shapeScale = 1/10.0;
	const float qEpsilon = 0.1;
	float attack = params[ATTACK_PARAM].value;
	float decay = params[DECAY_PARAM].value;
	if(inputs[ATTACK_INPUT].active) {
		attack += clamp(inputs[ATTACK_INPUT].value * params[ATTACK_CV_ATTENUVERTER_PARAM].value / 20.0f,-0.25f,.25f);
	}
	if(inputs[DECAY_INPUT].active) {
		decay += clamp(inputs[DECAY_INPUT].value * params[DECAY_CV_ATTENUVERTER_PARAM].value / 20.0f,-0.25f,.25f);
	}
	float slewAttack = slewMax * powf(slewMin / slewMax, attack);
	float slewDecay = slewMax * powf(slewMin / slewMax, decay);
	float out = 0.0;

	//Check Mod Q
	float currentQ = params[MOD_Q_PARAM].value;
	if(inputs[MOD_Q_PARAM].active) {
		currentQ += inputs[MOD_Q_INPUT].value * params[MODIFER_Q_CV_ATTENUVERTER_PARAM].value;
	}

	currentQ = clamp(currentQ,1.0f,15.0f);
	if (abs(currentQ - lastModQ) >= qEpsilon ) {
		for(int i=0; i<2*BANDS; i++) {
			iFilter[i]->setQ(currentQ);
			}
		lastModQ = currentQ;
	}

	//Check Carrier Q
	currentQ = params[CARRIER_Q_PARAM].value;
	if(inputs[CARRIER_Q_INPUT].active) {
		currentQ += inputs[CARRIER_Q_INPUT].value * params[CARRIER_Q_CV_ATTENUVERTER_PARAM].value;
	}

	currentQ = clamp(currentQ,1.0f,15.0f);
	if (abs(currentQ - lastCarrierQ) >= qEpsilon ) {
		for(int i=0; i<2*BANDS; i++) {
			cFilter[i]->setQ(currentQ);
			}
		lastCarrierQ = currentQ;
	}



	//First process all the modifier bands
	for(int i=0; i<BANDS; i++) {
		float coeff = mem[i];
		float peak = abs(iFilter[i+BANDS]->process(iFilter[i]->process(inM*params[GMOD_PARAM].value)));
		if (peak>coeff) {
			coeff += slewAttack * shapeScale * (peak - coeff) / engineGetSampleRate();
			if (coeff > peak)
				coeff = peak;
		}
		else if (peak < coeff) {
			coeff -= slewDecay * shapeScale * (coeff - peak) / engineGetSampleRate();
			if (coeff < peak)
				coeff = peak;
		}
		peaks[i]=peak;
		mem[i]=coeff;
		outputs[MOD_OUT+i].value = coeff * 5.0;
	}

	//Then process carrier bands. Mod bands are normalled to their matched carrier band unless an insert
	for(int i=0; i<BANDS; i++) {
		float coeff;
		if(inputs[(CARRIER_IN+i+bandOffset) % BANDS].active) {
			coeff = inputs[CARRIER_IN+i+bandOffset].value / 5.0;
		} else {
			coeff = mem[(i+bandOffset) % BANDS];
		}

		float bandOut = cFilter[i+BANDS]->process(cFilter[i]->process(inC*params[GCARR_PARAM].value)) * coeff * params[BG_PARAM+i].value;
		out += bandOut;
	}
	outputs[OUT].value = out * 5 * params[G_PARAM].value;

}

struct MrBlueSkyBandDisplay : TransparentWidget {
	MrBlueSky *module;
	std::shared_ptr<Font> font;

	MrBlueSkyBandDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
	}

	void draw(NVGcontext *vg) override {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgStrokeWidth(vg, 2);
		nvgTextLetterSpacing(vg, -2);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		//static const int portX0[4] = {20, 63, 106, 149};
		for (int i=0; i<BANDS; i++) {
			char fVal[10];
			snprintf(fVal, sizeof(fVal), "%1i", (int)module->freq[i]);
			nvgFillColor(vg,nvgRGBA(rescale(clamp(module->peaks[i],0.0f,1.0f),0,1,0,255), 0, 0, 255));
			nvgText(vg, 56 + 33*i, 30, fVal, NULL);
		}
	}
};

struct BandOffsetDisplay : TransparentWidget {
	MrBlueSky *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	BandOffsetDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/01 Digit.ttf"));
	}

	void drawDuration(NVGcontext *vg, Vec pos, float bandOffset) {
		nvgFontSize(vg, 20);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), " % 2.0f", bandOffset);
		nvgText(vg, pos.x + 22, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {

		drawDuration(vg, Vec(0, box.size.y - 150), module->bandOffset);
	}
};

struct MrBlueSkyWidget : ModuleWidget {
	MrBlueSkyWidget(MrBlueSky *module);
};

MrBlueSkyWidget::MrBlueSkyWidget(MrBlueSky *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*39, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/MrBlueSky.svg")));
		addChild(panel);
	}

	MrBlueSkyBandDisplay *bandDisplay = new MrBlueSkyBandDisplay();
	bandDisplay->module = module;
	bandDisplay->box.pos = Vec(12, 12);
	bandDisplay->box.size = Vec(700, 70);
	addChild(bandDisplay);

	{
		BandOffsetDisplay *offsetDisplay = new BandOffsetDisplay();
		offsetDisplay->module = module;
		offsetDisplay->box.pos = Vec(435, 200);
		offsetDisplay->box.size = Vec(box.size.x, 150);
		addChild(offsetDisplay);
	}

	for (int i = 0; i < BANDS; i++) {
		addParam( ParamWidget::create<RoundBlackKnob>(Vec(53 + 33*i, 120), module, MrBlueSky::BG_PARAM + i, 0, 2, 1));
	}
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(34, 177), module, MrBlueSky::ATTACK_PARAM, 0.0, 0.25, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(116, 177), module, MrBlueSky::DECAY_PARAM, 0.0, 0.25, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(198, 177), module, MrBlueSky::CARRIER_Q_PARAM, 1.0, 15.0, 5.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(280, 177), module, MrBlueSky::MOD_Q_PARAM, 1.0, 15.0, 5.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(392, 177), module, MrBlueSky::BAND_OFFSET_PARAM, -15.5, 15.5, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(40, 284), module, MrBlueSky::GMOD_PARAM, 1, 10, 5));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(120, 284), module, MrBlueSky::GCARR_PARAM, 1, 10, 5));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(207, 284), module, MrBlueSky::G_PARAM, 1, 10, 5));

	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(37, 238), module, MrBlueSky::ATTACK_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(119, 238), module, MrBlueSky::DECAY_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(202, 238), module, MrBlueSky::CARRIER_Q_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(284, 238), module, MrBlueSky::MODIFER_Q_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(395, 238), module, MrBlueSky::SHIFT_BAND_OFFSET_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));


	for (int i = 0; i < BANDS; i++) {
		addInput(Port::create<PJ301MPort>(Vec(56 + 33*i, 85), Port::INPUT, module, MrBlueSky::CARRIER_IN + i));
	}
	addInput(Port::create<PJ301MPort>(Vec(42, 330), Port::INPUT, module, MrBlueSky::IN_MOD));
	addInput(Port::create<PJ301MPort>(Vec(122, 330), Port::INPUT, module, MrBlueSky::IN_CARR));
	addInput(Port::create<PJ301MPort>(Vec(36, 209), Port::INPUT, module, MrBlueSky::ATTACK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(118, 209), Port::INPUT, module, MrBlueSky::DECAY_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(201, 209), Port::INPUT, module, MrBlueSky::CARRIER_Q_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(283, 209), Port::INPUT, module, MrBlueSky::MOD_Q_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(362, 184), Port::INPUT, module, MrBlueSky::SHIFT_BAND_OFFSET_LEFT_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(425, 184), Port::INPUT, module, MrBlueSky::SHIFT_BAND_OFFSET_RIGHT_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(394, 209), Port::INPUT, module, MrBlueSky::SHIFT_BAND_OFFSET_INPUT));

	for (int i = 0; i < BANDS; i++) {
		addOutput(Port::create<PJ301MPort>(Vec(56 + 33*i, 45), Port::OUTPUT, module, MrBlueSky::MOD_OUT + i));
	}
	addOutput(Port::create<PJ301MPort>(Vec(210, 330), Port::OUTPUT, module, MrBlueSky::OUT));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, MrBlueSky) {
   Model *modelMrBlueSky = Model::create<MrBlueSky, MrBlueSkyWidget>("Frozen Wasteland", "MrBlueSky", "Mr. Blue Sky", EFFECT_TAG);
   return modelMrBlueSky;
}
