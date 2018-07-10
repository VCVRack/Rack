#include "Gratrix.hpp"


namespace rack_plugin_Gratrix {

//============================================================================================================

struct VCA : MicroModule {
	enum ParamIds {
		LEVEL_PARAM,
		MIX_1_PARAM,
		MIX_2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		EXP_INPUT,     // N+1
		LIN_INPUT,     // N+1
		IN_INPUT,      // N+1
		NUM_INPUTS,
		OFF_INPUTS = EXP_INPUT
	};
	enum OutputIds {
		MIX_1_OUTPUT,  // 1
		MIX_2_OUTPUT,  // 1
		OUT_OUTPUT,    // N
		NUM_OUTPUTS,
		OFF_OUTPUTS = OUT_OUTPUT
	};

	VCA() : MicroModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step();
};


//============================================================================================================

void VCA::step() {
	float v = inputs[IN_INPUT].value * params[LEVEL_PARAM].value;
	if (inputs[LIN_INPUT].active)
		v *= clamp(inputs[LIN_INPUT].value / 10.0f, 0.0f, 1.0f);
	const float expBase = 50.0f;
	if (inputs[EXP_INPUT].active)
		v *= rescale(powf(expBase, clamp(inputs[EXP_INPUT].value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
	outputs[OUT_OUTPUT].value = v;
}


//============================================================================================================

struct VCABank : Module
{
	std::array<VCA, GTX__N> inst;

	VCABank()
	:
		Module(VCA::NUM_PARAMS,
			(GTX__N+1) * (VCA::NUM_INPUTS  - VCA::OFF_INPUTS ) + VCA::OFF_INPUTS,
			(GTX__N  ) * (VCA::NUM_OUTPUTS - VCA::OFF_OUTPUTS) + VCA::OFF_OUTPUTS)
	{}

	static constexpr std::size_t imap(std::size_t port, std::size_t bank)
	{
		return port + bank * VCA::NUM_INPUTS;
	}

	static constexpr std::size_t omap(std::size_t port, std::size_t bank)
	{
		return (port < VCA::OFF_OUTPUTS) ? port : port + bank * (VCA::NUM_OUTPUTS - VCA::OFF_OUTPUTS);
	}

	void step() override
	{
		for (std::size_t i=0; i<GTX__N; ++i)
		{
			for (std::size_t p=0; p<VCA::NUM_PARAMS;  ++p) inst[i].params[p]  = params[p];
			for (std::size_t p=0; p<VCA::NUM_INPUTS;  ++p) inst[i].inputs[p]  = inputs[imap(p, i)].active ? inputs[imap(p, i)] : inputs[imap(p, GTX__N)];
			for (std::size_t p=0; p<VCA::NUM_OUTPUTS; ++p) inst[i].outputs[p] = outputs[omap(p, i)];

			inst[i].step();

			for (std::size_t p=0; p<VCA::NUM_OUTPUTS; ++p) outputs[omap(p, i)].value = inst[i].outputs[p].value;
		}

		float mix = 0.0f;

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			mix += inst[i].outputs[VCA::OUT_OUTPUT].value;
		}

		outputs[VCA::MIX_1_OUTPUT].value = mix * params[VCA::MIX_1_PARAM].value;
		outputs[VCA::MIX_2_OUTPUT].value = mix * params[VCA::MIX_2_PARAM].value;
	}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_VCA : ModuleWidget
{
	GtxWidget_VCA(VCABank *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(12*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/VCA-F1.svg"), box.size, "VCA-F1");

			pg.nob_big(0, 0, "LEVEL");

			pg.nob_med(1, -0.28, "MIX OUT 1");
			pg.nob_med(1, +0.28, "MIX OUT 2");

			pg.bus_in (0, 1, "EXP"); pg.bus_in (1, 1, "LIN");
			pg.bus_in (0, 2, "IN");  pg.bus_out(1, 2, "OUT");
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/VCA-F1.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		addParam(createParamGTX<KnobFreeHug>(Vec(fx(0),      fy(0)),     module, VCA::LEVEL_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1-0.18), fy(-0.28)), module, VCA::MIX_1_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1-0.18), fy(+0.28)), module, VCA::MIX_2_PARAM, 0.0f, 1.0f, 0.5f));

		addOutput(createOutputGTX<PortOutMed>(Vec(fx(1+0.28), fy(-0.28)), module, VCA::MIX_1_OUTPUT));
		addOutput(createOutputGTX<PortOutMed>(Vec(fx(1+0.28), fy(+0.28)), module, VCA::MIX_2_OUTPUT));

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			addInput(createInputGTX<PortInMed>(Vec(px(1, i), py(1, i)), module, VCABank::imap(VCA::LIN_INPUT, i)));
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(1, i)), module, VCABank::imap(VCA::EXP_INPUT, i)));
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(2, i)), module, VCABank::imap(VCA::IN_INPUT,  i)));

			addOutput(createOutputGTX<PortOutMed>(Vec(px(1, i), py(2, i)), module, VCABank::omap(VCA::OUT_OUTPUT, i)));
		}

		addInput(createInputGTX<PortInMed>(Vec(gx(1), gy(1)), module, VCABank::imap(VCA::LIN_INPUT, GTX__N)));
		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(1)), module, VCABank::imap(VCA::EXP_INPUT, GTX__N)));
		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(2)), module, VCABank::imap(VCA::IN_INPUT,  GTX__N)));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, VCA_F1) {
   Model *model = Model::create<VCABank, GtxWidget_VCA>("Gratrix", "VCA-F1", "VCA-F1", AMPLIFIER_TAG);
   return model;
}
