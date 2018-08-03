#pragma once
#include <list>

#include "common.hpp"
#include "events.hpp"
#include "color.hpp"

#include "widgets/Widget.hpp"


namespace rack {


////////////////////
// Base widget
////////////////////


/** Instead of inheriting from Widget directly, inherit from VirtualWidget to guarantee that only one copy of Widget's member variables are used by each instance of the Widget hierarchy.
*/
struct VirtualWidget : virtual Widget {};

struct TransformWidget : VirtualWidget {
	/** The transformation matrix */
	float transform[6];
	TransformWidget();
	math::Rect getChildrenBoundingBox() override;
	void identity();
	void translate(math::Vec delta);
	void rotate(float angle);
	void scale(math::Vec s);
	void draw(NVGcontext *vg) override;
};

struct ZoomWidget : VirtualWidget {
	float zoom = 1.0;
	math::Vec getRelativeOffset(math::Vec v, Widget *relative) override;
	math::Rect getViewport(math::Rect r) override;
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
