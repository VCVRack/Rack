#pragma once
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {


struct MenuLabel : MenuEntry {
	std::string text;

	void draw(const DrawContext &ctx) override;
	void step() override;
};


} // namespace rack
