#pragma once
#include "widgets/Widget.hpp"
#include "ui/common.hpp"


namespace rack {


struct Label : Widget {
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
	void draw(const DrawContext &ctx) override;
};


} // namespace rack
