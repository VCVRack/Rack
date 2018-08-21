/*

FlipOLogic

clock divider and logic gate module that provides a variety of ways to create
interesting rhythmic patterns by combining/comparing different clocksignals.

"main" input (center, below the logic outis feeding a series of 3 flipflops,
creating /2 (FLIP), /4 (FLOP) , /8 (FLAP) divisions of the original clock signal.
it's also the default input for logic input B.
FLOP is fed into the left (AND) column input and into logic input A.
FLAP is fed into the right (XOR) column input and into logic input C.

so, with one clock signal the logic section compares the main input vs
the FLOP and FLAP divisions and then the logic outs get
AND compared on the left side vs the left input and 
XOR compared on the right side vs the right input.

this internal routing can be broken with other external sources or by
repatching the module itself for even more interesting combinations.

*/////////////////////////////////////////////////////////////////////////////


#include "pvc.hpp"

#include "dsp/digital.hpp" // SchmittTrigger // PulseGenerator

namespace rack_plugin_PvC {

struct FlipOLogic : Module {
	enum ParamIds {
		// IDEA: logic mode selectors for the left and right column
		// IDEA: left and right column outputs source selectors (previous or input)
		NUM_PARAMS
	};
	enum InputIds {
		FLIP_IN,
		LEFT_IN,
		RIGHT_IN,
		LGC_A_IN,
		LGC_B_IN,
		LGC_C_IN,

		NUM_INPUTS
	};
	enum OutputIds {
		FLIP_OUT,
		FLOP_OUT,
		FLAP_OUT,

		LGC_AND_OUT,
		LGC_NAND_OUT,
		LGC_OR_OUT,
		LGC_NOR_OUT,
		LGC_XOR_OUT,
		LGC_XNOR_OUT,

		LEFT_VS_AND_OUT,
		LEFT_VS_NAND_OUT,
		LEFT_VS_OR_OUT,
		LEFT_VS_NOR_OUT,
		LEFT_VS_XOR_OUT,
		LEFT_VS_XNOR_OUT,

		RIGHT_VS_AND_OUT,
		RIGHT_VS_NAND_OUT,
		RIGHT_VS_OR_OUT,
		RIGHT_VS_NOR_OUT,
		RIGHT_VS_XOR_OUT,
		RIGHT_VS_XNOR_OUT,

		NUM_OUTPUTS
	};
	enum LightIds {
		FLIP_LED,
		FLOP_LED,
		FLAP_LED,

		LGC_AND_LED,
		LGC_NAND_LED,
		LGC_OR_LED,
		LGC_NOR_LED,
		LGC_XOR_LED,
		LGC_XNOR_LED,

		LEFT_VS_AND_LED,
		LEFT_VS_NAND_LED,
		LEFT_VS_OR_LED,
		LEFT_VS_NOR_LED,
		LEFT_VS_XOR_LED,
		LEFT_VS_XNOR_LED,

		RIGHT_VS_AND_LED,
		RIGHT_VS_NAND_LED,
		RIGHT_VS_OR_LED,
		RIGHT_VS_NOR_LED,
		RIGHT_VS_XOR_LED,
		RIGHT_VS_XNOR_LED,

		NUM_LIGHTS
	};

	bool flpIn = false;

	bool flip = false;
	bool flop = false;
	bool flap = false;

	bool leftIn = false;
	bool rightIn = false;

	bool lgcInA = false;
	bool lgcInB = false;
	bool lgcInC = false;

	bool lgcAnd = false;
	bool lgcNand = false;
	bool lgcOr = false;
	bool lgcNor = false;
	bool lgcXor = false;
	bool lgcXnor = false;

	SchmittTrigger flipTrigger;
	SchmittTrigger flopTrigger;
	SchmittTrigger flapTrigger;

	FlipOLogic() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

