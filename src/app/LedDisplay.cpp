#include "app/LedDisplay.hpp"
#include "asset.hpp"
#include "window.hpp"
#include "event.hpp"
#include "context.hpp"


namespace rack {


void LedDisplay::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, nvgRGB(0x00, 0x00, 0x00));
	nvgFill(vg);

	Widget::draw(vg);
}


LedDisplaySeparator::LedDisplaySeparator() {
	box.size = math::Vec();
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
	box.size = mm2px(math::Vec(0, 28.0 / 3));
	font = Font::load(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
	color = nvgRGB(0xff, 0xd7, 0x14);
	textOffset = math::Vec(10, 18);
}

void LedDisplayChoice::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);

	if (font->handle >= 0) {
		nvgFillColor(vg, color);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 0.0);

		nvgFontSize(vg, 12);
		nvgText(vg, textOffset.x, textOffset.y, text.c_str(), NULL);
	}

	nvgResetScissor(vg);
}

void LedDisplayChoice::onButton(const event::Button &e) {
	if (e.action == GLFW_PRESS && (e.button == GLFW_MOUSE_BUTTON_LEFT || e.button == GLFW_MOUSE_BUTTON_RIGHT)) {
		event::Action eAction;
		onAction(eAction);
		e.consume(this);
	}
}


LedDisplayTextField::LedDisplayTextField() {
	font = Font::load(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
	color = nvgRGB(0xff, 0xd7, 0x14);
	textOffset = math::Vec(5, 5);
}


void LedDisplayTextField::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);

	// Background
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, nvgRGB(0x00, 0x00, 0x00));
	nvgFill(vg);

	// Text
	if (font->handle >= 0) {
		bndSetFont(font->handle);

		NVGcolor highlightColor = color;
		highlightColor.a = 0.5;
		int begin = std::min(cursor, selection);
		int end = (this == context()->event->selectedWidget) ? std::max(cursor, selection) : -1;
		bndIconLabelCaret(vg, textOffset.x, textOffset.y,
			box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
			-1, color, 12, text.c_str(), highlightColor, begin, end);

		bndSetFont(context()->window->uiFont->handle);
	}

	nvgResetScissor(vg);
}

int LedDisplayTextField::getTextPosition(math::Vec mousePos) {
	bndSetFont(font->handle);
	int textPos = bndIconLabelTextPosition(context()->window->vg, textOffset.x, textOffset.y,
		box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
		-1, 12, text.c_str(), mousePos.x, mousePos.y);
	bndSetFont(context()->window->uiFont->handle);
	return textPos;
}


} // namespace rack
