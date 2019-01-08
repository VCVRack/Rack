#pragma once
#include "ui/common.hpp"
#include "ui/Menu.hpp"
#include "ui/MenuEntry.hpp"
#include "ui/MenuOverlay.hpp"
#include "app.hpp"


namespace rack {


#define BND_LABEL_FONT_SIZE 13


struct MenuItem : MenuEntry {
	std::string text;
	std::string rightText;
	bool disabled = false;

	void draw(NVGcontext *vg) override;
	void step() override;
	void onEnter(const event::Enter &e) override;
	void onDragDrop(const event::DragDrop &e) override;
	void doAction();
	virtual Menu *createChildMenu() {return NULL;}
};


} // namespace rack
