#pragma once
#include "ui/common.hpp"
#include "ui/Button.hpp"


namespace rack {


struct ChoiceButton : Button {
	void draw(const DrawContext &ctx) override;
};


} // namespace rack
