#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>


namespace rack {
namespace ui {


/** Deletes itself from parent when clicked */
struct MenuOverlay : widget::OpaqueWidget {
	NVGcolor bgColor;

	MenuOverlay();
	void draw(const DrawArgs& args) override;
	void step() override;
	void onButton(const ButtonEvent& e) override;
	void onHoverKey(const HoverKeyEvent& e) override;
	void onAction(const ActionEvent& e) override;
};


} // namespace ui
} // namespace rack
