#pragma once
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {


struct MenuSeparator : MenuEntry {
	MenuSeparator();
	void draw(const DrawContext &ctx) override;
};


} // namespace rack
