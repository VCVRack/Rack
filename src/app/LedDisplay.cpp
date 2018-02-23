#include "app.hpp"
#include "asset.hpp"
#include "window.hpp"


namespace rack {


void LedDisplay::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, nvgRGB(0x00, 0x00, 0x00));
	nvgFill(vg);

	Widget::draw(vg);
}


LedDisplaySeparator::LedDisplaySeparator() {
	box.size = Vec();
}

void LedDisplaySeparator::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0, 0);
	nvgLineTo(vg, box.size.x, box.size.y);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, nvgRGB(0x33, 0x33, 0x33));
	nvgStroke(vg);
}


LedDisplayChoice::LedDisplayChoice() {
	box.size = mm2px(Vec(0, 28.0 / 3));
	font = Font::load(assetGlobal("res/fonts/ShareTechMono-Regular.ttf"));
	color = nvgRGB(0xff, 0xd7, 0x14);
}

void LedDisplayChoice::draw(NVGcontext *vg) {
	if (font->handle < 0)
		return;

	nvgFillColor(vg, color);
	nvgFontFaceId(vg, font->handle);
	nvgTextLetterSpacing(vg, 0.0);

	Vec textPos = Vec(10, 18);
	nvgFontSize(vg, 12);
	nvgText(vg, textPos.x, textPos.y, text.c_str(), NULL);
}

void LedDisplayChoice::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		EventAction eAction;
		onAction(eAction);
		e.consumed = true;
		e.target = this;
	}
}


LedDisplayTextField::LedDisplayTextField() {
	font = Font::load(assetGlobal("res/fonts/ShareTechMono-Regular.ttf"));
	color = nvgRGB(0xff, 0xd7, 0x14);
}

static const Vec textFieldPos = Vec(5, 5);

void LedDisplayTextField::draw(NVGcontext *vg) {
	// Background
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, nvgRGB(0x00, 0x00, 0x00));
	nvgFill(vg);

	// Text
	if (font->handle < 0)
		return;

	bndSetFont(font->handle);

	NVGcolor highlightColor = color;
	highlightColor.a = 0.5;
	int cend = (this == gFocusedWidget) ? end : -1;
	bndIconLabelCaret(vg, textFieldPos.x, textFieldPos.y,
		box.size.x - 2*textFieldPos.x, box.size.y - 2*textFieldPos.y,
		-1, color, 12, text.c_str(), highlightColor, begin, cend);

	bndSetFont(gGuiFont->handle);
}

int LedDisplayTextField::getTextPosition(Vec mousePos) {
	bndSetFont(font->handle);
	int textPos = bndIconLabelTextPosition(gVg, textFieldPos.x, textFieldPos.y,
		box.size.x - 2*textFieldPos.x, box.size.y - 2*textFieldPos.y,
		-1, 12, text.c_str(), mousePos.x, mousePos.y);
	bndSetFont(gGuiFont->handle);
	return textPos;
}


} // namespace rack
