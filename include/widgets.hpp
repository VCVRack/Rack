#pragma once

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <list>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <jansson.h>

#include "../lib/nanovg/src/nanovg.h"
#include "../lib/oui/blendish.h"

#include "util.hpp"


namespace rack {

struct Module;
struct Wire;

struct RackWidget;
struct ParamWidget;
struct InputPort;
struct OutputPort;

////////////////////
// base class and traits
////////////////////

// A node in the 2D scene graph
struct Widget {
	// Stores position and size
	Rect box = Rect(Vec(0, 0), Vec(INFINITY, INFINITY));
	Widget *parent = NULL;
	std::list<Widget*> children;

	virtual ~Widget();

	Vec getAbsolutePos();
	Rect getChildrenBoundingBox();

	// Gives ownership of widget to this widget instance
	void addChild(Widget *widget);
	// Does not delete widget but transfers ownership to caller
	// Silenty fails if widget is not a child
	void removeChild(Widget *widget);
	void clearChildren();

	// Advances the module by one frame
	virtual void step();
	// Draws to NanoVG context
	virtual void draw(NVGcontext *vg);

	// Override this to return NULL if the widget is to be "invisible" to mouse events.
	// Override this to return `this` if the widget is to override events of its children.
	virtual Widget *pick(Vec pos);

	// Events

	virtual void onMouseDown(int button) {}
	virtual void onMouseUp(int button) {}
	virtual void onMouseMove(Vec mouseRel) {}
	virtual void onMouseEnter() {}
	virtual void onMouseLeave() {}
	virtual void onScroll(Vec scrollRel) {}
	virtual void onDragStart() {}
	virtual void onDragDrop(Widget *origin) {}
	virtual void onDragHover(Widget *origin) {}
	virtual void onDragMove(Vec mouseRel) {}
	virtual void onDragEnd() {}
	virtual void onResize() {}
	virtual void onAction() {}
	virtual void onChange() {}
};

// Widget that does not respond to events
struct TransparentWidget : virtual Widget {
	Widget *pick(Vec pos) { return NULL; }
};

// Widget that does not respond to events, but allows its children to
struct TranslucentWidget : virtual Widget {
	Widget *pick(Vec pos) {
		Widget *picked = Widget::pick(pos);
		if (picked == this) {
			return NULL;
		}
		return picked;
	}
};

struct SpriteWidget : virtual Widget {
	Vec spriteOffset;
	Vec spriteSize;
	std::string spriteFilename;
	int index = 0;
	void draw(NVGcontext *vg);
};

struct QuantityWidget : virtual Widget {
	float value = 0.0;
	float minValue = 0.0;
	float maxValue = 1.0;
	float defaultValue = 0.0;
	std::string label;
	// Include a space character if you want a space after the number, e.g. " Hz"
	std::string unit;

	void setValue(float value);
	void setLimits(float minValue, float maxValue);
	void setDefaultValue(float defaultValue);
};

////////////////////
// gui elements
////////////////////

struct Label : TransparentWidget {
	std::string text;
	void draw(NVGcontext *vg);
};

// Deletes itself from parent when clicked
struct MenuOverlay : Widget {
	void onMouseDown(int button);
};

struct Menu : Widget {
	Menu() {
		box.size = Vec(0, 0);
	}
	// Transfers ownership, like addChild()
	void pushChild(Widget *child);
	void draw(NVGcontext *vg);
};

struct MenuEntry : Widget {
	std::string text;
	MenuEntry() {
		box.size = Vec(0, BND_WIDGET_HEIGHT);
	}
	float computeMinWidth(NVGcontext *vg);
};

struct MenuLabel : MenuEntry {
	void draw(NVGcontext *vg);
};

struct MenuItem : MenuEntry {
	BNDwidgetState state = BND_DEFAULT;

	void draw(NVGcontext *vg);

	void onMouseUp(int button);
	void onMouseEnter();
	void onMouseLeave() ;
};

struct Button : Widget {
	std::string text;
	BNDwidgetState state = BND_DEFAULT;

	Button();
	void draw(NVGcontext *vg);
	void onMouseEnter();
	void onMouseLeave() ;
	void onDragDrop(Widget *origin);
};

struct ChoiceButton : Button {
	void draw(NVGcontext *vg);
};

struct Slider : QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	Slider();
	void draw(NVGcontext *vg);
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
};

struct ScrollBar : Widget {
	enum { VERTICAL, HORIZONTAL } orientation;
	float containerOffset = 0.0;
	float containerSize = 0.0;
	BNDwidgetState state = BND_DEFAULT;

	ScrollBar();
	void draw(NVGcontext *vg);
	void move(float delta);
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
};

