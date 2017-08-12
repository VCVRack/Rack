#pragma once
#include <vector>
#include <jansson.h>
#include "widgets.hpp"


namespace rack {


struct Module;
struct Wire;

struct RackWidget;
struct ParamWidget;
struct Port;
struct Scene;

////////////////////
// module
////////////////////

// A 1U module should be 15x380. Thus the width of a module should be a factor of 15.
struct Model;
struct ModuleWidget : OpaqueWidget {
	Model *model = NULL;
	/** Owns the module pointer */
	Module *module = NULL;

	std::vector<Port*> inputs;
	std::vector<Port*> outputs;
	std::vector<ParamWidget*> params;

	~ModuleWidget();
	void setModule(Module *module);
	// Convenience functions for adding special widgets (calls addChild())
	void addInput(Port *input);
	void addOutput(Port *output);
	void addParam(ParamWidget *param);

	json_t *toJson();
	void fromJson(json_t *root);
	void disconnectPorts();
	void resetParams();
	void randomizeParams();

	void draw(NVGcontext *vg);

	bool requested = false;
	Vec requestedPos;
	Vec dragPos;
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
	void onMouseDown(int button);
};

struct WireWidget : OpaqueWidget {
	Port *inputPort = NULL;
	Port *outputPort = NULL;
	Port *hoveredInputPort = NULL;
	Port *hoveredOutputPort = NULL;
	Wire *wire = NULL;
	NVGcolor color;

	WireWidget();
	~WireWidget();
	void updateWire();
	void draw(NVGcontext *vg);
	void drawOutputPlug(NVGcontext *vg);
	void drawInputPlug(NVGcontext *vg);
};

struct RackWidget : OpaqueWidget {
	// Only put ModuleWidgets in here
	Widget *moduleContainer;
	// Only put WireWidgets in here
	Widget *wireContainer;
	WireWidget *activeWire = NULL;
	std::shared_ptr<Image> railsImage;

	RackWidget();
	~RackWidget();
	void clear();
	void savePatch(std::string filename);
	void loadPatch(std::string filename);
	json_t *toJson();
	void fromJson(json_t *root);

	void repositionModule(ModuleWidget *module);
	void step();
	void draw(NVGcontext *vg);

	void onMouseDown(int button);
};

struct Panel : TransparentWidget {
	NVGcolor backgroundColor;
	NVGcolor borderColor;
	std::shared_ptr<Image> backgroundImage;
	void draw(NVGcontext *vg);
};

////////////////////
// params
////////////////////

struct CircularShadow : TransparentWidget {
	float blur = 0.0;
	void draw(NVGcontext *vg);
};

struct Light : TransparentWidget {
	NVGcolor color;
	void draw(NVGcontext *vg);
};

struct ParamWidget : OpaqueWidget, QuantityWidget {
	Module *module = NULL;
	int paramId;

	json_t *toJson();
	void fromJson(json_t *root);
	void onMouseDown(int button);
	void onChange();
};

/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
	/** Tell engine to smoothly vary this parameter */
	void onChange();
};

struct SpriteKnob : virtual Knob, SpriteWidget {
	int minIndex, maxIndex, spriteCount;
	void step();
};

/** A knob which rotates an SVG and caches it in a framebuffer */
struct SVGKnob : virtual Knob, FramebufferWidget {
	/** Angles in radians */
	float minAngle, maxAngle;
	/** Not owned */
	TransformWidget *tw;
	SVGWidget *sw;

	SVGKnob();
	void setSVG(std::shared_ptr<SVG> svg);
	void step();
	void onChange();
};

/** Snaps to the nearest integer value on mouse release */
struct SnapKnob : virtual Knob {
	void onDragEnd() {
		setValue(roundf(value));
		Knob::onDragEnd();
	}
};

struct SVGSlider : Knob, FramebufferWidget {
	/** Intermediate positions will be interpolated between these positions */
	Vec minHandlePos, maxHandlePos;
	/** Not owned */
	SVGWidget *background;
	SVGWidget *handle;

	SVGSlider();
	void step();
	void onChange();
};

struct Switch : ParamWidget {
};

struct SVGSwitch : virtual Switch, FramebufferWidget {
	std::vector<std::shared_ptr<SVG>> frames;
	/** Not owned */
	SVGWidget *sw;

	SVGSwitch();
	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<SVG> svg);
	void step();
	void onChange();
};

/** A switch that cycles through each mechanical position */
struct ToggleSwitch : virtual Switch {
	void onDragStart() {
		// Cycle through values
		// e.g. a range of [0.0, 3.0] would have modes 0, 1, 2, and 3.
		if (value >= maxValue)
			setValue(minValue);
		else
			setValue(value + 1.0);
	}
};

/** A switch that is turned on when held */
struct MomentarySwitch : virtual Switch {
	void onDragStart() {
		setValue(maxValue);
	}
	void onDragEnd() {
		setValue(minValue);
	}
};

////////////////////
// ports
////////////////////

struct Port : OpaqueWidget {
	enum PortType {
		DEFAULT,
		INPUT,
		OUTPUT
	};

	Module *module = NULL;
	WireWidget *connectedWire = NULL;
	PortType type = DEFAULT;
	int portId;

	Port();
	~Port();
	void disconnect();

	void draw(NVGcontext *vg);
	void onMouseDown(int button);
	void onDragEnd();
	void onDragStart();
	void onDragDrop(Widget *origin);
	void onDragEnter(Widget *origin);
	void onDragLeave(Widget *origin);
};

struct SVGPort : Port, FramebufferWidget {
	SVGWidget *background;

	SVGPort();
	void draw(NVGcontext *vg);
};

/** If you don't add these to your ModuleWidget, they will fall out of the rack... */
struct SVGScrew : FramebufferWidget {
	SVGWidget *sw;

	SVGScrew();
};

////////////////////
// scene
////////////////////

struct Toolbar : OpaqueWidget {
	Slider *wireOpacitySlider;
	Slider *wireTensionSlider;
	RadioButton *cpuUsageButton;

	Toolbar();
	void draw(NVGcontext *vg);
};

struct PluginManagerWidget : Widget {
	Widget *loginWidget;
	Widget *manageWidget;
	Widget *downloadWidget;
	PluginManagerWidget();
	void step();
};

struct RackScene : Scene {
	Toolbar *toolbar;
	ScrollWidget *scrollWidget;

	RackScene();
	void step();
	void draw(NVGcontext *vg);
};

////////////////////
// globals
////////////////////

extern std::string gApplicationName;
extern std::string gApplicationVersion;

extern RackWidget *gRackWidget;

void sceneInit();
void sceneDestroy();

} // namespace rack
