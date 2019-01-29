#pragma once
#include "ui/common.hpp"
#include "ui/Button.hpp"


namespace rack {
namespace ui {


struct ChoiceButton : Button {
	void draw(const widget::DrawContext &ctx) override;
};


} // namespace ui
} // namespace rack
