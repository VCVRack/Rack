#pragma once
#include "widget/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {
namespace ui {


/** Parent must be a ScrollWidget */
struct ScrollBar : widget::OpaqueWidget {
	enum Orientation {
		VERTICAL,
		HORIZONTAL
	};
	Orientation orientation;
	BNDwidgetState state = BND_DEFAULT;
	float offset = 0.0;
	float size = 0.0;

	ScrollBar();
	void draw(const DrawArgs &args) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragMove(const widget::DragMoveEvent &e) override;
	void onDragEnd(const widget::DragEndEvent &e) override;
};


} // namespace ui
} // namespace rack