	void reset() override {
		flpIn = false;
		flip = flop = flap = false;
		leftIn = rightIn = false;
		lgcInA = lgcInB = lgcInC = false;
		lgcAnd = lgcNand = lgcOr = lgcNor = lgcXor = lgcXnor = false;
	}
};


void FlipOLogic::step() {
	flpIn = inputs[FLIP_IN].value > 0 ? true : false;

	flip = flipTrigger.process(flpIn) ? !flip : flip;
	flop = flopTrigger.process(flip) ? !flop : flop;
	flap = flapTrigger.process(flop) ? !flap : flap;

	leftIn = inputs[LEFT_IN].normalize(flop) > 0 ? true : false;
	rightIn = inputs[RIGHT_IN].normalize(flap) > 0 ? true : false;

	lgcInA = inputs[LGC_A_IN].normalize(leftIn) > 0 ? true : false;
	lgcInB = inputs[LGC_B_IN].normalize(flip) > 0 ? true : false;
	lgcInC = inputs[LGC_C_IN].normalize(rightIn) > 0 ? true : false;

	lgcAnd = lgcInA && lgcInB && lgcInC;
	lgcNand = !lgcAnd;
	lgcOr = lgcInA || lgcInB || lgcInC;
	lgcNor = !lgcInA && !lgcInB && !lgcInC;
	lgcXor = (lgcInA && !lgcInB && !lgcInC) || (!lgcInA && lgcInB && !lgcInC) || (!lgcInA && !lgcInB && lgcInC);
	lgcXnor = !lgcXor;

	outputs[FLIP_OUT].value = flip * 10.0f;
	lights[FLIP_LED].value = flip;
	outputs[FLOP_OUT].value = flop * 10.0f;
	lights[FLOP_LED].value = flop;
	outputs[FLAP_OUT].value = flap * 10.0f;
	lights[FLAP_LED].value = flap;

	outputs[LGC_AND_OUT].value = lgcAnd * 10.0f;
	lights[LGC_AND_LED].value = lgcAnd;
	outputs[LGC_NAND_OUT].value = lgcNand * 10.0f;
	lights[LGC_NAND_LED].value = lgcNand;
	outputs[LGC_OR_OUT].value = lgcOr * 10.0f;
	lights[LGC_OR_LED].value = lgcOr;
	outputs[LGC_NOR_OUT].value = lgcNor * 10.0f;
	lights[LGC_NOR_LED].value = lgcNor;
	outputs[LGC_XOR_OUT].value = lgcXor * 10.0f;
	lights[LGC_XOR_LED].value = lgcXor;
	outputs[LGC_XNOR_OUT].value = lgcXnor * 10.0f;
	lights[LGC_XNOR_LED].value = lgcXnor;

	outputs[LEFT_VS_AND_OUT].value = (leftIn && lgcAnd) * 10.0f;
	outputs[LEFT_VS_NAND_OUT].value = (leftIn && lgcNand) * 10.0f;
	outputs[LEFT_VS_OR_OUT].value = (leftIn && lgcOr) * 10.0f;
	outputs[LEFT_VS_NOR_OUT].value = (leftIn && lgcNor) * 10.0f;
	outputs[LEFT_VS_XOR_OUT].value = (leftIn && lgcXor) * 10.0f;
	outputs[LEFT_VS_XNOR_OUT].value = (leftIn && lgcXnor) * 10.0f;
	lights[LEFT_VS_AND_LED].value = (leftIn && lgcAnd);
	lights[LEFT_VS_NAND_LED].value = (leftIn && lgcNand);
	lights[LEFT_VS_OR_LED].value = (leftIn && lgcOr);
	lights[LEFT_VS_NOR_LED].value = (leftIn && lgcNor);
	lights[LEFT_VS_XOR_LED].value = (leftIn && lgcXor);
	lights[LEFT_VS_XNOR_LED].value = (leftIn && lgcXnor);

	outputs[RIGHT_VS_AND_OUT].value = (rightIn != lgcAnd) * 10.0f;
	outputs[RIGHT_VS_NAND_OUT].value = (rightIn != lgcNand) * 10.0f;
	outputs[RIGHT_VS_OR_OUT].value = (rightIn != lgcOr) * 10.0f;
	outputs[RIGHT_VS_NOR_OUT].value = (rightIn != lgcNor) * 10.0f;
	outputs[RIGHT_VS_XOR_OUT].value = (rightIn != lgcXor) * 10.0f;
	outputs[RIGHT_VS_XNOR_OUT].value = (rightIn != lgcXnor) * 10.0f;
	lights[RIGHT_VS_AND_LED].value = (rightIn != lgcAnd);
	lights[RIGHT_VS_NAND_LED].value = (rightIn != lgcNand);
	lights[RIGHT_VS_OR_LED].value = (rightIn != lgcOr);
	lights[RIGHT_VS_NOR_LED].value = (rightIn != lgcNor);
	lights[RIGHT_VS_XOR_LED].value = (rightIn != lgcXor);
	lights[RIGHT_VS_XNOR_LED].value = (rightIn != lgcXnor);
}


struct FlipOLogicWidget : ModuleWidget {
	FlipOLogicWidget(FlipOLogic *module);
};

FlipOLogicWidget::FlipOLogicWidget(FlipOLogic *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/FlipOLogic.svg")));

	// screws
	//addChild(Widget::create<ScrewHead1>(Vec(0, 0)));
	addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewHead3>(Vec(0, 365)));
	//addChild(Widget::create<ScrewHead4>(Vec(box.size.x - 15, 365)));

