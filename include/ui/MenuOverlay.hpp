#pragma once
#include "widget/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {
namespace ui {


/** Deletes itself from parent when clicked */
struct MenuOverlay : widget::OpaqueWidget {
	void step() override;
	void onButton(const widget::ButtonEvent &e) override;
	void onHoverKey(const widget::HoverKeyEvent &e) override;
};


} // namespace ui
} // namespace rack
