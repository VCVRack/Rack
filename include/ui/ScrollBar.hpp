#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>


namespace rack {
namespace ui {


/** Parent must be a ScrollWidget */
struct ScrollBar : widget::OpaqueWidget {
	enum Orientation {
		VERTICAL,
		HORIZONTAL
	};
	Orientation orientation;
	float offset = 0.0;
	float size = 0.0;

	ScrollBar();
	void draw(const DrawArgs& args) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragMove(const event::DragMove& e) override;
	void onDragEnd(const event::DragEnd& e) override;
};


} // namespace ui
} // namespace rack
