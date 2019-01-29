#pragma once
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {
namespace ui {


struct MenuSeparator : MenuEntry {
	MenuSeparator();
	void draw(const widget::DrawContext &ctx) override;
};


} // namespace ui
} // namespace rack
