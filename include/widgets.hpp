#pragma once
#include <list>
#include <memory>

#include "nanovg.h"
#include "nanosvg.h"

#include "util/common.hpp"
#include "events.hpp"
#include "util/color.hpp"


namespace rack {

////////////////////
// resources
////////////////////

// Constructing these directly will load from the disk each time. Use the load() functions to load from disk and cache them as long as the shared_ptr is held.
// Implemented in window.cpp

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

/** A node in the 2D scene graph
Never inherit from Widget directly. Instead, inherit from VirtualWidget declared below.
*/
struct Widget {
	/** Stores position and size */
	Rect box = Rect(Vec(), Vec(INFINITY, INFINITY));
	Widget *parent = NULL;
	std::list<Widget*> children;
	bool visible = true;

	virtual ~Widget();

	virtual Rect getChildrenBoundingBox();
	/**  Returns `v` transformed into the coordinate system of `relative` */
	virtual Vec getRelativeOffset(Vec v, Widget *relative);
	/** Returns `v` transformed into world coordinates */
	Vec getAbsoluteOffset(Vec v) {
		return getRelativeOffset(v, NULL);
	}
	/** Returns a subset of the given Rect bounded by the box of this widget and all ancestors */
	virtual Rect getViewport(Rect r);

	template <class T>
	T *getAncestorOfType() {
		if (!parent) return NULL;
		T *p = dynamic_cast<T*>(parent);
		if (p) return p;
		return parent->getAncestorOfType<T>();
	}

	template <class T>
	T *getFirstDescendantOfType() {
		for (Widget *child : children) {
			T *c = dynamic_cast<T*>(child);
			if (c) return c;
			c = child->getFirstDescendantOfType<T>();
			if (c) return c;
		}
		return NULL;
	}

	/** Adds widget to list of children.
	Gives ownership of widget to this widget instance.
	*/
	void addChild(Widget *widget);
	/** Removes widget from list of children if it exists.
	Does not delete widget but transfers ownership to caller
	*/
	void removeChild(Widget *widget);
	/** Removes and deletes all children */
	void clearChildren();
	/** Recursively finalizes event start/end pairs as needed */
	void finalizeEvents();

	/** Advances the module by one frame */
	virtual void step();
	/** Draws to NanoVG context */
	virtual void draw(NVGcontext *vg);

	// Events

	/** Called when a mouse button is pressed over this widget */
	virtual void onMouseDown(EventMouseDown &e);
	/** Called when a mouse button is released over this widget */
	virtual void onMouseUp(EventMouseUp &e);
	/** Called when the mouse moves over this widget.
	Called on every frame, even if `mouseRel = Vec(0, 0)`.
	*/
	virtual void onMouseMove(EventMouseMove &e);
	/** Called when a key is pressed while hovering over this widget */
	virtual void onHoverKey(EventHoverKey &e);
	/** Called when this widget begins responding to `onMouseMove` events */
	virtual void onMouseEnter(EventMouseEnter &e) {}
	/** Called when this widget no longer responds to `onMouseMove` events */
	virtual void onMouseLeave(EventMouseLeave &e) {}
	/** Called when this widget gains focus by responding to the `onMouseDown` event */
	virtual void onFocus(EventFocus &e) {}
	virtual void onDefocus(EventDefocus &e) {}
	/** Called when a printable character is received while this widget is focused */
	virtual void onText(EventText &e) {}
	/** Called when a key is pressed while this widget is focused */
	virtual void onKey(EventKey &e) {}
	/** Called when the scroll wheel is moved while the mouse is hovering over this widget */
	virtual void onScroll(EventScroll &e);

	/** Called when a widget responds to `onMouseDown` for a left button press */
	virtual void onDragStart(EventDragStart &e) {}
	/** Called when the left button is released and this widget is being dragged */
	virtual void onDragEnd(EventDragEnd &e) {}
	/** Called when a widget responds to `onMouseMove` and is being dragged */
	virtual void onDragMove(EventDragMove &e) {}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	virtual void onDragEnter(EventDragEnter &e) {}
	virtual void onDragLeave(EventDragEnter &e) {}
	/** Called when a drag action ends while hovering this widget */
	virtual void onDragDrop(EventDragDrop &e) {}
	/** Called when an OS selection of files is dragged-and-dropped on this widget */
	virtual void onPathDrop(EventPathDrop &e);

	/** Called when an event triggers an action */
	virtual void onAction(EventAction &e) {}
	/** For widgets with some concept of values, called when the value is changed */
	virtual void onChange(EventChange &e) {}
	/** Called when the zoom level is changed of this widget */
	virtual void onZoom(EventZoom &e);

