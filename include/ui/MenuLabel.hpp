#pragma once
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {


struct MenuLabel : MenuEntry {
	std::string text;

	void draw(NVGcontext *vg) override;
	void step() override;
};


} // namespace rack
