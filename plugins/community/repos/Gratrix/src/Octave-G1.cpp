//============================================================================================================
//!
//! \file Octave-G1.cpp
//!
//! \brief Octave-G1 quantises the input to 12-ET and provides an octaves-worth of output.
//!
//============================================================================================================


#include "Gratrix.hpp"

namespace rack_plugin_Gratrix {

//============================================================================================================
//! \brief Some settings.

enum Spec
{
	LO_BEGIN = -5,    // C-1
	LO_END   =  5,    // C+9
	LO_SIZE  = LO_END - LO_BEGIN + 1,
	E        = 12,    // ET
	N        = 12,    // Number of note outputs
	T        = 2,
	M        = 2*T+1  // Number of octave outputs
};


//============================================================================================================
//! \brief The module.

struct GtxModule_Octave_G1 : Module
{
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		VOCT_INPUT   = 0,
		NUM_INPUTS = VOCT_INPUT + 1
	};
	enum OutputIds {
		NOTE_OUTPUT = 0,
		OCT_OUTPUT  = NOTE_OUTPUT + N,
		NUM_OUTPUTS = OCT_OUTPUT  + M
	};
	enum LightIds {
		KEY_LIGHT  = 0,
		OCT_LIGHT  = KEY_LIGHT + E,
		NUM_LIGHTS = OCT_LIGHT + LO_SIZE
	};

	struct Decode
	{
		/*static constexpr*/ float e = static_cast<float>(E);  // Static constexpr gives
		/*static constexpr*/ float s = 1.0f / e;               // link error on Mac build.

		float in    = 0;  //!< Raw input.
		float out   = 0;  //!< Input quantized.
		int   note  = 0;  //!< Integer note (offset midi note).
		int   key   = 0;  //!< C, C#, D, D#, etc.
		int   oct   = 0;  //!< Octave (C4 = 0).

		void step(float input)
		{
			int safe, fnote;

			in    = input;
			fnote = std::floor(in * e + 0.5f);
			out   = fnote * s;
			note  = static_cast<int>(fnote);
			safe  = note + (E * 1000);  // push away from negative numbers
			key   = safe % E;
			oct   = (safe / E) - 1000;
		}
	};

	Decode input;

	//--------------------------------------------------------------------------------------------------------
	//! \brief Constructor.

	GtxModule_Octave_G1()
	:
		Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{}

	//--------------------------------------------------------------------------------------------------------
	//! \brief Step function.

	void step() override
	{
		// Clear all lights

		float leds[NUM_LIGHTS] = {};

		// Decode inputs and params
		input.step(inputs[VOCT_INPUT].value);

		for (std::size_t i=0; i<N; ++i)
		{
			outputs[i + NOTE_OUTPUT].value = input.out + i * input.s;
		}

		for (std::size_t i=0; i<M; ++i)
		{
			outputs[i + OCT_OUTPUT].value = (input.out - T) + i;
		}

		// Lights

		leds[KEY_LIGHT + input.key] = 1.0f;

		if (LO_BEGIN <= input.oct && input.oct <= LO_END)
		{
			leds[OCT_LIGHT + input.oct - LO_BEGIN] = 1.0f;
		}

		// Write output in one go, seems to prevent flicker

		for (std::size_t i=0; i<NUM_LIGHTS; ++i)
		{
			lights[i].value = leds[i];
		}
	}
};


static int x(std::size_t i, double radius) { return static_cast<int>(6*15     + 0.5 + radius * dx(i, E)); }
static int y(std::size_t i, double radius) { return static_cast<int>(-20+206  + 0.5 + radius * dy(i, E)); }


//============================================================================================================
//! \brief The widget.

