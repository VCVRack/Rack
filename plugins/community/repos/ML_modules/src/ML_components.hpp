#include "rack.hpp"

#pragma once

using namespace rack;

RACK_PLUGIN_DECLARE(ML_modules);

#ifdef USE_VST2
#define plugin "ML_modules"
#endif // USE_VST2



struct MLSVGSwitch : virtual ParamWidget, FramebufferWidget {

	CircularShadow *shadow;

	std::vector<std::shared_ptr<SVG>> frames;
	SVGWidget *sw;

	MLSVGSwitch();
	/** Adds an SVG file to represent the next switch position */

	void addFrame(std::shared_ptr<SVG> svg);
	void onChange(EventChange &e) override;

};



template <typename BASE>
struct MLLargeLight : BASE {
	MLLargeLight() {
		this->box.size = Vec(16.0, 16.0);
	}
};

template <typename BASE>
struct MLMediumLight : BASE {
	MLMediumLight() {
		this->box.size = Vec(12.0, 12.0);
	}
};


template <typename BASE>
struct MLSmallLight : BASE {
	MLSmallLight() {
		this->box.size = Vec(8.0, 8.0);
	}
};

struct WhiteLight : ModuleLightWidget {
	WhiteLight();
};


struct BlueMLKnob : RoundKnob {
        BlueMLKnob();
};

struct SmallBlueMLKnob : RoundKnob {
        SmallBlueMLKnob();
};

struct BlueSnapMLKnob : RoundKnob {
        BlueSnapMLKnob();
};

struct SmallBlueSnapMLKnob : RoundKnob {
        SmallBlueSnapMLKnob();
};


struct RedMLKnob : RoundKnob {
        RedMLKnob();
};

struct SmallRedMLKnob : RoundKnob {
        SmallRedMLKnob();
};

struct RedSnapMLKnob : RoundKnob {
        RedSnapMLKnob();
};

struct SmallRedSnapMLKnob : RoundKnob {
        SmallRedSnapMLKnob();
};



struct GreyMLKnob : RoundKnob {
        GreyMLKnob();
};

struct SmallGreyMLKnob : RoundKnob {
        SmallGreyMLKnob();
};


struct GreySnapMLKnob : RoundKnob {
        GreySnapMLKnob();
};

struct SmallGreySnapMLKnob : RoundKnob {
        SmallGreySnapMLKnob();
};


struct MLPort : SVGPort {
	MLPort();
};


struct MLButton : MLSVGSwitch, MomentarySwitch {
	MLButton();
};

struct MLSmallButton : MLSVGSwitch, MomentarySwitch {
	MLSmallButton();
};

struct ML_ResetButton : MLSVGSwitch, MomentarySwitch {
	ML_ResetButton();
};

struct ML_LEDButton : MLSVGSwitch, MomentarySwitch {
	
	ML_LEDButton();
};

struct ML_MediumLEDButton : MLSVGSwitch, MomentarySwitch {
	
	ML_MediumLEDButton();
};


struct ML_SmallLEDButton : MLSVGSwitch, MomentarySwitch {
	
	ML_SmallLEDButton();
};



struct MLSwitch : MLSVGSwitch, ToggleSwitch {

	MLSwitch();
};

struct MLSwitch2 : MLSVGSwitch, ToggleSwitch {
	MLSwitch2();
};

struct BlueMLSwitch : MLSVGSwitch, ToggleSwitch {
	BlueMLSwitch();
};



struct MLScrew : FramebufferWidget {

    SVGWidget *sw;
    TransformWidget *tw;

	MLScrew() {

        tw = new TransformWidget();
	    addChild(tw);
	    sw = new SVGWidget();
	    tw->addChild(sw);
	    sw->setSVG(SVG::load(assetPlugin(plugin, "res/MLScrew.svg")));
		tw->box.size = sw->box.size;	

        float angle = 1.71f * (rand() / (static_cast<double>(RAND_MAX) + 1.0)); 

        Vec transl = tw->box.getCenter();
        tw->translate( transl );
        tw->rotate(angle);
        tw->translate( transl.neg() );

	}

    
};
