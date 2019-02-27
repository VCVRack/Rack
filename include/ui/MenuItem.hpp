#pragma once
#include "ui/common.hpp"
#include "ui/Menu.hpp"
#include "ui/MenuEntry.hpp"
#include "app.hpp"


namespace rack {
namespace ui {


struct MenuItem : MenuEntry {
	std::string text;
	std::string rightText;
	bool disabled = false;

	void draw(const DrawArgs &args) override;
	void step() override;
	void onEnter(const widget::EnterEvent &e) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragDrop(const widget::DragDropEvent &e) override;
	void doAction();
	virtual Menu *createChildMenu() {return NULL;}
};


} // namespace ui
} // namespace rack
