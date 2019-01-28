#pragma once
#include "widgets/Widget.hpp"
#include "ui/common.hpp"


namespace rack {


struct Tooltip : Widget {
	std::string text;

	void step() override;
	void draw(const DrawContext &ctx) override;
};


} // namespace rack