	/** Helper function for creating and initializing a Widget with certain arguments (in this case just the position).
	In this project, you will find this idiom everywhere, as an easier alternative to constructor arguments, for building a Widget (or a subclass) with a one-liner.
	Example:
		addChild(Widget::create<SVGWidget>(Vec(10, 10)))
	*/
	template <typename T = Widget>
	static T *create(Vec pos = Vec()) {
		T *o = new T();
		o->box.pos = pos;
		return o;
	}
};

/** Instead of inheriting from Widget directly, inherit from VirtualWidget to guarantee that only one copy of Widget's member variables are used by each instance of the Widget hierarchy.
*/
struct VirtualWidget : virtual Widget {};

struct TransformWidget : VirtualWidget {
	/** The transformation matrix */
	float transform[6];
	TransformWidget();
	Rect getChildrenBoundingBox() override;
	void identity();
	void translate(Vec delta);
	void rotate(float angle);
	void scale(Vec s);
	void draw(NVGcontext *vg) override;
};

struct ZoomWidget : VirtualWidget {
	float zoom = 1.0;
	Vec getRelativeOffset(Vec v, Widget *relative) override;
	Rect getViewport(Rect r) override;
	void setZoom(float zoom);
	void draw(NVGcontext *vg) override;
	void onMouseDown(EventMouseDown &e) override;
	void onMouseUp(EventMouseUp &e) override;
	void onMouseMove(EventMouseMove &e) override;
	void onHoverKey(EventHoverKey &e) override;
	void onScroll(EventScroll &e) override;
	void onPathDrop(EventPathDrop &e) override;
};

////////////////////
// Trait widgets
////////////////////

/** Widget that does not respond to events */
struct TransparentWidget : VirtualWidget {
	void onMouseDown(EventMouseDown &e) override {}
	void onMouseUp(EventMouseUp &e) override {}
	void onMouseMove(EventMouseMove &e) override {}
	void onScroll(EventScroll &e) override {}
};

/** Widget that automatically responds to all mouse events but gives a chance for children to respond instead */
struct OpaqueWidget : VirtualWidget {
	void onMouseDown(EventMouseDown &e) override {
		Widget::onMouseDown(e);
		if (!e.target)
			e.target = this;
		e.consumed = true;
	}
	void onMouseUp(EventMouseUp &e) override {
		Widget::onMouseUp(e);
		if (!e.target)
			e.target = this;
		e.consumed = true;
	}
	void onMouseMove(EventMouseMove &e) override {
		Widget::onMouseMove(e);
		if (!e.target)
			e.target = this;
		e.consumed = true;
	}
	void onScroll(EventScroll &e) override {
		Widget::onScroll(e);
		e.consumed = true;
	}
};

struct SpriteWidget : VirtualWidget {
	Vec spriteOffset;
	Vec spriteSize;
	std::shared_ptr<Image> spriteImage;
	int index = 0;
	void draw(NVGcontext *vg) override;
};

struct SVGWidget : VirtualWidget {
	std::shared_ptr<SVG> svg;
	/** Sets the box size to the svg image size */
	void wrap();
	/** Sets and wraps the SVG */
	void setSVG(std::shared_ptr<SVG> svg);
	void draw(NVGcontext *vg) override;
};

/** Caches a widget's draw() result to a framebuffer so it is called less frequently
When `dirty` is true, its children will be re-rendered on the next call to step() override.
Events are not passed to the underlying scene.
*/
struct FramebufferWidget : VirtualWidget {
	/** Set this to true to re-render the children to the framebuffer the next time it is drawn */
	bool dirty = true;
	/** A margin in pixels around the children in the framebuffer
	This prevents cutting the rendered SVG off on the box edges.
	*/
	float oversample;
	/** The root object in the framebuffer scene
	The FramebufferWidget owns the pointer
	*/
	struct Internal;
	Internal *internal;

	FramebufferWidget();
	~FramebufferWidget();
	void draw(NVGcontext *vg) override;
	int getImageHandle();
	void onZoom(EventZoom &e) override;
};

/** A Widget representing a float value */
struct QuantityWidget : VirtualWidget {
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
// globals
////////////////////

extern Widget *gHoveredWidget;
extern Widget *gDraggedWidget;
extern Widget *gDragHoveredWidget;
extern Widget *gFocusedWidget;
extern Widget *gTempWidget;


} // namespace rack