struct GtxWidget_Octave_G1 : ModuleWidget
{
	GtxWidget_Octave_G1(GtxModule_Octave_G1 *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(12*15, 380);

	//	double r1 = 30;
		double r2 = 55;

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Octave-G1.svg"), box.size, "OCTAVE-G1");

			pg.circle(Vec(x(0, 0), y(0, 0)), r2+16, "fill:#7092BE;stroke:none");
			pg.circle(Vec(x(0, 0), y(0, 0)), r2-16, "fill:#CEE1FD;stroke:none");

	/*		// Wires
			for (std::size_t i=0; i<N; ++i)
			{
						 pg.line(Vec(x(i,   r1), y(i,   r1)), Vec(x(i, r2), y(i, r2)), "stroke:#440022;stroke-width:1");
				if (i) { pg.line(Vec(x(i-1, r1), y(i-1, r1)), Vec(x(i, r1), y(i, r1)), "stroke:#440022;stroke-width:1"); }
			}
	*/
			// Ports
			pg.circle(Vec(x(0, 0), y(0, 0)), 10, "stroke:#440022;stroke-width:1");
			for (std::size_t i=0; i<N; ++i)
			{
				 pg.circle(Vec(x(i, r2), y(i,   r2)), 10, "stroke:#440022;stroke-width:1");
			}

			pg.prt_out(-0.20, 2,          "", "-2");
			pg.prt_out( 0.15, 2,          "", "-1");
			pg.prt_out( 0.50, 2, "TRANSPOSE",  "0");
			pg.prt_out( 0.85, 2,          "", "+1");
			pg.prt_out( 1.20, 2,          "", "+2");
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Octave-G1.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		addInput(createInputGTX<PortInMed>(Vec(x(0, 0), y(0, 0)), module, GtxModule_Octave_G1::VOCT_INPUT));
		for (std::size_t i=0; i<N; ++i)
		{
			addOutput(createOutputGTX<PortOutMed>(Vec(x(i, r2), y(i, r2)), module, i + GtxModule_Octave_G1::NOTE_OUTPUT));
		}

		addOutput(createOutputGTX<PortOutMed>(Vec(gx(-0.20), gy(2)), module, 0 + GtxModule_Octave_G1::OCT_OUTPUT));
		addOutput(createOutputGTX<PortOutMed>(Vec(gx( 0.15), gy(2)), module, 1 + GtxModule_Octave_G1::OCT_OUTPUT));
		addOutput(createOutputGTX<PortOutMed>(Vec(gx( 0.50), gy(2)), module, 2 + GtxModule_Octave_G1::OCT_OUTPUT));
		addOutput(createOutputGTX<PortOutMed>(Vec(gx( 0.85), gy(2)), module, 3 + GtxModule_Octave_G1::OCT_OUTPUT));
		addOutput(createOutputGTX<PortOutMed>(Vec(gx( 1.20), gy(2)), module, 4 + GtxModule_Octave_G1::OCT_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) - 30, fy(0-0.28) + 5), module, GtxModule_Octave_G1::KEY_LIGHT +  0));  // C
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) - 25, fy(0-0.28) - 5), module, GtxModule_Octave_G1::KEY_LIGHT +  1));  // C#
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) - 20, fy(0-0.28) + 5), module, GtxModule_Octave_G1::KEY_LIGHT +  2));  // D
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) - 15, fy(0-0.28) - 5), module, GtxModule_Octave_G1::KEY_LIGHT +  3));  // Eb
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) - 10, fy(0-0.28) + 5), module, GtxModule_Octave_G1::KEY_LIGHT +  4));  // E
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5)     , fy(0-0.28) + 5), module, GtxModule_Octave_G1::KEY_LIGHT +  5));  // F
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) +  5, fy(0-0.28) - 5), module, GtxModule_Octave_G1::KEY_LIGHT +  6));  // Fs
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) + 10, fy(0-0.28) + 5), module, GtxModule_Octave_G1::KEY_LIGHT +  7));  // G
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) + 15, fy(0-0.28) - 5), module, GtxModule_Octave_G1::KEY_LIGHT +  8));  // Ab
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) + 20, fy(0-0.28) + 5), module, GtxModule_Octave_G1::KEY_LIGHT +  9));  // A
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) + 25, fy(0-0.28) - 5), module, GtxModule_Octave_G1::KEY_LIGHT + 10));  // Bb
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) + 30, fy(0-0.28) + 5), module, GtxModule_Octave_G1::KEY_LIGHT + 11));  // B

		for (std::size_t i=0; i<LO_SIZE; ++i)
		{
			addChild(ModuleLightWidget::create<SmallLight<RedLight>>(l_s(gx(0.5) + (i - LO_SIZE/2) * 10, fy(0-0.28) + 20), module, GtxModule_Octave_G1::OCT_LIGHT + i));
		}
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, Octave_G1) {
   Model *model = Model::create<GtxModule_Octave_G1, GtxWidget_Octave_G1>("Gratrix", "Octave-G1", "Octave-G1", SYNTH_VOICE_TAG);  // right tag?
   return model;
}
