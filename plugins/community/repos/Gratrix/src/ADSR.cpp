#include "Gratrix.hpp"
#include "dsp/digital.hpp"


namespace rack_plugin_Gratrix {

//============================================================================================================

struct ADSR : MicroModule {
	enum ParamIds {
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ATTACK_INPUT,     // 1
		DECAY_INPUT,      // 1
		SUSTAIN_INPUT,    // 1
		RELEASE_INPUT,    // 1
		GATE_INPUT,       // N+1
		TRIG_INPUT,       // N+1
		NUM_INPUTS,
		OFF_INPUTS = GATE_INPUT
	};
	enum OutputIds {
		ENVELOPE_OUTPUT,  // N
		INVERTED_OUTPUT,  // N
		NUM_OUTPUTS,
		OFF_OUTPUTS = ENVELOPE_OUTPUT
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool decaying = false;
	float env = 0.0f;
	SchmittTrigger trigger;

	ADSR() : MicroModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step();
};


//============================================================================================================

void ADSR::step() {
	float attack = clamp(params[ATTACK_INPUT].value + inputs[ATTACK_INPUT].value / 10.0f, 0.0f, 1.0f);
	float decay = clamp(params[DECAY_PARAM].value + inputs[DECAY_INPUT].value / 10.0f, 0.0f, 1.0f);
	float sustain = clamp(params[SUSTAIN_PARAM].value + inputs[SUSTAIN_INPUT].value / 10.0f, 0.0f, 1.0f);
	float release = clamp(params[RELEASE_PARAM].value + inputs[RELEASE_PARAM].value / 10.0f, 0.0f, 1.0f);

	// Gate and trigger
	bool gated = inputs[GATE_INPUT].value >= 1.0f;
	if (trigger.process(inputs[TRIG_INPUT].value))
		decaying = false;

	const float base = 20000.0f;
	const float maxTime = 10.0f;
	if (gated) {
		if (decaying) {
			// Decay
			if (decay < 1e-4) {
				env = sustain;
			}
			else {
				env += powf(base, 1 - decay) / maxTime * (sustain - env) * engineGetSampleTime();
			}
		}
		else {
			// Attack
			// Skip ahead if attack is all the way down (infinitely fast)
			if (attack < 1e-4) {
				env = 1.0f;
			}
			else {
				env += powf(base, 1 - attack) / maxTime * (1.01f - env) * engineGetSampleTime();
			}
			if (env >= 1.0f) {
				env = 1.0f;
				decaying = true;
			}
		}
	}
	else {
		// Release
		if (release < 1e-4) {
			env = 0.0f;
		}
		else {
			env += powf(base, 1 - release) / maxTime * (0.0f - env) * engineGetSampleTime();
		}
		decaying = false;
	}

	outputs[ENVELOPE_OUTPUT].value = 10.0 * env;
	outputs[INVERTED_OUTPUT].value = 10.0 * (1.0 - env);
}


//============================================================================================================

struct GtxModule_ADSR : Module
{
	std::array<ADSR, GTX__N> inst;

	GtxModule_ADSR()
	:
		Module(ADSR::NUM_PARAMS,
			(GTX__N+1) * (ADSR::NUM_INPUTS  - ADSR::OFF_INPUTS ) + ADSR::OFF_INPUTS,
			(GTX__N  ) * (ADSR::NUM_OUTPUTS - ADSR::OFF_OUTPUTS) + ADSR::OFF_OUTPUTS)
	{}

	static constexpr std::size_t imap(std::size_t port, std::size_t bank)
	{
		return (port < ADSR::OFF_INPUTS)  ? port : port + bank * (ADSR::NUM_INPUTS  - ADSR::OFF_INPUTS);
	}

	static constexpr std::size_t omap(std::size_t port, std::size_t bank)
	{
	//	return (port < ADSR::OFF_OUTPUTS) ? port : port + bank * (ADSR::NUM_OUTPUTS - ADSR::OFF_OUTPUTS);
		return                                     port + bank *  ADSR::NUM_OUTPUTS;
	}

