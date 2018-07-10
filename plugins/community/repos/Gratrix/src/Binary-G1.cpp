//============================================================================================================
//!
//! \file Binary-G1.cpp
//!
//! \brief Binary-G1 provides some simple logic gates.
//!
//============================================================================================================


#include "Gratrix.hpp"


namespace rack_plugin_Gratrix {

//============================================================================================================
//! \brief The module.

struct GtxModule_Binary_G1 : Module
{
	enum ParamIds {
		INVERT_A_PARAM,
		INVERT_B_PARAM,
		FUNCTION_AB_1_PARAM,
		FUNCTION_AB_2_PARAM,
		INVERT_1_PARAM,
		INVERT_2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_A_INPUT,
		IN_B_INPUT,
		NUM_INPUTS,
		OFF_INPUTS = IN_A_INPUT
	};
	enum OutputIds {
		OUT_1_OUTPUT,
		OUT_2_OUTPUT,
		NUM_OUTPUTS,
		OFF_OUTPUTS = OUT_1_OUTPUT
	};
	enum LightIds {
		FUNCTION_0_AB_1_LIGHT,
		FUNCTION_1_AB_1_LIGHT,
		FUNCTION_2_AB_1_LIGHT,
		FUNCTION_3_AB_1_LIGHT,
		FUNCTION_4_AB_1_LIGHT,
		FUNCTION_0_AB_2_LIGHT,
		FUNCTION_1_AB_2_LIGHT,
		FUNCTION_2_AB_2_LIGHT,
		FUNCTION_3_AB_2_LIGHT,
		FUNCTION_4_AB_2_LIGHT,
		NUM_LIGHTS
	};

	GtxModule_Binary_G1()
	:
		Module(NUM_PARAMS,
		(GTX__N+1) * (NUM_INPUTS  - OFF_INPUTS ) + OFF_INPUTS,
		(GTX__N  ) * (NUM_OUTPUTS - OFF_OUTPUTS) + OFF_OUTPUTS,
		NUM_LIGHTS)
	{
	}

	static constexpr std::size_t imap(std::size_t port, std::size_t bank)
	{
		return port + bank * NUM_INPUTS;
	}

	static constexpr std::size_t omap(std::size_t port, std::size_t bank)
	{
		return port + bank * NUM_OUTPUTS;
	}

	void step() override
	{
		float leds[NUM_LIGHTS] = {};

		int fn1 = FUNCTION_0_AB_1_LIGHT + static_cast<int>(params[FUNCTION_AB_1_PARAM].value + 0.5f);
		int fn2 = FUNCTION_0_AB_2_LIGHT + static_cast<int>(params[FUNCTION_AB_2_PARAM].value + 0.5f);

		if (fn1 >= FUNCTION_0_AB_1_LIGHT && fn1 <= FUNCTION_4_AB_1_LIGHT) leds[fn1] = 1.0;
		if (fn2 >= FUNCTION_0_AB_2_LIGHT && fn2 <= FUNCTION_4_AB_2_LIGHT) leds[fn2] = 1.0;

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			bool inA = (inputs[imap(IN_A_INPUT, i)].active ? inputs[imap(IN_A_INPUT, i)].value : inputs[imap(IN_A_INPUT, GTX__N)].value) >= 1.0f;
			bool inB = (inputs[imap(IN_B_INPUT, i)].active ? inputs[imap(IN_B_INPUT, i)].value : inputs[imap(IN_B_INPUT, GTX__N)].value) >= 1.0f;

			inA  ^= (params[INVERT_A_PARAM].value < 0.5f);
			inB  ^= (params[INVERT_B_PARAM].value < 0.5f);

			bool out1, out2;

			switch (fn1)
			{
				case FUNCTION_0_AB_1_LIGHT : out1 = inA      ; break;
				case FUNCTION_1_AB_1_LIGHT : out1 =       inB; break;
				case FUNCTION_2_AB_1_LIGHT : out1 = inA & inB; break;
				case FUNCTION_3_AB_1_LIGHT : out1 = inA | inB; break;
				case FUNCTION_4_AB_1_LIGHT : out1 = inA ^ inB; break;
				default                    : out1 = false;
			}

			switch (fn2)
			{
				case FUNCTION_0_AB_2_LIGHT : out2 = inA      ; break;
				case FUNCTION_1_AB_2_LIGHT : out2 =       inB; break;
				case FUNCTION_2_AB_2_LIGHT : out2 = inA & inB; break;
				case FUNCTION_3_AB_2_LIGHT : out2 = inA | inB; break;
				case FUNCTION_4_AB_2_LIGHT : out2 = inA ^ inB; break;
				default                    : out2 = false;
			}

			out1 ^= (params[INVERT_1_PARAM].value < 0.5f);
			out2 ^= (params[INVERT_2_PARAM].value < 0.5f);

			outputs[omap(OUT_1_OUTPUT, i)].value = out1 ? 10.0f : 0.0f;
			outputs[omap(OUT_2_OUTPUT, i)].value = out2 ? 10.0f : 0.0f;
		}

