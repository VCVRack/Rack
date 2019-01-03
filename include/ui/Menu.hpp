#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {


struct Menu : OpaqueWidget {
	Menu *parentMenu = NULL;
	Menu *childMenu = NULL;
	/** The entry which created the child menu */
	MenuEntry *activeEntry = NULL;

	Menu();
	~Menu();
	void setChildMenu(Menu *menu);
	void step() override;
	void draw(NVGcontext *vg) override;
	void onHoverScroll(const event::HoverScroll &e) override;
};


} // namespace rack
