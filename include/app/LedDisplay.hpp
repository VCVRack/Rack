#pragma once
#include "app/common.hpp"
#include "widgets/Widget.hpp"
#include "widgets/TransparentWidget.hpp"
#include "ui/TextField.hpp"


namespace rack {


struct LedDisplay : Widget {
	void draw(const DrawContext &ctx) override;
};

struct LedDisplaySeparator : TransparentWidget {
	LedDisplaySeparator();
	void draw(const DrawContext &ctx) override;
};

struct LedDisplayChoice : TransparentWidget {
	std::string text;
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	LedDisplayChoice();
	void draw(const DrawContext &ctx) override;
	void onButton(const event::Button &e) override;
};

struct LedDisplayTextField : TextField {
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	LedDisplayTextField();
	void draw(const DrawContext &ctx) override;
	int getTextPosition(math::Vec mousePos) override;
};


} // namespace rack
