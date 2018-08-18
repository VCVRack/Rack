#include "rack.hpp"
#include "Biquad.h"
#include "VAStateVariableFilter.h"

using namespace rack;

////////////////////
// module widgets
////////////////////
RACK_PLUGIN_DECLARE(Autodafe);

#ifndef RACK_PLUGIN_SHARED
#define plugin "Autodafe"
#endif // RACK_PLUGIN_SHARED

namespace rack_plugin_Autodafe {
 

/////////////////////////////
// CUSTOM KNOBS & GRAPHICS //
/////////////////////////////

struct AutodafeKnobRed : SVGKnob {
    AutodafeKnobRed() {
        box.size = Vec(20, 20);
        minAngle = -0.75*M_PI;
        maxAngle = 0.75*M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobRed.svg")));
    }
};

struct AutodafeKnobRedBig : SVGKnob {
    AutodafeKnobRedBig() {
        box.size = Vec(35, 35);
        minAngle = -0.75*M_PI;
        maxAngle = 0.75*M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobRedBig.svg")));
    }
};

struct AutodafeKnobBlue : SVGKnob {
	AutodafeKnobBlue() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobBlue.svg")));
	}
};

struct AutodafeKnobBlueBig : SVGKnob {
	AutodafeKnobBlueBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobBlueBig.svg")));
	}
};

struct AutodafeKnobGreen : SVGKnob {
	AutodafeKnobGreen() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobGreen.svg")));
	}
};

struct AutodafeKnobGreenBig : SVGKnob {
	AutodafeKnobGreenBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobGreenBig.svg")));
	}
};

struct AutodafeKnobPurple : SVGKnob {
	AutodafeKnobPurple() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobPurple.svg")));
	}
};

struct AutodafeKnobPurpleSmall : SVGKnob {
	AutodafeKnobPurpleSmall() {
		box.size = Vec(15, 15);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobPurpleSmall.svg")));
	}
};

struct AutodafeKnobPurpleBig : SVGKnob {
	AutodafeKnobPurpleBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobPurpleBig.svg")));
	}
};

struct AutodafeKnobWhite : SVGKnob {
	AutodafeKnobWhite() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobWhite.svg")));
	}
};

struct AutodafeKnobWhiteBig : SVGKnob {
	AutodafeKnobWhiteBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobWhiteBig.svg")));
	}
};

struct AutodafeKnobBrown : SVGKnob {
	AutodafeKnobBrown() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobBrown.svg")));
	}
};

struct AutodafeKnobBrownBig : SVGKnob {
	AutodafeKnobBrownBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobBrownBig.svg")));
	}
};

struct AutodafeKnobBlack : SVGKnob {
	AutodafeKnobBlack() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobBlack.svg")));
	}

};

struct AutodafeKnobBlackSmall : SVGKnob {
	AutodafeKnobBlackSmall() {
		box.size = Vec(15, 15);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobBlackSmall.svg")));
	}

};

struct AutodafeKnobBlackBig : SVGKnob {
	AutodafeKnobBlackBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobBlackBig.svg")));
	}
};

struct AutodafeKnobOrange : SVGKnob {
	AutodafeKnobOrange() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobOrange.svg")));
	}
};

struct AutodafeKnobOrangeBig : SVGKnob {
	AutodafeKnobOrangeBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobOrangeBig.svg")));
	}
};

struct AutodafeKnobYellow : SVGKnob {
	AutodafeKnobYellow() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobYellow.svg")));
	}
};

struct AutodafeKnobYellowBig : SVGKnob {
	AutodafeKnobYellowBig() {
		box.size = Vec(35, 35);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/AutodafeKnobYellowBig.svg")));
	}
};

struct AutodafeButton : SVGSwitch, MomentarySwitch {
    AutodafeButton() {

    	addFrame(SVG::load(assetPlugin(plugin, "res/AutodafeButton.svg")));
        //SVG::load("res/AutodafeButton.svg");
		
		box.size = Vec(20,20);
      
    } 
};   

struct BtnUp : SVGSwitch, MomentarySwitch {
	BtnUp() {
		addFrame(SVG::load(assetPlugin(plugin, "res/BtnUp.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BtnDwn : SVGSwitch, MomentarySwitch {
	BtnDwn() {
		addFrame(SVG::load(assetPlugin(plugin, "res/BtnDwn.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BtnTrigSequencer : SVGSwitch, MomentarySwitch {
	BtnTrigSequencer() {
		addFrame(SVG::load(assetPlugin(plugin, "res/BtnTrigSequencer.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
 
struct BtnTrigSequencerSmall : SVGSwitch, MomentarySwitch {
	BtnTrigSequencerSmall() {
		addFrame(SVG::load(assetPlugin(plugin, "res/BtnTrigSequencerSmall.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct WhiteKey : SVGSwitch, MomentarySwitch {
	WhiteKey() {
		addFrame(SVG::load(assetPlugin(plugin, "res/WhiteKey.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BlackKey : SVGSwitch, MomentarySwitch {
	BlackKey() {
		addFrame(SVG::load(assetPlugin(plugin, "res/BlackKey.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};



///////////////////////////////////
///      WIDGETS                ///
///////////////////////////////////


//// EFFECTS

struct FlangerFxWidget : ModuleWidget{
	FlangerFxWidget();
};


struct EchoFxWidget : ModuleWidget{
	EchoFxWidget();
};

struct PitchShifterFxWidget : ModuleWidget{
	PitchShifterFxWidget();
};


struct CompressorWidget : ModuleWidget{
	CompressorWidget();
};

struct KnobDemoWidget : ModuleWidget{
	KnobDemoWidget();
};



///OSCILLATORS

struct WavesModelWidget : ModuleWidget{
	WavesModelWidget();
};


struct SquareVCOModelWidget : ModuleWidget{
	SquareVCOModelWidget();
};

struct AMVCOModelWidget : ModuleWidget{
	AMVCOModelWidget();
};

struct FMVCOModelWidget : ModuleWidget{
	FMVCOModelWidget();
};

struct CosineVCOModelWidget : ModuleWidget{
	CosineVCOModelWidget();
};

struct OPERATORModelWidget : ModuleWidget{
	OPERATORModelWidget();
};

struct DWOVCOModelWidget : ModuleWidget{
	DWOVCOModelWidget();
};

struct SamplerModelWidget : ModuleWidget{
	SamplerModelWidget();
};

struct DrumSamplerWidget : ModuleWidget{
	DrumSamplerWidget();
	Menu *createContextMenu() override;
	
};

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;
