#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>


namespace rack {
namespace ui {


/** Deletes itself from parent when clicked */
struct MenuOverlay : widget::OpaqueWidget {
	void draw(const DrawArgs& args) override;
	void step() override;
	void onButton(const event::Button& e) override;
	void onHoverKey(const event::HoverKey& e) override;
	void onAction(const event::Action& e) override;
};


} // namespace ui
} // namespace rack
