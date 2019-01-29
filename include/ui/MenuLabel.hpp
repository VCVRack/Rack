#pragma once
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {
namespace ui {


struct MenuLabel : MenuEntry {
	std::string text;

	void draw(const widget::DrawContext &ctx) override;
	void step() override;
};


} // namespace ui
} // namespace rack
