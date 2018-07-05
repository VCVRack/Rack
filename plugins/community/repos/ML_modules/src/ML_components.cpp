#include "ML_components.hpp"


#include <functional>

using namespace rack;



MLSVGSwitch::MLSVGSwitch() {

    shadow = new CircularShadow();
    addChild(shadow);
    shadow->box.size = Vec();

	sw = new SVGWidget();
	addChild(sw);
}

void MLSVGSwitch::addFrame(std::shared_ptr<SVG> svg) {
	frames.push_back(svg);
	// If this is our first frame, automatically set SVG and size
	if (!sw->svg) {
		sw->setSVG(svg);
		box.size = sw->box.size;
	}
}

void MLSVGSwitch::onChange(EventChange &e) {
	assert(frames.size() > 0);
	float valueScaled = rescale(value, minValue, maxValue, 0, frames.size() - 1);
	int index = clamp((int) roundf(valueScaled), 0, (int) frames.size() - 1);
	sw->setSVG(frames[index]);
	dirty = true;
	ParamWidget::onChange(e);
}




WhiteLight::WhiteLight() {
	addBaseColor(COLOR_WHITE);
}

BlueMLKnob::BlueMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/Knob.svg")));
};

SmallBlueMLKnob::SmallBlueMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/Knob_28.svg")));
};

BlueSnapMLKnob::BlueSnapMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/Knob.svg")));
	snap = true;
};

SmallBlueSnapMLKnob::SmallBlueSnapMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/Knob_28.svg")));
	snap = true;
};


RedMLKnob::RedMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/RedKnob.svg")));
};

SmallRedMLKnob::SmallRedMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/RedKnob_28.svg")));
};

RedSnapMLKnob::RedSnapMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/RedKnob.svg")));
	snap = true;
};

SmallRedSnapMLKnob::SmallRedSnapMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/RedKnob_28.svg")));
	snap = true;
};



GreyMLKnob::GreyMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/GreyKnob.svg")));
};

SmallGreyMLKnob::SmallGreyMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/GreyKnob_28.svg")));
};


GreySnapMLKnob::GreySnapMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/GreyKnob.svg")));
	snap = true;
};

SmallGreySnapMLKnob::SmallGreySnapMLKnob() {
    setSVG(SVG::load(assetPlugin(plugin,"res/GreyKnob_28.svg")));
	snap = true;
};



MLPort::MLPort() {
	setSVG(SVG::load(assetPlugin(plugin, "res/Jack.svg")));
};


MLButton::MLButton() {
	addFrame(SVG::load(assetPlugin(plugin, "res/MLButton_0.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/MLButton_1.svg")));
	sw->wrap();
	box.size = sw->box.size;
};


MLSmallButton::MLSmallButton() {
	addFrame(SVG::load(assetPlugin(plugin, "res/SmallButton_0.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/SmallButton_1.svg")));
	sw->wrap();
	box.size = sw->box.size;
};


ML_LEDButton::ML_LEDButton() {

	addFrame(SVG::load(assetPlugin(plugin, "res/LEDButton.svg")));
	sw->wrap();
	box.size = sw->box.size;

	shadow->box.size = box.size;
	shadow->blurRadius = 0.0f;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);
	
};

ML_MediumLEDButton::ML_MediumLEDButton() {

	addFrame(SVG::load(assetPlugin(plugin, "res/LEDButton_medium.svg")));
	sw->wrap();
	box.size = sw->box.size;

	shadow->box.size = box.size;
	shadow->blurRadius = 0.0f;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);
	
};


ML_SmallLEDButton::ML_SmallLEDButton() {

	addFrame(SVG::load(assetPlugin(plugin, "res/LEDButton_small.svg")));
	sw->wrap();
	box.size = sw->box.size;

	shadow->box.size = box.size;
	shadow->blurRadius = 0.0f;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);
	
};




ML_ResetButton::ML_ResetButton() {

	addFrame(SVG::load(assetPlugin(plugin, "res/ResetButton_0.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/ResetButton_1.svg")));

	sw->wrap();
	box.size = sw->box.size;
}

MLSwitch::MLSwitch() {

	addFrame(SVG::load(assetPlugin(plugin, "res/Switch_0.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/Switch_1.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/Switch_2.svg")));

	shadow->box.size = box.size;
	shadow->blurRadius = 0.0f;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);

};

MLSwitch2::MLSwitch2() {
	addFrame(SVG::load(assetPlugin(plugin, "res/Switch_0.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/Switch_2.svg")));

	shadow->box.size = box.size;
	shadow->blurRadius = 0.0f;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);
    
};

BlueMLSwitch::BlueMLSwitch() {
	addFrame(SVG::load(assetPlugin(plugin, "res/BlueSwitch_0.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/BlueSwitch_1.svg")));
	addFrame(SVG::load(assetPlugin(plugin, "res/BlueSwitch_2.svg")));

   	shadow->box.size = box.size;
	shadow->blurRadius = 0.0f;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);

};
