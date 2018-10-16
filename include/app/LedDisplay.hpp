#pragma once
#include "app/common.hpp"


namespace rack {


struct LedDisplay : virtual Widget {
	void draw(NVGcontext *vg) override;
};

struct LedDisplaySeparator : TransparentWidget {
	LedDisplaySeparator();
	void draw(NVGcontext *vg) override;
};

struct LedDisplayChoice : TransparentWidget {
	std::string text;
	std::shared_ptr<Font> font;
	Vec textOffset;
	NVGcolor color;
	LedDisplayChoice();
	void draw(NVGcontext *vg) override;
	void onButton(event::Button &e) override;
};

struct LedDisplayTextField : TextField {
	std::shared_ptr<Font> font;
	Vec textOffset;
	NVGcolor color;
	LedDisplayTextField();
	void draw(NVGcontext *vg) override;
	int getTextPosition(Vec mousePos) override;
};


} // namespace rack
