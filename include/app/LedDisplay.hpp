#pragma once
#include "app/common.hpp"
#include "widget/Widget.hpp"
#include "widget/TransparentWidget.hpp"
#include "ui/TextField.hpp"


namespace rack {
namespace app {


struct LedDisplay : widget::Widget {
	void draw(const widget::DrawContext &ctx) override;
};

struct LedDisplaySeparator : widget::TransparentWidget {
	LedDisplaySeparator();
	void draw(const widget::DrawContext &ctx) override;
};

struct LedDisplayChoice : widget::TransparentWidget {
	std::string text;
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	NVGcolor bgColor;
	LedDisplayChoice();
	void draw(const widget::DrawContext &ctx) override;
	void onButton(const event::Button &e) override;
};

struct LedDisplayTextField : ui::TextField {
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	LedDisplayTextField();
	void draw(const widget::DrawContext &ctx) override;
	int getTextPosition(math::Vec mousePos) override;
};


} // namespace app
} // namespace rack
