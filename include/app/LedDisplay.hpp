#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/TransparentWidget.hpp"
#include "ui/TextField.hpp"


namespace rack {
namespace app {


struct LedDisplay : widget::OpaqueWidget {
	void draw(const DrawArgs &args) override;
};

struct LedDisplaySeparator : widget::TransparentWidget {
	LedDisplaySeparator();
	void draw(const DrawArgs &args) override;
};

struct LedDisplayChoice : widget::TransparentWidget {
	std::string text;
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	NVGcolor bgColor;
	LedDisplayChoice();
	void draw(const DrawArgs &args) override;
	void onButton(const widget::ButtonEvent &e) override;
};

struct LedDisplayTextField : ui::TextField {
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	LedDisplayTextField();
	void draw(const DrawArgs &args) override;
	int getTextPosition(math::Vec mousePos) override;
};


} // namespace app
} // namespace rack
