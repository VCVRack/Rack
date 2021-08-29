#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>
#include <ui/MenuEntry.hpp>


namespace rack {
namespace ui {


struct Menu : widget::OpaqueWidget {
	Menu* parentMenu = NULL;
	Menu* childMenu = NULL;
	/** The entry which created the child menu */
	MenuEntry* activeEntry = NULL;
	BNDcornerFlags cornerFlags = BND_CORNER_NONE;

	Menu();
	~Menu();
	void setChildMenu(Menu* menu);
	void step() override;
	void draw(const DrawArgs& args) override;
	void onHoverScroll(const HoverScrollEvent& e) override;
};


} // namespace ui
} // namespace rack
