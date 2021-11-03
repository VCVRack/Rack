#include <app/LedDisplay.hpp>
#include <asset.hpp>
#include <window/Window.hpp>
#include <context.hpp>


namespace rack {
namespace app {


void LedDisplay::draw(const DrawArgs& args) {
	math::Rect r = box.zeroPos();

	// Black background
	nvgBeginPath(args.vg);
	nvgRect(args.vg, RECT_ARGS(r));
	NVGcolor topColor = nvgRGB(0x22, 0x22, 0x22);
	NVGcolor bottomColor = nvgRGB(0x12, 0x12, 0x12);
	nvgFillPaint(args.vg, nvgLinearGradient(args.vg, 0.0, 0.0, 0.0, 25.0, topColor, bottomColor));
	// nvgFillColor(args.vg, bottomColor);
	nvgFill(args.vg);

	// Outer strokes
	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, 0.0, -0.5);
	nvgLineTo(args.vg, box.size.x, -0.5);
	nvgStrokeColor(args.vg, nvgRGBAf(0, 0, 0, 0.24));
	nvgStrokeWidth(args.vg, 1.0);
	nvgStroke(args.vg);

	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, 0.0, box.size.y + 0.5);
	nvgLineTo(args.vg, box.size.x, box.size.y + 0.5);
	nvgStrokeColor(args.vg, nvgRGBAf(1, 1, 1, 0.30));
	nvgStrokeWidth(args.vg, 1.0);
	nvgStroke(args.vg);

	// Inner strokes
	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, 0.0, 2.5);
	nvgLineTo(args.vg, box.size.x, 2.5);
	nvgStrokeColor(args.vg, nvgRGBAf(1, 1, 1, 0.20));
	nvgStrokeWidth(args.vg, 1.0);
	nvgStroke(args.vg);

	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, 0.0, box.size.y - 2.5);
	nvgLineTo(args.vg, box.size.x, box.size.y - 2.5);
	nvgStrokeColor(args.vg, nvgRGBAf(1, 1, 1, 0.20));
	nvgStrokeWidth(args.vg, 1.0);
	nvgStroke(args.vg);

	// Black border
	math::Rect rBorder = r.shrink(math::Vec(1, 1));
	nvgBeginPath(args.vg);
	nvgRect(args.vg, RECT_ARGS(rBorder));
	nvgStrokeColor(args.vg, bottomColor);
	nvgStrokeWidth(args.vg, 2.0);
	nvgStroke(args.vg);

	// Draw children inside box
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));
	Widget::draw(args);
	nvgResetScissor(args.vg);
}


void LedDisplay::drawLayer(const DrawArgs& args, int layer) {
	// Draw children inside box
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));
	Widget::drawLayer(args, layer);
	nvgResetScissor(args.vg);
}


LedDisplaySeparator::LedDisplaySeparator() {
	box.size = math::Vec();
}


void LedDisplaySeparator::draw(const DrawArgs& args) {
	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, 0, 0);
	nvgLineTo(args.vg, box.size.x, box.size.y);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, nvgRGB(0x33, 0x33, 0x33));
	nvgStroke(args.vg);
}


LedDisplayChoice::LedDisplayChoice() {
	box.size = window::mm2px(math::Vec(0, 28.0 / 3));
	fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
	textOffset = math::Vec(10, 18);
	color = nvgRGB(0xff, 0xd7, 0x14);
	bgColor = nvgRGBAf(0, 0, 0, 0);
}


void LedDisplayChoice::draw(const DrawArgs& args) {
	if (bgColor.a > 0.0) {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(args.vg, bgColor);
		nvgFill(args.vg);
	}

	Widget::draw(args);
}


void LedDisplayChoice::drawLayer(const DrawArgs& args, int layer) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	if (layer == 1) {
		std::shared_ptr<window::Font> font = APP->window->loadFont(fontPath);
		if (font && font->handle >= 0) {
			nvgFillColor(args.vg, color);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0.0);

			nvgFontSize(args.vg, 12);
			nvgText(args.vg, textOffset.x, textOffset.y, text.c_str(), NULL);
		}
	}

	Widget::drawLayer(args, layer);
	nvgResetScissor(args.vg);
}


void LedDisplayChoice::onButton(const ButtonEvent& e) {
	OpaqueWidget::onButton(e);

	if (e.action == GLFW_PRESS && (e.button == GLFW_MOUSE_BUTTON_LEFT || e.button == GLFW_MOUSE_BUTTON_RIGHT)) {
		ActionEvent eAction;
		onAction(eAction);
		e.consume(this);
	}
}


LedDisplayTextField::LedDisplayTextField() {
	fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
	textOffset = math::Vec(5, 5);
	color = nvgRGB(0xff, 0xd7, 0x14);
	bgColor = nvgRGB(0x00, 0x00, 0x00);
}


void LedDisplayTextField::draw(const DrawArgs& args) {
	Widget::draw(args);
}


void LedDisplayTextField::drawLayer(const DrawArgs& args, int layer) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	if (layer == 1) {
		// Text
		std::shared_ptr<window::Font> font = APP->window->loadFont(fontPath);
		if (font && font->handle >= 0) {
			bndSetFont(font->handle);

			NVGcolor highlightColor = color;
			highlightColor.a = 0.5;
			int begin = std::min(cursor, selection);
			int end = (this == APP->event->selectedWidget) ? std::max(cursor, selection) : -1;
			bndIconLabelCaret(args.vg,
				textOffset.x, textOffset.y,
				box.size.x - 2 * textOffset.x, box.size.y - 2 * textOffset.y,
				-1, color, 12, text.c_str(), highlightColor, begin, end);

			bndSetFont(APP->window->uiFont->handle);
		}
	}

	Widget::drawLayer(args, layer);
	nvgResetScissor(args.vg);
}


int LedDisplayTextField::getTextPosition(math::Vec mousePos) {
	std::shared_ptr<window::Font> font = APP->window->loadFont(fontPath);
	if (!font || !font->handle)
		return 0;

	bndSetFont(font->handle);
	int textPos = bndIconLabelTextPosition(APP->window->vg,
		textOffset.x, textOffset.y,
		box.size.x - 2 * textOffset.x, box.size.y - 2 * textOffset.y,
		-1, 12, text.c_str(), mousePos.x, mousePos.y);
	bndSetFont(APP->window->uiFont->handle);
	return textPos;
}


} // namespace app
} // namespace rack
