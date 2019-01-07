#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {


/** Deletes itself from parent when clicked */
struct MenuOverlay : OpaqueWidget {
	void step() override;
	void onButton(const event::Button &e) override;
	void onHoverKey(const event::HoverKey &e) override;
};


} // namespace rack
