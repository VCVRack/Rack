#pragma once
#include <list>
#include <memory>

#include "../ext/nanovg/src/nanovg.h"
#include "../ext/oui-blendish/blendish.h"
#include "../ext/nanosvg/src/nanosvg.h"

#include "math.hpp"
#include "util.hpp"


namespace rack {


////////////////////
// resources
////////////////////

// Constructing these directly will load from the disk each time. Use the load() functions to load from disk and cache them as long as the shared_ptr is held.
// Implemented in gui.cpp

struct Font {
	int handle;
	Font(const std::string &filename);
	~Font();
	static std::shared_ptr<Font> load(const std::string &filename);
};

struct Image {
	int handle;
	Image(const std::string &filename);
	~Image();
	static std::shared_ptr<Image> load(const std::string &filename);
};

struct SVG {
	NSVGimage *handle;
	SVG(const std::string &filename);
	~SVG();
	static std::shared_ptr<SVG> load(const std::string &filename);
};


////////////////////
// Base widget
////////////////////

/** A node in the 2D scene graph */
struct Widget {
	/** Stores position and size */
	Rect box = Rect(Vec(), Vec(INFINITY, INFINITY));
	Widget *parent = NULL;
	std::list<Widget*> children;
	bool visible = true;

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
	/** Removes widget from list of children if it exists.
	Does not delete widget but transfers ownership to caller
	Silently fails if widget is not a child
	*/
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
	virtual void onSelect() {}
	virtual void onDeselect() {}
	virtual void onText(int codepoint) {}
	virtual void onKey(int key) {}
	virtual Widget *onScroll(Vec pos, Vec scrollRel);

	/** Called when a widget responds to `onMouseDown` for a left button press */
	virtual void onDragStart() {}
	/** Called when the left button is released and this widget is being dragged */
	virtual void onDragEnd() {}
	/** Called when a widget responds to `onMouseMove` and is being dragged */
	virtual void onDragMove(Vec mouseRel) {}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	virtual void onDragEnter(Widget *origin) {}
	virtual void onDragLeave(Widget *origin) {}
	virtual void onDragDrop(Widget *origin) {}

	virtual void onAction() {}
	virtual void onChange() {}
};

struct TransformWidget : Widget {
	/** The transformation matrix */
	float transform[6];
	TransformWidget();
	void identity();
	void translate(Vec delta);
	void rotate(float angle);
	void scale(Vec s);
	void draw(NVGcontext *vg);
};


////////////////////
// Trait widgets
////////////////////

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
	std::shared_ptr<Image> spriteImage;
	int index = 0;
	void draw(NVGcontext *vg);
};

struct SVGWidget : virtual Widget {
	std::shared_ptr<SVG> svg;
	/** Sets the box size to the svg image size */
	void wrap();
	void draw(NVGcontext *vg);
};

/** Caches a widget's draw() result to a framebuffer so it is called less frequently
When `dirty` is true, its children will be re-rendered on the next call to step().
Events are not passed to the underlying scene.
*/
struct FramebufferWidget : virtual Widget {
	/** Set this to true to re-render the children to the framebuffer in the next step() */
	bool dirty = true;
	/** A margin in pixels around the children in the framebuffer
	This prevents cutting the rendered SVG off on the box edges.
	*/
	Vec padding;
	/** The root object in the framebuffer scene
	The FramebufferWidget owns the pointer
	*/
	struct Internal;
	Internal *internal;

	FramebufferWidget();
	~FramebufferWidget();
	void step();
	void draw(NVGcontext *vg);
	int getImageHandle();
};

struct QuantityWidget : virtual Widget {
	float value = 0.0;
	float minValue = 0.0;
	float maxValue = 1.0;
	float defaultValue = 0.0;
	std::string label;
	/** Include a space character if you want a space after the number, e.g. " Hz" */
	std::string unit;
	/** The decimal place to round for displaying values.
	A precision of 2 will display as "1.00" for example.
	*/
	int precision = 2;

	QuantityWidget();
	void setValue(float value);
	void setLimits(float minValue, float maxValue);
	void setDefaultValue(float defaultValue);
	/** Generates the display value */
	std::string getText();
};

////////////////////
// GUI widgets
////////////////////

struct Label : Widget {
	std::string text;
	Label() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
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

	Button() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg);
	void onMouseEnter();
	void onMouseLeave();
	void onDragStart();
	void onDragEnd();
	void onDragDrop(Widget *origin);
};

struct ChoiceButton : Button {
	void draw(NVGcontext *vg);
};

struct RadioButton : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	RadioButton() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg);
	void onMouseEnter();
	void onMouseLeave();
	void onDragDrop(Widget *origin);
};

struct Slider : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	Slider() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
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

	ScrollBar() {
		box.size = Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
	}
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

struct TextField : OpaqueWidget {
	std::string text;
	std::string placeholder;
	int begin = 0;
	int end = 0;

	TextField() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg);
	Widget *onMouseDown(Vec pos, int button);
	void onText(int codepoint);
	void onKey(int scancode);
	void onSelect();
	void insertText(std::string newText);
};

struct PasswordField : TextField {
	void draw(NVGcontext *vg);
};

struct ProgressBar : TransparentWidget, QuantityWidget {
	ProgressBar() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg);
};

struct Tooltip : Widget {
	void step();
	void draw(NVGcontext *vg);
};

struct Scene : OpaqueWidget {
	Widget *overlay = NULL;
	void setOverlay(Widget *w);
	Menu *createMenu();
	void step();
};


////////////////////
// globals
////////////////////

extern Vec gMousePos;
extern Widget *gHoveredWidget;
extern Widget *gDraggedWidget;
extern Widget *gDragHoveredWidget;
extern Widget *gSelectedWidget;
extern int gGuiFrame;

extern Scene *gScene;


} // namespace rack
