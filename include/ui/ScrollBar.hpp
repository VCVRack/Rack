#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {


/** Parent must be a ScrollWidget */
struct ScrollBar : OpaqueWidget {
	enum Orientation {
		VERTICAL,
		HORIZONTAL
	};
	Orientation orientation;
	BNDwidgetState state = BND_DEFAULT;
	float offset = 0.0;
	float size = 0.0;

	ScrollBar();
	void draw(NVGcontext *vg) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragMove(const event::DragMove &e) override;
	void onDragEnd(const event::DragEnd &e) override;
};


} // namespace rack
