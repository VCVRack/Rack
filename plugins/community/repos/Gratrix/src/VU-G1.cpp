//============================================================================================================
//!
//! \file VU-G1.cpp
//!
//! \brief VU-G1 is a simple six input volume monitoring module.
//!
//============================================================================================================


#include "Gratrix.hpp"


namespace rack_plugin_Gratrix {

//============================================================================================================
//! \brief The module.

struct GtxModule_VU_G1 : Module
{
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,      // N+1
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS,
	};
	enum LightIds {
		NUM_LIGHTS = 10  // N
	};

	GtxModule_VU_G1()
	:
		Module(NUM_PARAMS, (GTX__N+1) * NUM_INPUTS, NUM_OUTPUTS, GTX__N * NUM_LIGHTS)
	{
	}

	static constexpr std::size_t imap(std::size_t port, std::size_t bank)
	{
		return port + bank * NUM_INPUTS;
	}

	void step() override
	{
		for (std::size_t i=0; i<GTX__N; ++i)
		{
			float input = inputs[imap(IN1_INPUT, i)].active ? inputs[imap(IN1_INPUT, i)].value : inputs[imap(IN1_INPUT, GTX__N)].value;
			float dB    = logf(fabsf(input * 0.1f)) * (10.0f / logf(20.0f));
			float dB2   = dB * (1.0f / 3.0f);

			for (int j = 0; j < NUM_LIGHTS; j++)
			{
				float b = clamp(dB2 + (j+1), 0.0f, 1.0f);

				lights[NUM_LIGHTS * i + j].setBrightnessSmooth(b * 0.9f);
			}
		}
	}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_VU_G1 : ModuleWidget
{
	GtxWidget_VU_G1(GtxModule_VU_G1 *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(6*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/VU-G1.svg"), box.size, "VU-G1");

			pg.nob_big(0, 0, "VOLUME");
			pg.bus_in (0, 2, "IN");
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/VU-G1.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(2, i)), module, GtxModule_VU_G1::imap(GtxModule_VU_G1::IN1_INPUT, i)));
		}

		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(2)), module, GtxModule_VU_G1::imap(GtxModule_VU_G1::IN1_INPUT, GTX__N)));

		for (std::size_t i=0, k=0; i<GTX__N; ++i)
		{
			for (std::size_t j=0; j<GtxModule_VU_G1::NUM_LIGHTS; ++j, ++k)
			{
				switch (j)
				{
					case 0  : addChild(ModuleLightWidget::create<SmallLight<   RedLight>>(l_s(gx(0)+(i-2.5f)*13.0f, gy(0.5)+(j-4.5f)*11.0f), module, k)); break;
					case 1  :
					case 2  : addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(l_s(gx(0)+(i-2.5f)*13.0f, gy(0.5)+(j-4.5f)*11.0f), module, k)); break;
					default : addChild(ModuleLightWidget::create<SmallLight< GreenLight>>(l_s(gx(0)+(i-2.5f)*13.0f, gy(0.5)+(j-4.5f)*11.0f), module, k)); break;
				}
			}
		}
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, VU_G1) {
   Model *model = Model::create<GtxModule_VU_G1, GtxWidget_VU_G1>("Gratrix", "VU-G1", "VU-G1", VISUAL_TAG);
   return model;
}
