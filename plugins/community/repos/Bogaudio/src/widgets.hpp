#include "rack.hpp"

using namespace rack;

#ifdef USE_VST2
#define plugin "Bogaudio"
#else
extern Plugin *plugin;
#endif // USE_VST2

namespace bogaudio {

struct Button18 : SVGSwitch, MomentarySwitch {
	Button18();
};

struct BGKnob : RoundKnob {
	BGKnob(const char* svg, int dim);
};

struct Knob16 : BGKnob {
	Knob16();
};

struct Knob19 : BGKnob {
	Knob19();
};

struct Knob26 : BGKnob {
	Knob26();
};

struct Knob29 : BGKnob {
	Knob29();
};

struct Knob38 : BGKnob {
	Knob38();
};

struct Knob45 : BGKnob {
	Knob45();
};

struct Knob68 : BGKnob {
	Knob68();
};

struct Port24 : SVGPort {
	Port24();
};

struct SliderSwitch : SVGSwitch, ToggleSwitch {
	CircularShadow* shadow = NULL;
	SliderSwitch();
};

struct SliderSwitch2State14 : SliderSwitch {
	SliderSwitch2State14();
};

struct StatefulButton : ParamWidget, FramebufferWidget {
	std::vector<std::shared_ptr<SVG>> _frames;
	SVGWidget* _svgWidget; // deleted elsewhere.
	CircularShadow* shadow = NULL;

	StatefulButton(const char* offSVGPath, const char* onSVGPath);
	void step() override;
	void onDragStart(EventDragStart& e) override;
	void onDragEnd(EventDragEnd& e) override;
};

struct StatefulButton9 : StatefulButton {
	StatefulButton9();
};

struct StatefulButton18 : StatefulButton {
	StatefulButton18();
};

struct ToggleButton : SVGSwitch, ToggleSwitch {
};

struct ToggleButton18 : ToggleButton {
	ToggleButton18();
};

NVGcolor decibelsToColor(float db);

struct VUSlider : Knob {
	const float slideHeight = 13.0f;
	float* _vuLevel = NULL;

	VUSlider(float height = 183.0f) {
		box.size = Vec(18.0f, height);
	}

	void setVULevel(float* vuLevel) {
		_vuLevel = vuLevel;
	}
	void draw(NVGcontext* vg) override;
};

struct VUSlider151 : VUSlider {
	VUSlider151() : VUSlider(151.0f) {}
};

} // namespace bogaudio