	void step() override
	{
		for (std::size_t i=0; i<GTX__N; ++i)
		{
			for (std::size_t p=0; p<ADSR::NUM_PARAMS;  ++p) inst[i].params[p]  = params[p];
			for (std::size_t p=0; p<ADSR::NUM_INPUTS;  ++p) inst[i].inputs[p]  = inputs[imap(p, i)].active ? inputs[imap(p, i)] : inputs[imap(p, GTX__N)];
			for (std::size_t p=0; p<ADSR::NUM_OUTPUTS; ++p) inst[i].outputs[p] = outputs[omap(p, i)];

			inst[i].step();

			for (std::size_t p=0; p<ADSR::NUM_OUTPUTS; ++p) outputs[omap(p, i)].value = inst[i].outputs[p].value;
		}
	}
};


//============================================================================================================

struct GtxWidget_ADSR : ModuleWidget
{
	GtxWidget_ADSR(GtxModule_ADSR *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(12*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/ADSR-F1.svg"), box.size, "ADSR-F1");

			pg.nob_med(0, -0.28, "ATTACK");  pg.nob_med(1, -0.28, "DECAY");
			pg.nob_med(0, +0.28, "SUSTAIN"); pg.nob_med(1, +0.28, "RELEASE");

			pg.bus_in(0, 1, "GATE");  pg.bus_out(1, 1, "OUT");
			pg.bus_in(0, 2, "TRIG");  pg.bus_out(1, 2, "INV OUT");
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/ADSR-F1.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		addParam(createParamGTX<KnobFreeMed>(Vec(fx(0+0.18), fy(-0.28)), module, ADSR::ATTACK_PARAM,  0.0, 1.0, 0.5));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1+0.18), fy(-0.28)), module, ADSR::DECAY_PARAM,   0.0, 1.0, 0.5));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(0+0.18), fy(+0.28)), module, ADSR::SUSTAIN_PARAM, 0.0, 1.0, 0.5));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1+0.18), fy(+0.28)), module, ADSR::RELEASE_PARAM, 0.0, 1.0, 0.5));

		addInput(createInputGTX<PortInMed>(Vec(fx(0-0.28), fy(-0.28)), module, ADSR::ATTACK_INPUT));
		addInput(createInputGTX<PortInMed>(Vec(fx(1-0.28), fy(-0.28)), module, ADSR::DECAY_INPUT));
		addInput(createInputGTX<PortInMed>(Vec(fx(0-0.28), fy(+0.28)), module, ADSR::SUSTAIN_INPUT));
		addInput(createInputGTX<PortInMed>(Vec(fx(1-0.28), fy(+0.28)), module, ADSR::RELEASE_INPUT));

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(1, i)), module, GtxModule_ADSR::imap(ADSR::GATE_INPUT, i)));
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(2, i)), module, GtxModule_ADSR::imap(ADSR::TRIG_INPUT, i)));

			addOutput(createOutputGTX<PortOutMed>(Vec(px(1, i), py(1, i)), module, GtxModule_ADSR::omap(ADSR::ENVELOPE_OUTPUT, i)));
			addOutput(createOutputGTX<PortOutMed>(Vec(px(1, i), py(2, i)), module, GtxModule_ADSR::omap(ADSR::INVERTED_OUTPUT, i)));
		}

		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(1)), module, GtxModule_ADSR::imap(ADSR::GATE_INPUT, GTX__N)));
		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(2)), module, GtxModule_ADSR::imap(ADSR::TRIG_INPUT, GTX__N)));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, ADSR_F1) {
   Model *model = Model::create<GtxModule_ADSR, GtxWidget_ADSR>("Gratrix", "ADSR-F1", "ADSR-F1", ENVELOPE_GENERATOR_TAG);
   return model;
}
