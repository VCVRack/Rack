#pragma once
#include <widget/Widget.hpp>
#include <ui/common.hpp>


namespace rack {
namespace ui {


struct Label : widget::Widget {
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,
	};

	std::string text;
	float fontSize;
	NVGcolor color;
	Alignment alignment = LEFT_ALIGNMENT;

	Label();
	void draw(const DrawArgs& args) override;
};


} // namespace ui
} // namespace rack
