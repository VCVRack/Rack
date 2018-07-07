/*
 * LOGgui.hpp
 *
 * GUI objects from LOGinstruments
 */

#ifndef SRC_LOGGUI_HPP_
#define SRC_LOGGUI_HPP_

#include <cstring>

#include "rack.hpp"

using namespace rack;

struct VCSPin4State : SVGSwitch, ToggleSwitch {
	VCSPin4State() {
		addFrame(SVG::load(assetPlugin(plugin, "res/VCSPinNone.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/VCSPinRed.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/VCSPinBlue.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/VCSPinBlack.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct VCSPin2State : SVGSwitch, ToggleSwitch {
	VCSPin2State() {
		addFrame(SVG::load(assetPlugin(plugin, "res/VCSPinNone.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/VCSPinWhite.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct baseTxtLabel : Widget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	char text[128];
	const int fh = 14;

	baseTxtLabel(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		setColor(0x00, 0x00, 0x00, 0xFF);
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
		setText(" ");
	}

	baseTxtLabel(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		setColor(r, g, b, a);
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
		setText(" ");
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	void setText(const char * txt) {
		strncpy(text, txt, sizeof(text));
		box.size.x = strlen(text) * 8;
	}

	void draw(NVGcontext *vg) override {
		Widget::draw(vg);
		drawTxt(vg, text);
	}

	void drawTxt(NVGcontext *vg, const char * txt) {

		Vec c = Vec(box.size.x/2, box.size.y);

		nvgFontSize(vg, fh);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgFillColor(vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));

		nvgText(vg, c.x, c.y+fh, txt, NULL);
	}

};

struct paperTxtLabel : baseTxtLabel {

	using baseTxtLabel::baseTxtLabel;

	void draw(NVGcontext *vg) override {
		Widget::draw(vg);
		drawPaperBG(vg);
		drawTxt(vg, text);
	}

	void drawPaperBG(NVGcontext *vg) {
		Vec c = Vec(box.size.x/2, box.size.y);
		const int whalf = box.size.x/2;

		// Draw rectangle
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xF0));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, c.x -whalf, c.y +2);
			nvgLineTo(vg, c.x +whalf, c.y +2);
			nvgQuadTo(vg, c.x +whalf +5, c.y +2+7, c.x +whalf, c.y+fh+2);
			nvgLineTo(vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(vg, c.x -whalf -5, c.y +2+7, c.x -whalf, c.y +2);
			nvgClosePath(vg);
		}
		nvgFill(vg);
		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0x0F));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, c.x -whalf, c.y +2);
			nvgLineTo(vg, c.x +whalf, c.y +2);
			nvgQuadTo(vg, c.x +whalf +5, c.y +2+7, c.x +whalf, c.y+fh+2);
			nvgLineTo(vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(vg, c.x -whalf -5, c.y +2+7, c.x -whalf, c.y +2);
			nvgClosePath(vg);
		}
		nvgStrokeWidth(vg, 0.5);
		nvgStroke(vg);
	}

};

struct txtKnob : RoundBlackKnob {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;

	txtKnob() {
		setColor(0x00, 0x00, 0x00, 0xFF);
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	txtKnob(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		setColor(r, g, b, a);
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		char tbuf[128];

		FramebufferWidget::draw(vg);
		//drawTxt(vg, "VAL", value); // this follows the cursor only if the param ranges from minAngle to maxAngle (e.g. +-0.83*PI)

		if ( maxValue - floor(maxValue)  == 0.0 ) {
			snprintf(tbuf, sizeof(tbuf), "%d", (int)maxValue);
		} else {
			snprintf(tbuf, sizeof(tbuf), "%.3G", maxValue);
		}
		drawTxt(vg, tbuf, maxAngle);

		if ( minValue - floor(minValue) == 0.0 ) {
			snprintf(tbuf, sizeof(tbuf), "%d", (int)minValue);
		} else {
			snprintf(tbuf, sizeof(tbuf), "%.3G", minValue);
		}
		drawTxt(vg, tbuf, minAngle);
	}

	void drawTxt(NVGcontext *vg, const char * txt, float angle) {
		int fh = 14;
		nvgFontSize(vg, fh);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);
		nvgFillColor(vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
		Vec c = Vec(box.size.x/2, box.size.y/2);
		float r = box.size.x / 2;
		float x = c.x + (r*sinf(angle) );
		float y = fh + c.y - (r*cosf(angle) );
		int xl = strlen(txt) * 10;
		int xs = 10; // a little spacing on the right
		nvgText(vg, angle > 0.0 ? (x + xs) : (x - xl), y, txt, NULL);
		/*
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		nvgBeginPath(vg);
		nvgMoveTo(vg, c.x, c.y);
		nvgLineTo(vg, 0, 0);
		nvgClosePath(vg);
		nvgStroke(vg);
		*/
	}
};

struct valueKnob : RoundBlackKnob {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;

	valueKnob() {
		setColor(0x00, 0x00, 0x00, 0xFF);
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	valueKnob(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		setColor(r, g, b, a);
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		char tbuf[128];

		FramebufferWidget::draw(vg);
		//drawTxt(vg, "VAL", value); // this follows the cursor only if the param ranges from minAngle to maxAngle (e.g. +-0.83*PI)

		snprintf(tbuf, sizeof(tbuf), "%.3G", value);
		drawValue(vg, tbuf);

	}

	void drawValue(NVGcontext *vg, const char * txt) {

		Vec c = Vec(box.size.x/2, box.size.y);
		const int fh = 14;
		const int whalf = 15;

		// Draw rectangle
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xF0));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, c.x -whalf, c.y +2);
			nvgLineTo(vg, c.x +whalf, c.y +2);
			nvgQuadTo(vg, c.x +whalf +5, c.y +2+7, c.x +whalf, c.y+fh+2);
			nvgLineTo(vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(vg, c.x -whalf -5, c.y +2+7, c.x -whalf, c.y +2);
			/*
			nvgMoveTo(vg, c.x -whalf, c.y +2);
			nvgLineTo(vg, c.x +whalf, c.y +2);
			nvgLineTo(vg, c.x +whalf, c.y+fh+2);
			nvgLineTo(vg, c.x -whalf, c.y+fh+2);
			nvgLineTo(vg, c.x -whalf, c.y +2);
			*/
			nvgClosePath(vg);
		}
		nvgFill(vg);
		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0x0F));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, c.x -whalf, c.y +2);
			nvgLineTo(vg, c.x +whalf, c.y +2);
			nvgQuadTo(vg, c.x +whalf +5, c.y +2+7, c.x +whalf, c.y+fh+2);
			nvgLineTo(vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(vg, c.x -whalf -5, c.y +2+7, c.x -whalf, c.y +2);
			nvgClosePath(vg);
		}
		nvgStrokeWidth(vg, 0.5);
		nvgStroke(vg);


		nvgFontSize(vg, fh);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgFillColor(vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));

		nvgText(vg, c.x, c.y+fh, txt, NULL);
		/*
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		nvgBeginPath(vg);
		nvgMoveTo(vg, c.x, c.y);
		nvgLineTo(vg, 0, 0);
		nvgClosePath(vg);
		nvgStroke(vg);
		*/
	}
};

struct valueSnapKnob : valueKnob {
	valueSnapKnob() {
		snap = true;
	}
};



#endif /* SRC_LOGGUI_HPP_ */
