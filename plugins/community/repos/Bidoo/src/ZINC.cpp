#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/decimator.hpp"
#include "dep/filters/biquad.h"

using namespace std;

namespace rack_plugin_Bidoo {

#define BANDS 16

struct ZINC : Module {
	enum ParamIds {
		BG_PARAM,
		ATTACK_PARAM = BG_PARAM + BANDS,
		DECAY_PARAM,
		Q_PARAM,
		GMOD_PARAM,
		GCARR_PARAM,
		G_PARAM,
		SHAPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_MOD,
		IN_CARR,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEARN_LIGHT,
		NUM_LIGHTS
	};
	Biquad* iFilter[2*BANDS];
	Biquad* cFilter[2*BANDS];
	float mem[BANDS] = {0.0f};
	float freq[BANDS] = {125.0f,185.0f,270.0f,350.0f,430.0f,530.0f,630.0f,780.0f,950.0f,1150.0f,1380.0f,1680.0f,2070.0f,2780.0f,3800.0f,6400.0f};
	float peaks[BANDS] = {0.0f};

	ZINC() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for(int i=0; i<2*BANDS; i++) {
			iFilter[i] = new Biquad(bq_type_bandpass, freq[i%BANDS] / engineGetSampleRate(), 5.0, 6.0);
			cFilter[i] = new Biquad(bq_type_bandpass, freq[i%BANDS] / engineGetSampleRate(), 5.0, 6.0);
			}
	}

	void step() override;

};

void ZINC::step() {
	float inM = inputs[IN_MOD].value/5.0f;
	float inC = inputs[IN_CARR].value/5.0f;
	const float slewMin = 0.001f;
	const float slewMax = 500.0f;
	const float shapeScale = 1.0f/10.0f;
	float attack = params[ATTACK_PARAM].value;
	float decay = params[DECAY_PARAM].value;
	float slewAttack = slewMax * powf(slewMin / slewMax, attack);
	float slewDecay = slewMax * powf(slewMin / slewMax, decay);
	float out = 0.0f;

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
		out += cFilter[i+BANDS]->process(cFilter[i]->process(inC*params[GCARR_PARAM].value)) * coeff * params[BG_PARAM+i].value;
	}
	outputs[OUT].value = out * 5.0f * params[G_PARAM].value;
}

struct ZINCDisplay : TransparentWidget {
	ZINC *module;
	std::shared_ptr<Font> font;

	ZINCDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
		nvgStrokeWidth(vg, 2);
		nvgTextLetterSpacing(vg, -2);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		static const int portX0[4] = {20, 63, 106, 149};
		for (int i=0; i<BANDS; i++) {
			char fVal[10];
			snprintf(fVal, sizeof(fVal), "%1i", (int)module->freq[i]);
			nvgFillColor(vg,nvgRGBA(0, 0, 0, 255));
			nvgText(vg, portX0[i%(BANDS/4)]+1, 35+43*(int)(i/4), fVal, NULL);
		}
	}
};

struct ZINCWidget : ModuleWidget {
	ParamWidget *controls[16];
	void step() override;

	ZINCWidget(ZINC *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/ZINC.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		ZINCDisplay *display = new ZINCDisplay();
		display->module = module;
		display->box.pos = Vec(12, 12);
		display->box.size = Vec(110, 70);
		addChild(display);

		static const float portX0[4] = {20, 63, 106, 149};

		for (int i = 0; i < BANDS; i++) {
			controls[i]=ParamWidget::create<BidooziNCColoredKnob>(Vec(portX0[i%(BANDS/4)]+2, 50+43*(int)(i/4)+2), module, ZINC::BG_PARAM + i, 0, 2, 1);
			BidooziNCColoredKnob *control = dynamic_cast<BidooziNCColoredKnob*>(controls[i]);
			control->coeff=module->peaks+i;
			addParam(controls[i]);
		}
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[1]+4, 230), module, ZINC::ATTACK_PARAM, 0.0, 0.25, 0.0));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[2]+4, 230), module, ZINC::DECAY_PARAM, 0.0, 0.25, 0.0));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[0]+20, 268), module, ZINC::GMOD_PARAM, 1, 10, 1));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[1]+20, 268), module, ZINC::GCARR_PARAM, 1, 10, 1));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[2]+20, 268), module, ZINC::G_PARAM, 1, 10, 1));

		addInput(Port::create<PJ301MPort>(Vec(portX0[0]+27.5, 320), Port::INPUT, module, ZINC::IN_MOD));
		addInput(Port::create<PJ301MPort>(Vec(portX0[1]+22.5, 320), Port::INPUT, module, ZINC::IN_CARR));
		addOutput(Port::create<PJ301MPort>(Vec(portX0[2]+16.5, 320), Port::OUTPUT, module, ZINC::OUT));
	}
};

void ZINCWidget::step() {
	for (int i = 0; i < BANDS; i++) {
			BidooziNCColoredKnob* knob = dynamic_cast<BidooziNCColoredKnob*>(controls[i]);
			knob->dirty = true;
	}
	ModuleWidget::step();
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, ZINC) {
   Model *modelZINC = Model::create<ZINC, ZINCWidget>("Bidoo", "ziNC", "ziNC vocoder", EFFECT_TAG);
   return modelZINC;
}