	addInput(Port::create<InPortBin>(Vec(4,22), Port::INPUT, module,FlipOLogic::LGC_A_IN));
	addInput(Port::create<InPortBin>(Vec(34,22), Port::INPUT, module,FlipOLogic::LGC_B_IN));
	addInput(Port::create<InPortBin>(Vec(64,22), Port::INPUT, module,FlipOLogic::LGC_C_IN));

	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(13,55),module,FlipOLogic::LEFT_VS_AND_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,60), Port::OUTPUT, module,FlipOLogic::LEFT_VS_AND_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,65),module,FlipOLogic::LGC_AND_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,70), Port::OUTPUT, module,FlipOLogic::LGC_AND_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(73,55),module,FlipOLogic::RIGHT_VS_AND_LED));
	addOutput(Port::create<OutPortBin>(Vec(64,60), Port::OUTPUT, module,FlipOLogic::RIGHT_VS_AND_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(13,95),module,FlipOLogic::LEFT_VS_NAND_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,100), Port::OUTPUT, module,FlipOLogic::LEFT_VS_NAND_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,105),module,FlipOLogic::LGC_NAND_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,110), Port::OUTPUT, module,FlipOLogic::LGC_NAND_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(73,95),module,FlipOLogic::RIGHT_VS_NAND_LED));
	addOutput(Port::create<OutPortBin>(Vec(64,100), Port::OUTPUT, module,FlipOLogic::RIGHT_VS_NAND_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(13,135),module,FlipOLogic::LEFT_VS_OR_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,140), Port::OUTPUT, module,FlipOLogic::LEFT_VS_OR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,145),module,FlipOLogic::LGC_OR_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,150), Port::OUTPUT, module,FlipOLogic::LGC_OR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(73,135),module,FlipOLogic::RIGHT_VS_OR_LED));
	addOutput(Port::create<OutPortBin>(Vec(64,140), Port::OUTPUT, module,FlipOLogic::RIGHT_VS_OR_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(13,175),module,FlipOLogic::LEFT_VS_NOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,180), Port::OUTPUT, module,FlipOLogic::LEFT_VS_NOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,185),module,FlipOLogic::LGC_NOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,190), Port::OUTPUT, module,FlipOLogic::LGC_NOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(73,175),module,FlipOLogic::RIGHT_VS_NOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(64,180), Port::OUTPUT, module,FlipOLogic::RIGHT_VS_NOR_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(13,215),module,FlipOLogic::LEFT_VS_XOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,220), Port::OUTPUT, module,FlipOLogic::LEFT_VS_XOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,225),module,FlipOLogic::LGC_XOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,230), Port::OUTPUT, module,FlipOLogic::LGC_XOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(73,215),module,FlipOLogic::RIGHT_VS_XOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(64,220), Port::OUTPUT, module,FlipOLogic::RIGHT_VS_XOR_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(13,255),module,FlipOLogic::LEFT_VS_XNOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,260), Port::OUTPUT, module,FlipOLogic::LEFT_VS_XNOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,265),module,FlipOLogic::LGC_XNOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,270), Port::OUTPUT, module,FlipOLogic::LGC_XNOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(73,255),module,FlipOLogic::RIGHT_VS_XNOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(64,260), Port::OUTPUT, module,FlipOLogic::RIGHT_VS_XNOR_OUT));

	addInput(Port::create<InPortBin>(Vec(4,294), Port::INPUT, module,FlipOLogic::LEFT_IN));
	addInput(Port::create<InPortBin>(Vec(34,298), Port::INPUT, module,FlipOLogic::FLIP_IN));
	addInput(Port::create<InPortBin>(Vec(64,294), Port::INPUT, module,FlipOLogic::RIGHT_IN));

	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(13,331),module,FlipOLogic::FLOP_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,336), Port::OUTPUT, module,FlipOLogic::FLOP_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,331),module,FlipOLogic::FLIP_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,336), Port::OUTPUT, module,FlipOLogic::FLIP_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(73,331),module,FlipOLogic::FLAP_LED));
	addOutput(Port::create<OutPortBin>(Vec(64,336), Port::OUTPUT, module,FlipOLogic::FLAP_OUT));
}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, FlipOLogic) {
   Model *modelFlipOLogic = Model::create<FlipOLogic, FlipOLogicWidget>(
      "PvC", "FlipOLogic", "FlipOLogic", LOGIC_TAG, SWITCH_TAG, CLOCK_MODULATOR_TAG);
   return modelFlipOLogic;
}
