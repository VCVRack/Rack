#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>


namespace rack {
namespace ui {


/** Parent must be a ScrollWidget */
struct Scrollbar : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	bool vertical = false;

	Scrollbar();
	~Scrollbar();
	void draw(const DrawArgs& args) override;
	void onButton(const event::Button& e) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragEnd(const event::DragEnd& e) override;
	void onDragMove(const event::DragMove& e) override;
};


DEPRECATED typedef Scrollbar ScrollBar;


} // namespace ui
} // namespace rack