// Handles a container with scrollbars
struct ScrollWidget : Widget {
	Widget *container;
	ScrollBar *hScrollBar;
	ScrollBar *vScrollBar;

	ScrollWidget();
	void draw(NVGcontext *vg);
	void onResize();
	void onScroll(Vec scrollRel);
};

struct Tooltip : TransparentWidget {
	void step();
	void draw(NVGcontext *vg);
};

////////////////////
// module
////////////////////

// A 1U module should be 15x380. Thus the width of a module should be a factor of 15.
struct Model;
struct ModuleWidget : Widget {
	Model *model = NULL;
	// Eventually this should be replaced with a `moduleId` which will be used for inter-process communication between the gui world and the audio world.
	Module *module = NULL;
	// int moduleId;

	std::vector<InputPort*> inputs;
	std::vector<OutputPort*> outputs;
	std::vector<ParamWidget*> params;

	ModuleWidget(Module *module);
	~ModuleWidget();
	// Convenience functions for adding special widgets (calls addChild())
	void addInput(InputPort *input);
	void addOutput(OutputPort *output);
	void addParam(ParamWidget *param);

	json_t *toJson();
	void fromJson(json_t *root);
	void disconnectPorts();
	void resetParams();
	void cloneParams(ModuleWidget *source);

	void draw(NVGcontext *vg);

	bool requested = false;
	Vec requestedPos;
	Vec dragPos;
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
	void onMouseDown(int button);
};

struct WireWidget : Widget {
	OutputPort *outputPort = NULL;
	InputPort *inputPort = NULL;
	Wire *wire = NULL;
	NVGcolor color;

	WireWidget();
	~WireWidget();
	void updateWire();
	void draw(NVGcontext *vg);
	void drawOutputPlug(NVGcontext *vg);
	void drawInputPlug(NVGcontext *vg);
};

struct RackWidget : Widget {
	// Only put ModuleWidgets in here
	Widget *moduleContainer;
	// Only put WireWidgets in here
	Widget *wireContainer;
	// An unowned reference to the currently dragged wire
	WireWidget *activeWire = NULL;

	RackWidget();
	~RackWidget();
	void clear();
	void savePatch(std::string filename);
	void loadPatch(std::string filename);
	json_t *toJson();
	void fromJson(json_t *root);

	int frame = 0;
	void repositionModule(ModuleWidget *module);
	void step();
	void draw(NVGcontext *vg);

	void onMouseDown(int button);
};

////////////////////
// params
////////////////////

struct Light : TransparentWidget, SpriteWidget {
	NVGcolor color;
	void draw(NVGcontext *vg);
};

// If you don't add these to your ModuleWidget, it will fall out of the RackWidget
struct Screw : TransparentWidget, SpriteWidget {
	Screw();
};

struct ParamWidget : QuantityWidget {
	Module *module = NULL;
	int paramId;

	json_t *toJson();
	void fromJson(json_t *root);
	void onMouseDown(int button);
	void onChange();
};

struct Knob : ParamWidget, SpriteWidget {
	int minIndex, maxIndex, spriteCount;
	void step();
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
};

struct Switch : ParamWidget, SpriteWidget {
};

struct ToggleSwitch : virtual Switch {
	void onDragStart() {
		index = 1;
	}
	void onDragEnd() {
		index = 0;
	}
	void onDragDrop(Widget *origin) {
		if (origin != this)
			return;

		// Cycle through modes
		// e.g. a range of [0.0, 3.0] would have modes 0, 1, 2, and 3.
		float v = value + 1.0;
		setValue(v > maxValue ? minValue : v);
	}
};

struct MomentarySwitch : virtual Switch {
	void onDragStart() {
		setValue(maxValue);
		index = 1;
	}
	void onDragEnd() {
		setValue(minValue);
		index = 0;
	}
};

////////////////////
// ports
////////////////////

struct Port : Widget {
	Module *module = NULL;
	WireWidget *connectedWire = NULL;

	Port();
	~Port();
	void disconnect();

	int type;
	void draw(NVGcontext *vg);
	void drawGlow(NVGcontext *vg);
	void onMouseDown(int button);
	void onDragEnd();
};

struct InputPort : Port {
	int inputId;

	void draw(NVGcontext *vg);
	void onDragStart();
	void onDragDrop(Widget *origin);
};

struct OutputPort : Port {
	int outputId;

	void draw(NVGcontext *vg);
	void onDragStart();
	void onDragDrop(Widget *origin);
};

////////////////////
// scene
////////////////////

struct Toolbar : Widget {
	Slider *wireOpacitySlider;
	Toolbar();
	void draw(NVGcontext *vg);
};

struct Scene : Widget {
	Toolbar *toolbar;
	ScrollWidget *scrollWidget;
	Scene();
	void onResize();
};


} // namespace rack
