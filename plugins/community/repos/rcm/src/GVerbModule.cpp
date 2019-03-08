#include "rcm.h"
#include "GVerbWidget.hpp"
#include "../include/BaseWidget.hpp"

// extern "C" 
// {
	#include "../include/gverb.h"
// }

namespace rack_plugin_rcm {

struct Follower {
	float level = 0.f;

	void step(float* left, float* right) {
		auto value = max(abs(*left), abs(*right));

		if (value >= level) {
			level = value;
		} else {
			level -= (level - value) * 0.001;
		}

		if (level > 10.f) {
			*left /= (level / 10.f);
			*right /= (level / 10.f);
		}
	}
};

struct GVerbModule : Module {
	enum ParamIds {
		ROOM_SIZE_PARAM,
		REV_TIME_PARAM,
		DAMPING_PARAM,
		SPREAD_PARAM,
		BANDWIDTH_PARAM,
		EARLY_LEVEL_PARAM,
		TAIL_LEVEL_PARAM,
		MIX_PARAM,
		RESET_PARAM,
		ROOM_SIZE_POT_PARAM,
		DAMPING_POT_PARAM,
		REV_TIME_POT_PARAM,
		BANDWIDTH_POT_PARAM,
		EARLY_LEVEL_POT_PARAM,
		TAIL_LEVEL_POT_PARAM,
		MIX_POT_PARAM,
		SPREAD_POT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LEFT_AUDIO,
		RIGHT_AUDIO,
		ROOM_SIZE_INPUT,
		DAMPING_INPUT,
		REV_TIME_INPUT,
		BANDWIDTH_INPUT,
		EARLY_LEVEL_INPUT,
		TAIL_LEVEL_INPUT,
		MIX_INPUT,
		SPREAD_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	ty_gverb* gverbL = NULL;
	ty_gverb* gverbR = NULL;

	GVerbModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;
	void disposeGverbL();
	void disposeGverbR();

	float p_frequency = 0.f;
	float p_room_size = 0.f;
	float p_rev_time = 0.f;
	float p_damping = 0.f;
	float p_bandwidth = 0.f;
	float p_early_level = 0.f;
	float p_tail_level = 0.f;
	float p_reset = 0.f;

	Follower follower;

