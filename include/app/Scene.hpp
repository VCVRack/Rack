#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <app/RackScrollWidget.hpp>
#include <app/RackWidget.hpp>


namespace rack {
namespace app {


struct Scene : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	// Convenience variables for accessing important widgets
	RackScrollWidget* rackScroll;
	RackWidget* rack;
	widget::Widget* menuBar;
	widget::Widget* browser;

	double lastAutosaveTime = 0.0;
	/** The last mouse position in the Scene */
	math::Vec mousePos;

	PRIVATE Scene();
	PRIVATE ~Scene();
	math::Vec getMousePos();
	void step() override;
	void draw(const DrawArgs& args) override;
	void onHover(const HoverEvent& e) override;
	void onDragHover(const DragHoverEvent& e) override;
	void onHoverKey(const HoverKeyEvent& e) override;
	void onPathDrop(const PathDropEvent& e) override;
};


} // namespace app
} // namespace rack
