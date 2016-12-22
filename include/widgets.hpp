#pragma once

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <list>
#include <map>

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

/** A node in the 2D scene graph */
struct Widget {
	/** Stores position and size */
	Rect box = Rect(Vec(), Vec(INFINITY, INFINITY));
	Widget *parent = NULL;
	std::list<Widget*> children;

	virtual ~Widget();

	Vec getAbsolutePos();
	Rect getChildrenBoundingBox();
	template <class T>
	T *getAncestorOfType() {
		if (!parent) return NULL;
		T *p = dynamic_cast<T*>(parent);
		if (p) return p;
		return parent->getAncestorOfType<T>();
	}

	/** Adds widget to list of children.
	Gives ownership of widget to this widget instance.
	*/
	void addChild(Widget *widget);
	// Does not delete widget but transfers ownership to caller
	// Silenty fails if widget is not a child
	void removeChild(Widget *widget);
	void clearChildren();

	/** Advances the module by one frame */
	virtual void step();
	/** Draws to NanoVG context */
	virtual void draw(NVGcontext *vg);

	// Events

	/** Called when a mouse button is pressed over this widget
	0 for left, 1 for right, 2 for middle.
	Return `this` to accept the event.
	Return NULL to reject the event and pass it to the widget behind this one.
	*/
	virtual Widget *onMouseDown(Vec pos, int button);
	virtual Widget *onMouseUp(Vec pos, int button);
	virtual Widget *onMouseMove(Vec pos, Vec mouseRel);
	/** Called when this widget begins responding to `onMouseMove` events */
	virtual void onMouseEnter() {}
	/** Called when another widget begins responding to `onMouseMove` events */
	virtual void onMouseLeave() {}
	virtual Widget *onScroll(Vec pos, Vec scrollRel);

	/** Called when a widget responds to `onMouseDown` for a left button press */
	virtual void onDragStart() {}
	/** Called when a widget responds to `onMouseMove` and is being dragged */
	virtual void onDragMove(Vec mouseRel) {}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	virtual void onDragDrop(Widget *origin) {}
	/** Called when the left button is released and this widget is being dragged */
	virtual void onDragEnd() {}

	virtual void onAction() {}
	virtual void onChange() {}
};

/** Widget that does not respond to events */
struct TransparentWidget : virtual Widget {
	Widget *onMouseDown(Vec pos, int button) {return NULL;}
	Widget *onMouseUp(Vec pos, int button) {return NULL;}
	Widget *onMouseMove(Vec pos, Vec mouseRel) {return NULL;}
	Widget *onScroll(Vec pos, Vec scrollRel) {return NULL;}
};

/** Widget that itself responds to mouse events */
struct OpaqueWidget : virtual Widget {
	Widget *onMouseDown(Vec pos, int button) {
		Widget *w = Widget::onMouseDown(pos, button);
		if (w) return w;
		onMouseDown(button);
		return this;
	}
	Widget *onMouseUp(Vec pos, int button) {
		Widget *w = Widget::onMouseUp(pos, button);
		if (w) return w;
		onMouseUp(button);
		return this;
	}
	Widget *onMouseMove(Vec pos, Vec mouseRel) {
		Widget *w = Widget::onMouseMove(pos, mouseRel);
		if (w) return w;
		onMouseMove(mouseRel);
		return this;
	}

	/** "High level" events called by the above lower level events.
	Use these if you don't care about the clicked position.
	*/
	virtual void onMouseDown(int button) {}
	virtual void onMouseUp(int button) {}
	virtual void onMouseMove(Vec mouseRel) {}
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
	/** Include a space character if you want a space after the number, e.g. " Hz" */
	std::string unit;
	/** The digit place to round for displaying values.
	A precision of -2 will display as "1.00" for example.
	*/
	int precision = -2;

	QuantityWidget();
	void setValue(float value);
	void setLimits(float minValue, float maxValue);
	void setDefaultValue(float defaultValue);
	/** Generates the display value */
	std::string getText();
};

////////////////////
// gui elements
////////////////////

struct Label : Widget {
	std::string text;
	void draw(NVGcontext *vg);
};

// Deletes itself from parent when clicked
struct MenuOverlay : OpaqueWidget {
	void step();
	Widget *onScroll(Vec pos, Vec scrollRel) {
		return this;
	}
	void onDragDrop(Widget *origin);
};

struct Menu : OpaqueWidget {
	Menu() {
		box.size = Vec(0, 0);
	}
	// Resizes menu and calls addChild()
	void pushChild(Widget *child);
	void draw(NVGcontext *vg);
};

struct MenuEntry : OpaqueWidget {
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

	void onMouseEnter();
	void onMouseLeave() ;
	void onDragDrop(Widget *origin);
};

struct Button : OpaqueWidget {
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

struct Slider : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	Slider();
	void draw(NVGcontext *vg);
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
};

struct ScrollBar : OpaqueWidget {
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
struct ScrollWidget : OpaqueWidget {
	Widget *container;
	ScrollBar *hScrollBar;
	ScrollBar *vScrollBar;

	ScrollWidget();
	void step();
	void draw(NVGcontext *vg);
	Widget *onScroll(Vec pos, Vec scrollRel);
};

struct Tooltip : Widget {
	void step();
	void draw(NVGcontext *vg);
};

////////////////////
// module
////////////////////

// A 1U module should be 15x380. Thus the width of a module should be a factor of 15.
struct Model;
struct ModuleWidget : OpaqueWidget {
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

struct WireWidget : OpaqueWidget {
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

struct RackWidget : OpaqueWidget {
	// Only put ModuleWidgets in here
	Widget *moduleContainer;
	// Only put WireWidgets in here
	Widget *wireContainer;
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

struct ModulePanel : TransparentWidget {
	std::string imageFilename;
	NVGcolor backgroundColor;
	void draw(NVGcontext *vg);
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

struct ParamWidget : OpaqueWidget, QuantityWidget {
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

struct Port : OpaqueWidget, SpriteWidget {
	Module *module = NULL;
	WireWidget *connectedWire = NULL;

	Port();
	~Port();
	void disconnect();

	int type;
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

struct Toolbar : OpaqueWidget {
	Slider *wireOpacitySlider;
	Slider *wireTensionSlider;
	Toolbar();
	void draw(NVGcontext *vg);
};

struct Scene : OpaqueWidget {
	Toolbar *toolbar;
	ScrollWidget *scrollWidget;
	Widget *overlay = NULL;
	Scene();
	void setOverlay(Widget *w);
	void step();
};


} // namespace rack