	float getParam(ParamIds param, InputIds mod, ParamIds trim, float min, float max);
	void handleParam(float value, float* store, void (*change)(ty_gverb*,float));

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void GVerbModule::disposeGverbL() {
	if (gverbL != NULL) {
		gverb_free(gverbL);
		gverbL = NULL;
	}
}

void GVerbModule::disposeGverbR() {
	if (gverbR != NULL) {
		gverb_free(gverbR);
		gverbR = NULL;
	}
}

float GVerbModule::getParam(ParamIds param, InputIds mod, ParamIds trim, float min, float max) {
	return clamp2(params[param].value + (((clamp2(inputs[mod].value, -10.f, 10.f)/10) * max) * params[trim].value), min, max);
}

void GVerbModule::handleParam(float value, float* store, void (*change)(ty_gverb*,float)) {
	if (*store != value) {
		if (gverbL != NULL) {
			change(gverbL, value);
		}
		if (gverbR != NULL) {
			change(gverbR, value);
		}
		*store = value;
	}
}

void GVerbModule::step() {
	auto reset = max(params[RESET_PARAM].value, inputs[RESET_INPUT].value);
	auto mix = getParam(MIX_PARAM, MIX_INPUT, MIX_POT_PARAM, 0.f, 1.f);

	if (p_frequency != engineGetSampleRate()) { disposeGverbL(); disposeGverbR(); }
	if (p_reset == 0.f && reset > 0.f) {
		disposeGverbL();
		disposeGverbR();
	}

	if (gverbL != NULL && !inputs[LEFT_AUDIO].active) {
		disposeGverbL();
	}

	if (gverbR != NULL && !inputs[RIGHT_AUDIO].active) {
		disposeGverbR();
	}

	p_reset = reset;

	if (gverbL == NULL) {

		if (inputs[LEFT_AUDIO].active) {
			gverbL = gverb_new(
				engineGetSampleRate(), // freq
				300,    // max room size
				params[ROOM_SIZE_PARAM].value,    // room size
				params[REV_TIME_PARAM].value,     // revtime
				params[DAMPING_PARAM].value,   // damping
				90.0,   // spread
				params[BANDWIDTH_PARAM].value,     // input bandwidth
				params[EARLY_LEVEL_PARAM].value,   // early level
				params[TAIL_LEVEL_PARAM].value    // tail level
			);

			p_frequency = engineGetSampleRate();
		}
	}

	if (gverbR == NULL) {

		if (inputs[RIGHT_AUDIO].active) {
			gverbR = gverb_new(
				engineGetSampleRate(), // freq
				300,    // max room size
				params[ROOM_SIZE_PARAM].value,    // room size
				params[REV_TIME_PARAM].value,     // revtime
				params[DAMPING_PARAM].value,   // damping
				90.0,   // spread
				params[BANDWIDTH_PARAM].value,     // input bandwidth
				params[EARLY_LEVEL_PARAM].value,   // early level
				params[TAIL_LEVEL_PARAM].value    // tail level
			);

			p_frequency = engineGetSampleRate();
		}
	}


	if (gverbL != NULL || gverbR != NULL) {
		handleParam(getParam(ROOM_SIZE_PARAM, ROOM_SIZE_INPUT, ROOM_SIZE_POT_PARAM, 2.f, 300.f), &p_room_size, gverb_set_roomsize);
		handleParam(getParam(REV_TIME_PARAM, REV_TIME_INPUT, REV_TIME_POT_PARAM, 0.f, 10000.f), &p_rev_time, gverb_set_revtime);
		handleParam(getParam(DAMPING_PARAM, DAMPING_INPUT, DAMPING_POT_PARAM, 0.f, 1.f), &p_damping, gverb_set_damping);
		handleParam(getParam(BANDWIDTH_PARAM, BANDWIDTH_INPUT, BANDWIDTH_POT_PARAM, 0.f, 1.f), &p_bandwidth, gverb_set_inputbandwidth);
		handleParam(getParam(EARLY_LEVEL_PARAM, EARLY_LEVEL_INPUT, EARLY_LEVEL_POT_PARAM, 0.f, 1.f), &p_early_level, gverb_set_earlylevel);
		handleParam(getParam(TAIL_LEVEL_PARAM, TAIL_LEVEL_INPUT, TAIL_LEVEL_POT_PARAM, 0.f, 1.f), &p_tail_level, gverb_set_taillevel);

		auto engineCount = gverbL != NULL && gverbR != NULL ? 2 : 1;
		auto spread = getParam(SPREAD_PARAM, SPREAD_INPUT, SPREAD_POT_PARAM, 0.f, 1.f);
		auto L_L = 0.f, L_R = 0.f;
		auto R_L = 0.f, R_R = 0.f;

		if (gverbL != NULL) {
			gverb_do(gverbL, inputs[LEFT_AUDIO].value / 10.f, &L_L, &L_R);

			L_L = isfinite(L_L) ? L_L * 10.f : 0.f;
			L_R = isfinite(L_R) ? L_R * 10.f : 0.f;
		}

		auto L_L_S = (L_L + ((1-spread) * L_R)) / (2-spread);
		auto L_R_S = (L_R + ((1-spread) * L_L)) / (2-spread);

		if (gverbR != NULL) {
			gverb_do(gverbR, inputs[RIGHT_AUDIO].value / 10.f, &R_L, &R_R);

			R_L = isfinite(R_L) ? R_L * 10.f : 0.f;
			R_R = isfinite(R_R) ? R_R * 10.f : 0.f;
		}

		auto R_L_S = (R_L + ((1-spread) * R_R)) / (2-spread);
		auto R_R_S = (R_R + ((1-spread) * R_L)) / (2-spread);

		outputs[LEFT_OUTPUT].value = L_L_S + R_L_S / engineCount;
		outputs[RIGHT_OUTPUT].value = L_R_S + R_R_S / engineCount;

		follower.step(&outputs[LEFT_OUTPUT].value, &outputs[RIGHT_OUTPUT].value);

		outputs[LEFT_OUTPUT].value = ((1 - mix) * (inputs[LEFT_AUDIO].active ? inputs[LEFT_AUDIO].value : inputs[RIGHT_AUDIO].value)) + (mix * outputs[LEFT_OUTPUT].value);
		outputs[RIGHT_OUTPUT].value = ((1 - mix) * (inputs[RIGHT_AUDIO].active ? inputs[RIGHT_AUDIO].value : inputs[LEFT_AUDIO].value)) + (mix * outputs[RIGHT_OUTPUT].value);

	} else {
		outputs[LEFT_OUTPUT].value = outputs[RIGHT_OUTPUT].value = 0.f;
	}
}

struct GVerbModuleWidget : BaseWidget {
	GVerbModuleWidget(GVerbModule *module) : BaseWidget(module) {
		colourHotZone = Rect(Vec(111.572, 10), Vec(46.856, 13));
		backgroundHue = 0.06667f;
		backgroundSaturation = 1.f;
		backgroundLuminosity = 0.58f;

		setPanel(SVG::load(assetPlugin(plugin, "res/Reverb.svg")));

		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(50, 44), module, GVerbModule::ROOM_SIZE_PARAM, 2.0, 300.0, 20.0));
		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(50, 115), module, GVerbModule::DAMPING_PARAM, 0.0, 1.0, 0.98));

		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(127, 60), module, GVerbModule::REV_TIME_PARAM, 0.0, 10000.0, 1.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(127, 120), module, GVerbModule::BANDWIDTH_PARAM, 0.0, 1.0, 0.01));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(185, 60), module, GVerbModule::EARLY_LEVEL_PARAM, 0.0, 1.0, 0.8));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(185, 120), module, GVerbModule::TAIL_LEVEL_PARAM, 0.0, 1.0, 0.5));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(84, 189), module, GVerbModule::MIX_PARAM, 0.0, 1.0, 0.4));
		addParam(ParamWidget::create<RoundBlackKnob>(Vec(135, 189), module, GVerbModule::SPREAD_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<PB61303>(Vec(186, 189), module, GVerbModule::RESET_PARAM, 0.0, 1.0, 0.0));

		addParam(ParamWidget::create<Trimpot>(Vec(15, 263), module, GVerbModule::ROOM_SIZE_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(42, 263), module, GVerbModule::DAMPING_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(70, 263), module, GVerbModule::REV_TIME_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(97, 263), module, GVerbModule::BANDWIDTH_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(124, 263), module, GVerbModule::EARLY_LEVEL_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(151, 263), module, GVerbModule::TAIL_LEVEL_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(178, 263), module, GVerbModule::MIX_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(205, 263), module, GVerbModule::SPREAD_POT_PARAM, -1.f, 1.f, 0.f));

		addInput(Port::create<PJ301MPort>(Vec(14, 286), Port::INPUT, module, GVerbModule::ROOM_SIZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(41, 286), Port::INPUT, module, GVerbModule::DAMPING_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(68, 286), Port::INPUT, module, GVerbModule::REV_TIME_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(95, 286), Port::INPUT, module, GVerbModule::BANDWIDTH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(123, 286), Port::INPUT, module, GVerbModule::EARLY_LEVEL_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(150, 286), Port::INPUT, module, GVerbModule::TAIL_LEVEL_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(177, 286), Port::INPUT, module, GVerbModule::MIX_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(204, 286), Port::INPUT, module, GVerbModule::SPREAD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(232, 286), Port::INPUT, module, GVerbModule::RESET_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(14, 332), Port::INPUT, module, GVerbModule::LEFT_AUDIO));
		addInput(Port::create<PJ301MPort>(Vec(41, 332), Port::INPUT, module, GVerbModule::RIGHT_AUDIO));

		addOutput(Port::create<PJ301MPort>(Vec(204, 332), Port::OUTPUT, module, GVerbModule::LEFT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(232, 332), Port::OUTPUT, module, GVerbModule::RIGHT_OUTPUT));
	}
};

} // namespace rack_plugin_rcm

using namespace rack_plugin_rcm;

RACK_PLUGIN_MODEL_INIT(rcm, GVerbModule) {
   Model *modelGVerbModule = Model::create<GVerbModule, GVerbModuleWidget>("rcm", "rcm-gverb", "GVerb", REVERB_TAG);
   return modelGVerbModule;
}
