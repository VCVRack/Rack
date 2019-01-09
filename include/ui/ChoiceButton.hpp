#pragma once
#include "ui/common.hpp"
#include "ui/Button.hpp"


namespace rack {


struct ChoiceButton : Button {
	void draw(NVGcontext *vg) override;
};


} // namespace rack
