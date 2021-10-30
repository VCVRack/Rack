#pragma once
#include <app/common.hpp>
#include <widget/Widget.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/TextField.hpp>


namespace rack {
namespace app {


struct LedDisplay : widget::Widget {
	void draw(const DrawArgs& args) override;
	void drawLayer(const DrawArgs& args, int layer) override;
};

struct LedDisplaySeparator : widget::Widget {
	LedDisplaySeparator();
	void draw(const DrawArgs& args) override;
};

struct LedDisplayChoice : widget::OpaqueWidget {
	std::string text;
	std::string fontPath;
	math::Vec textOffset;
	NVGcolor color;
	NVGcolor bgColor;
	LedDisplayChoice();
	void draw(const DrawArgs& args) override;
	void drawLayer(const DrawArgs& args, int layer) override;
	void onButton(const ButtonEvent& e) override;
};

struct LedDisplayTextField : ui::TextField {
	std::string fontPath;
	math::Vec textOffset;
	NVGcolor color;
	NVGcolor bgColor;
	LedDisplayTextField();
	void draw(const DrawArgs& args) override;
	void drawLayer(const DrawArgs& args, int layer) override;
	int getTextPosition(math::Vec mousePos) override;
};


} // namespace app
} // namespace rack