		for (std::size_t i=0; i<NUM_LIGHTS; ++i)
		{
			lights[i].value = leds[i];
		}
	}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_Binary_G1 : ModuleWidget
{
	GtxWidget_Binary_G1(GtxModule_Binary_G1 *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(12*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Binary-G1.svg"), box.size, "BINARY-G1");

			pg.nob_sml_raw(fx(1 - 0.75) - 3, fy(-0.28), "OP 1");
			pg.nob_sml_raw(fx(1 - 0.75) - 3, fy(+0.28), "OP 2");

			pg.tog(1 - 1.27, -0.28, "A", "INV A");  pg.tog(1.27, -0.28, "1", "INV 1");
			pg.tog(1 - 1.27, +0.28, "B", "INV B");  pg.tog(1.27, +0.28, "2", "INV 2");

			pg.text(Vec(fx(0.72) - 5, fy(-0.28) - 6 * rad_l_s() + 3), "A",   10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(-0.28) - 3 * rad_l_s() + 3), "B",   10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(-0.28) - 0 * rad_l_s() + 3), "AND", 10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(-0.28) + 3 * rad_l_s() + 3), "OR",  10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(-0.28) + 6 * rad_l_s() + 3), "XOR", 10, "start");

			pg.text(Vec(fx(0.72) - 5, fy(+0.28) - 6 * rad_l_s() + 3), "A",   10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(+0.28) - 3 * rad_l_s() + 3), "B",   10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(+0.28) - 0 * rad_l_s() + 3), "AND", 10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(+0.28) + 3 * rad_l_s() + 3), "OR",  10, "start");
			pg.text(Vec(fx(0.72) - 5, fy(+0.28) + 6 * rad_l_s() + 3), "XOR", 10, "start");

			pg.bus_in(0, 1, "IN A"); pg.bus_out(1, 1, "OUT 1");
			pg.bus_in(0, 2, "IN B"); pg.bus_out(1, 2, "OUT 2");
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Binary-G1.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		addParam(ParamWidget::create<CKSS>(  tog(fx(1 - 1.27)    , fy(-0.28)), module, GtxModule_Binary_G1::INVERT_A_PARAM,      0.0, 1.0, 1.0));
		addParam(ParamWidget::create<CKSS>(  tog(fx(1 - 1.27)    , fy(+0.28)), module, GtxModule_Binary_G1::INVERT_B_PARAM,      0.0, 1.0, 1.0));
		addParam(createParamGTX<KnobSnapSml>(Vec(fx(1 - 0.75) - 3, fy(-0.28)), module, GtxModule_Binary_G1::FUNCTION_AB_1_PARAM, 0.0, 4.0, 2.0));
		addParam(createParamGTX<KnobSnapSml>(Vec(fx(1 - 0.75) - 3, fy(+0.28)), module, GtxModule_Binary_G1::FUNCTION_AB_2_PARAM, 0.0, 4.0, 2.0));
		addParam(ParamWidget::create<CKSS>(  tog(fx(    1.27)    , fy(-0.28)), module, GtxModule_Binary_G1::INVERT_1_PARAM,      0.0, 1.0, 1.0));
		addParam(ParamWidget::create<CKSS>(  tog(fx(    1.27)    , fy(+0.28)), module, GtxModule_Binary_G1::INVERT_2_PARAM,      0.0, 1.0, 1.0));

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(1, i)), module, GtxModule_Binary_G1::imap(GtxModule_Binary_G1::IN_A_INPUT, i)));
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(2, i)), module, GtxModule_Binary_G1::imap(GtxModule_Binary_G1::IN_B_INPUT, i)));

			addOutput(createOutputGTX<PortOutMed>(Vec(px(1, i), py(1, i)), module, GtxModule_Binary_G1::omap(GtxModule_Binary_G1::OUT_1_OUTPUT, i)));
			addOutput(createOutputGTX<PortOutMed>(Vec(px(1, i), py(2, i)), module, GtxModule_Binary_G1::omap(GtxModule_Binary_G1::OUT_2_OUTPUT, i)));
		}

		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(1)), module, GtxModule_Binary_G1::imap(GtxModule_Binary_G1::IN_A_INPUT, GTX__N)));
		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(2)), module, GtxModule_Binary_G1::imap(GtxModule_Binary_G1::IN_B_INPUT, GTX__N)));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(-0.28) - 6 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_0_AB_1_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(-0.28) - 3 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_1_AB_1_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(-0.28) - 0 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_2_AB_1_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(-0.28) + 3 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_3_AB_1_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(-0.28) + 6 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_4_AB_1_LIGHT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(+0.28) - 6 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_0_AB_2_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(+0.28) - 3 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_1_AB_2_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(+0.28) - 0 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_2_AB_2_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(+0.28) + 3 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_3_AB_2_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(l_s(fx(0.72) - 2.5 * rad_l_s() - 5, fy(+0.28) + 6 * rad_l_s()), module, GtxModule_Binary_G1::FUNCTION_4_AB_2_LIGHT));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, Binary_G1) {
   Model *model = Model::create<GtxModule_Binary_G1, GtxWidget_Binary_G1>("Gratrix", "Binary-G1", "Binary-G1", LOGIC_TAG);
   return model;
}
