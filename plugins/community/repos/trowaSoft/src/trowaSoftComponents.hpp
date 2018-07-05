#ifndef TROWASOFT_COMPONENTS_HPP
#define TROWASOFT_COMPONENTS_HPP

#include "rack.hpp"
using namespace rack;

#include <string.h>
#include <stdio.h>
#include "window.hpp"
#include "ui.hpp"
#include "util/math.hpp"
#include "dsp/digital.hpp"
#include "componentlibrary.hpp"
#include "plugin.hpp"
#include "trowaSoftUtilities.hpp"
#include "util/color.hpp"

#ifdef USE_VST2
#define plugin "trowaSoft"
#endif // USE_VST2


//=======================================================
// trowaSoft - TurtleMonkey Components 
//=======================================================


//::: Helpers :::::::::::::::::::::::::::::::::::::::::::
// Extra colors (may be defined in future?)
#ifndef COLOR_MAGENTA
	#define COLOR_MAGENTA nvgRGB(240, 50, 230)
#endif
#ifndef COLOR_LIME
	#define COLOR_LIME nvgRGB(210, 245, 60)
#endif
#ifndef COLOR_PINK
	#define COLOR_PINK nvgRGB(250, 190, 190)
#endif
#ifndef COLOR_TEAL
	#define COLOR_TEAL nvgRGB(0, 128, 128)
#endif
#ifndef COLOR_LAVENDER
	#define COLOR_LAVENDER nvgRGB(230, 190, 255)
#endif
#ifndef COLOR_BROWN
	#define COLOR_BROWN nvgRGB(170, 110, 40)
#endif
#ifndef COLOR_BEIGE
	#define COLOR_BEIGE nvgRGB(255, 250, 200)
#endif
#ifndef COLOR_MAROON
	#define COLOR_MAROON nvgRGB(128, 0, 0)
#endif
#ifndef COLOR_MINT
	#define COLOR_MINT nvgRGB(170, 255, 195)
#endif
#ifndef COLOR_OLIVE
	#define COLOR_OLIVE nvgRGB(128, 128, 0)
#endif
#ifndef COLOR_CORAL
	#define COLOR_CORAL nvgRGB(255, 215, 180)
#endif
#ifndef COLOR_NAVY
	#define COLOR_NAVY nvgRGB(0, 0, 128)
#endif
#ifndef COLOR_DARK_ORANGE
	#define COLOR_DARK_ORANGE nvgRGB(0xFF, 0x8C, 0x00)
#endif 
#ifndef COLOR_PUMPKIN_ORANGE
	#define COLOR_PUMPKIN_ORANGE nvgRGB(0xF8, 0x72, 0x17)
#endif 
#ifndef COLOR_DARK_GRAY
	#define COLOR_DARK_GRAY nvgRGB(0x33, 0x33, 0x33)
#endif

#ifndef COLOR_TS_RED
	#define COLOR_TS_RED nvgRGB(0xFF, 0x00, 0x00)
#endif 
#ifndef COLOR_TS_ORANGE
	// Orange in components.hpp is different
	#define COLOR_TS_ORANGE nvgRGB(0xFF, 0xA5, 0x00)
#endif 
#ifndef COLOR_TS_GREEN
	#define COLOR_TS_GREEN nvgRGB(0x00, 0xFF, 0x00)
#endif 
#ifndef COLOR_TS_BLUE
	#define COLOR_TS_BLUE nvgRGB(0x33, 0x66, 0xFF)
#endif 
#ifndef COLOR_TS_GRAY
	#define COLOR_TS_GRAY nvgRGB(0xAA, 0xAA, 0xAB)
#endif 

#ifndef  KNOB_SENSITIVITY
#define KNOB_SENSITIVITY 0.0015
#endif // ! KNOB_SENSITIVITY


//-----------------------------------------------------------------
// Form controls - Default colors and such
//-----------------------------------------------------------------
#define FORMS_DEFAULT_TEXT_COLOR		nvgRGB(0xee, 0xee, 0xee)
#define FORMS_DEFAULT_BORDER_COLOR		nvgRGB(0x66, 0x66, 0x66)
#define FORMS_DEFAULT_BG_COLOR			COLOR_BLACK


//--------------------------------------------------------------
// ColorValueLight - Sorta like the old ColorValueLight that was in Rack.
//--------------------------------------------------------------
struct ColorValueLight : ModuleLightWidget {
	NVGcolor baseColor;
	// Pixels to add for outer radius (either px or relative %).
	float outerRadiusHalo = 0.35;
	bool outerRadiusRelative = true;
	ColorValueLight()
	{
		bgColor = nvgRGB(0x20, 0x20, 0x20);
		borderColor = nvgRGBA(0, 0, 0, 0);
		return;
	};
	virtual ~ColorValueLight(){};
	// Set a single color
	void setColor(NVGcolor bColor)
	{
		color = bColor;
		baseColor = bColor;
		if (baseColors.size() < 1)
		{
			baseColors.push_back(bColor);			
		}
		else
		{
			baseColors[0] = bColor;
		}
	}
	void drawHalo(NVGcontext *vg) override {
		float radius = box.size.x / 2.0;
		float oradius = radius + ((outerRadiusRelative) ? (radius*outerRadiusHalo) : outerRadiusHalo);

		nvgBeginPath(vg);
		nvgRect(vg, radius - oradius, radius - oradius, 2 * oradius, 2 * oradius);

		NVGpaint paint;
		NVGcolor icol = colorMult(color, 0.10);
		NVGcolor ocol = nvgRGB(0, 0, 0);
		paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
		nvgFillPaint(vg, paint);
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgFill(vg);
	}
};

//------------------------------------------------------------------------------------------------
// TS_Label : Label with the trowaSoft default label font.
//------------------------------------------------------------------------------------------------
struct TS_Label : Label {
	// Font size. Default is 10.
	int fontSize = 10;
	// Font face
	std::shared_ptr<Font> font;
	// The font color. Default is Dark Gray.
	NVGcolor textColor = COLOR_DARK_GRAY;
	enum TextAlignment {
		Left,
		Center,
		Right
	};
	// Text alignment. Default is Left.
	TextAlignment textAlign = TextAlignment::Left;
	enum VerticalAlignment {
		Baseline,
		Top,
		Middle,
		Bottom
	};
	// Vertical alignment. Default is Baseline.
	VerticalAlignment verticalAlign = VerticalAlignment::Baseline;
	// The text letter spacing (default 1.0f).
	float textLetterSpacing = 1.0f;

	bool drawBackground = false;
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	float padding = 0.f;

	TS_Label(const char* fontPath)
	{
		font = Font::load(assetPlugin(plugin, fontPath));
		return;
	}
	TS_Label() : TS_Label(TROWA_LABEL_FONT)
	{
		return;
	}
	TS_Label(const char* fontPath, Vec size) : TS_Label(fontPath)
	{
		box.size = size;
		return;
	}
	TS_Label(Vec size) : TS_Label(TROWA_LABEL_FONT)
	{
		box.size = size;
		return;
	}
	TS_Label(const char* fontPath, std::string text, Vec size) : TS_Label(fontPath, size)
	{
		this->text = text;
		return;
	}
	TS_Label(std::string text, Vec size) : TS_Label(size)
	{
		this->text = text;
		return;
	}

	void draw(NVGcontext *vg) override
	{
		if (visible)
		{
			//nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER);//Restore to default.
			if (drawBackground)
			{
				nvgBeginPath(vg);
				nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
				nvgFillColor(vg, backgroundColor);
				nvgFill(vg);
				nvgStrokeWidth(vg, 1.0);
				nvgStrokeColor(vg, borderColor);
				nvgStroke(vg);
			}

			nvgFontFaceId(vg, font->handle);
			nvgFontSize(vg, fontSize);
			nvgTextLetterSpacing(vg, textLetterSpacing);
			float x = 0;
			float y = 0;
			uint8_t alignment = 0x00;
			switch (textAlign)
			{
			case TextAlignment::Center:
				x = box.size.x / 2.0;
				alignment = NVGalign::NVG_ALIGN_CENTER;
				break;
			case TextAlignment::Right:
				x = box.size.x - padding;
				alignment = NVGalign::NVG_ALIGN_RIGHT;
				break;
			case TextAlignment::Left:
			default:
				x = 0.0f + padding;
				alignment = NVGalign::NVG_ALIGN_LEFT;
				break;
			}
			switch (verticalAlign)
			{
			case VerticalAlignment::Middle:
				y = box.size.y / 2.0;
				alignment |= NVGalign::NVG_ALIGN_MIDDLE;
				break;
			case VerticalAlignment::Bottom:
				y = box.size.y - padding;
				alignment |= NVGalign::NVG_ALIGN_BOTTOM;
				break;
			case VerticalAlignment::Top:
				y = 0.0f + padding;
				alignment |= NVGalign::NVG_ALIGN_TOP;
				break;
			case VerticalAlignment::Baseline:
			default:
				y = 0.0f + padding;
				alignment |= NVGalign::NVG_ALIGN_BASELINE;  // Default, align text vertically to baseline.
				break;
			}
			nvgTextAlign(vg, alignment);
			nvgFillColor(vg, textColor);
			nvgText(vg, x, y, text.c_str(), NULL);
		}
		return;
	}

};

//--------------------------------------------------------------
// TS_PadSwitch
// Empty momentary switch of given size.
//--------------------------------------------------------------
struct TS_PadSwitch : MomentarySwitch {
	int btnId = -1;
	// Group id (to match guys that should respond to mouse down drag).
	int groupId = -1;
	TS_PadSwitch() {
		return;
	}
	TS_PadSwitch(Vec size) {
		box.size = size;		
		return;
	}
	// Allow mouse-down & drag to set buttons (i.e. on Sequencer grid where there are many buttons). 
	// Suggestion from @LKHSogpit, Solution from @AndrewBelt.
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/7
	// https://github.com/VCVRack/Rack/issues/607
	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(EventDragStart &e) override {
		float newVal = (value < maxValue) ? maxValue : minValue;
		//debug("onDragStart(%d) - Current Value is %.1f, setting to %.1f.", btnId, value, newVal);
		setValue(newVal); // Toggle Value
		return;
	}
	/** Called when the left button is released and this widget is being dragged */
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/12
	// Last button keeps pressed down.
	void onDragEnd(EventDragEnd &e) override {
		//debug("onDragEnd(%d) - Current Value is %.1f, setting to %.1f (off).", btnId, value, minValue);
		setValue(minValue); // Turn Off
		return;
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(EventDragEnter &e) override {
		// Set these no matter what because if you drag back onto your starting square, you want to toggle it again.
		TS_PadSwitch *origin = dynamic_cast<TS_PadSwitch*>(e.origin);
		if (origin && origin->groupId == this->groupId) {
			float newVal = (value < maxValue) ? maxValue : minValue;
			//debug("onDragEnter(%d) - Current Value is %.1f, setting to %.1f.", btnId, value, newVal);
			setValue(newVal); // Toggle Value
		}
	}
	void onDragLeave(EventDragEnter &e) override {
		TS_PadSwitch *origin = dynamic_cast<TS_PadSwitch*>(e.origin);
		if (origin && origin->groupId == this->groupId) {
			//debug("onDragLeave(%d) - Current Value is %.1f, setting to %.1f (off).", btnId, value, minValue);
			setValue(minValue); // Turn Off
		}
	}
	void onMouseUp(EventMouseUp &e) override {
		//debug("onMouseUp(%d) - Current Value is %.1f, setting to %.1f (off).", btnId, value, minValue);
		setValue(minValue); // Turn Off
	};

};

//--------------------------------------------------------------
// TS_PadSquare - A Square Pad button.
//--------------------------------------------------------------
struct TS_PadSquare : SVGSwitch, TS_PadSwitch {
	TS_PadSquare() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_0.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
	TS_PadSquare(Vec size)
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/ComponentLibrary/TS_pad_0.svg")));
		sw->box.size = size;
		box.size = size;
	}
};

//--------------------------------------------------------------
// TS_PadBtn - A wide Pad button. (Empty text)
//--------------------------------------------------------------
struct TS_PadBtn : SVGSwitch, MomentarySwitch {
	
	TS_PadBtn() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_btn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_btn_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

//--------------------------------------------------------------
// TS_PadRun - A wide Pad button. (RUN >)
//--------------------------------------------------------------
struct TS_Pad_Run : SVGSwitch, MomentarySwitch {
	
	TS_Pad_Run() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_run_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_run_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

//--------------------------------------------------------------
// TS_PadReset - A wide Pad button. (< RST)
//--------------------------------------------------------------
struct TS_Pad_Reset : SVGSwitch, MomentarySwitch {
	
	TS_Pad_Reset() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_reset_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_reset_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

struct HideableLEDButton : LEDButton
{

	void draw(NVGcontext *vg) override {
		if (visible) {
			LEDButton::draw(vg);
		}
	}

	void onMouseDown(EventMouseDown &e) override {
		if (visible) {
			LEDButton::onMouseDown(e);
		}
	};
	void onMouseUp(EventMouseUp &e) override {
		if (visible) {
			LEDButton::onMouseUp(e);
		}
	};
	/** Called on every frame, even if mouseRel = Vec(0, 0) */
	void onMouseMove(EventMouseMove &e) override {
		if (visible) {
			LEDButton::onMouseMove(e);
		}
	}
	void onHoverKey(EventHoverKey &e) override {
		if (visible) {
			LEDButton::onHoverKey(e);
		}
	};
	///** Called when this widget begins responding to `onMouseMove` events */
	//virtual void onMouseEnter(EventMouseEnter &e) {}
	///** Called when another widget begins responding to `onMouseMove` events */
	//virtual void onMouseLeave(EventMouseLeave &e) {}
	//virtual void onFocus(EventFocus &e) {}
	//virtual void onDefocus(EventDefocus &e) {}
	//virtual void onScroll(EventScroll &e);

	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(EventDragStart &e) override {
		if (visible) {
			LEDButton::onDragStart(e);
		}
	}
	/** Called when the left button is released and this widget is being dragged */
	void onDragEnd(EventDragEnd &e) override {
		if (visible) {
			LEDButton::onDragEnd(e);
		}
	}
	/** Called when a widget responds to `onMouseMove` and is being dragged */
	void onDragMove(EventDragMove &e) override {
		if (visible) {
			LEDButton::onDragMove(e);
		}
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(EventDragEnter &e) override {
		if (visible) {
			LEDButton::onDragEnter(e);
		}
	}
	void onDragLeave(EventDragEnter &e) override {
		if (visible) {
			LEDButton::onDragLeave(e);
		}
	}
	void onDragDrop(EventDragDrop &e) override {
		if (visible) {
			LEDButton::onDragDrop(e);
		}
	}
	void onPathDrop(EventPathDrop &e)override {
		if (visible) {
			LEDButton::onPathDrop(e);
		}
	}

	void onAction(EventAction &e) override {
		if (visible) {
			LEDButton::onAction(e);
		}
	}


};
//--------------------------------------------------------------
// TS_ScreenBtn - Screen button.
//--------------------------------------------------------------
struct TS_ScreenBtn : MomentarySwitch {
	bool visible = true;
	// Text to display on the btn.
	std::string btnText;
	// Background color
	NVGcolor backgroundColor = nvgRGBA(0, 0, 0, 0);
	// Text color
	NVGcolor color = COLOR_TS_GRAY;
	// Border color
	NVGcolor borderColor = COLOR_TS_GRAY;
	// Border width
	int borderWidth = 1;
	// Corner radius. 0 for straight corners.
	int cornerRadius = 5;
	// Font size for our display numbers
	int fontSize = 10;
	// Font face
	std::shared_ptr<Font> font = NULL;
	// Padding
	int padding = 1;

	enum TextAlignment {
		Left,
		Center,
		Right
	};

	TextAlignment textAlign = TextAlignment::Center;

	//TS_ScreenBtn()
	//{		
	//	return;
	//}
	TS_ScreenBtn(Vec size, Module* module, int paramId, std::string text, float minVal, float maxVal, float defVal)
	{
		box.size = size;
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 10;
		btnText = text;
		this->module = module;
		this->paramId = paramId;
		this->value = 0.0;
		this->minValue = minVal;
		this->maxValue = maxVal;
		this->defaultValue = defVal;
		return;
	}
	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(EventDragStart &e) override {
		if (visible) {
			MomentarySwitch::onDragStart(e);
		}
		return;
	}
	/** Called when the left button is released and this widget is being dragged */
	void onDragEnd(EventDragEnd &e) override {
		if (visible) {
			MomentarySwitch::onDragEnd(e);
		}
		return;
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(EventDragEnter &e) override {
		if (visible) {
			MomentarySwitch::onDragEnter(e);
		}
	}
	void onMouseDown(EventMouseDown &e) override {
		if (visible) {
			MomentarySwitch::onMouseDown(e);
		}
	}

	void setVisible(bool visible) {
		this->visible = visible;
		return;
	}

	virtual void draw(NVGcontext *vg) override
	{
		if (!visible)
			return;
		// Background
		nvgBeginPath(vg);
		if (cornerRadius > 0)
			nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		else
			nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		// Background - border.
		if (borderWidth > 0) {
			nvgStrokeWidth(vg, borderWidth);
			nvgStrokeColor(vg, borderColor);
			nvgStroke(vg);
		}
		// Text
		nvgBeginPath(vg);
		nvgScissor(vg, padding, padding, box.size.x - 2*padding, box.size.y - 2*padding);
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, color);

		float x, y;
		NVGalign nvgAlign;
		y = box.size.y / 2.0f;
		switch (textAlign) {
			case TextAlignment::Left:
				nvgAlign = NVG_ALIGN_LEFT;
				x = box.size.x + padding;
				break;
			case TextAlignment::Right:
				nvgAlign = NVG_ALIGN_RIGHT;
				x = box.size.x - padding;
				break;
			case TextAlignment::Center:
			default:
				nvgAlign = NVG_ALIGN_CENTER;
				x = box.size.x / 2.0f;
				break;
		}

		nvgTextAlign(vg, nvgAlign | NVG_ALIGN_MIDDLE);
		nvgText(vg, x, y, btnText.c_str(), NULL);
		nvgResetScissor(vg);

		return;
	}
};
//--------------------------------------------------------------
// TS_ScreenCheckBox : Screen checkbox button
//--------------------------------------------------------------
struct TS_ScreenCheckBox : TS_ScreenBtn {
	// If the check box should show as checked.
	bool checked = false;
	float checkBoxWidth = 14;
	float checkBoxHeight = 14;

	TS_ScreenCheckBox(Vec size, Module* module, int paramId, std::string text, float minVal, float maxVal, float defVal)
		: TS_ScreenBtn(size, module, paramId, text, minVal, maxVal, defVal)
	{
		return;
	}

	void draw(NVGcontext *vg) override
	{
		if (!visible)
			return;
		// Background
		nvgBeginPath(vg);
		if (cornerRadius > 0)
			nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		else
			nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		// Background - border.
		if (borderWidth > 0) {
			nvgStrokeWidth(vg, borderWidth);
			nvgStrokeColor(vg, borderColor);
			nvgStroke(vg);
		}

		// Text
		nvgBeginPath(vg);
		nvgScissor(vg, padding, padding, box.size.x - 2 * padding, box.size.y - 2 * padding);
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, color);

		float x, y;
		NVGalign nvgAlign;
		y = box.size.y / 2.0f;
		switch (textAlign) {
		case TextAlignment::Left:
			nvgAlign = NVG_ALIGN_LEFT;
			x = box.size.x + padding;
			break;
		case TextAlignment::Right:
			nvgAlign = NVG_ALIGN_RIGHT;
			x = box.size.x - padding;
			break;
		case TextAlignment::Center:
		default:
			nvgAlign = NVG_ALIGN_CENTER;
			x = box.size.x / 2.0f;
			break;
		}
		nvgTextAlign(vg, nvgAlign | NVG_ALIGN_MIDDLE);
		float txtBounds[4] = { 0,0,0,0 };
		nvgTextBounds(vg, x, y, btnText.c_str(), NULL, txtBounds);
		nvgText(vg, x, y, btnText.c_str(), NULL);
		nvgResetScissor(vg);

		// Check box ::::::::::::::::::::::::::::::::::::::::::::::

		float boxX = txtBounds[0] - checkBoxWidth - padding;
		float boxY = y - checkBoxHeight / 2.0 - padding;
		nvgBeginPath(vg);
		nvgRoundedRect(vg, boxX, boxY, checkBoxWidth, checkBoxHeight, 3);
		nvgStrokeColor(vg, color);
		nvgStrokeWidth(vg, 1.0f);
		nvgStroke(vg);

		if (checked)
		{
			nvgBeginPath(vg);
			nvgRoundedRect(vg, boxX + padding, boxY + padding, checkBoxWidth - padding * 2, checkBoxHeight - padding*2, 3);
			nvgFillColor(vg, color);
			nvgFill(vg);
		}

		return;
	}
};

//------------------------------------------------------------------------------------------------
// TS_LightArc - Lighted arc for light knobs
//------------------------------------------------------------------------------------------------
struct TS_LightArc : ColorValueLight {
	// The inner radius 
	float innerRadius = 22;	
	// Pointer to current angle in radians. This is the differential like from a knob.
	float* currentAngle_radians;
	// Font size for our display numbers
	int fontSize;
	// Font face
	std::shared_ptr<Font> font;
	// Numeric value to print out
	float* numericValue;
	// Buffer for our light string.
	char lightString[10];
	// The point where the angle is considered 0 degrees / radians.
	float zeroAnglePoint;
	// Pointer to the Sequencer Value mode information.
	ValueSequencerMode* valueMode;
	
	TS_LightArc()
	{
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 10;
		bgColor = nvgRGBAf(0.0, 0, 0, /*alpha */ 1.0);
		baseColor = COLOR_WHITE;
		zeroAnglePoint = TROWA_ANGLE_STRAIGHT_UP_RADIANS;
	}	
	void draw(NVGcontext *vg) override
	{
		float oradius = box.size.x / 2.0; // 25
		float radius = oradius - 2; // 23
		
		float angle = *currentAngle_radians;
		zeroAnglePoint = valueMode->zeroPointAngle_radians;
		int dir = (angle < zeroAnglePoint) ? NVG_CCW : NVG_CW;

		// Background - Solid
		nvgBeginPath(vg);
		nvgCircle(vg, oradius, oradius, innerRadius);
		nvgFillColor(vg, bgColor);
		nvgFill(vg);
		
		nvgStrokeWidth(vg, radius - innerRadius);
		NVGcolor borderColor = color;// bgColor;
		borderColor.a *= 0.5;//1.0;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		// svg Angles go clockwise from positive x -->
		
		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgCircle(vg, oradius, oradius, radius);		
		borderColor = color;
		borderColor.a = 0.25;
		nvgStrokeWidth(vg, oradius - radius);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		
		nvgBeginPath(vg);
		//nvgArcTo(vg, oradius, oradius, float x2, float y2, float radius);
		// Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
		// and the arc is drawn from angle a0 to a1, and swept in direction dir (NVG_CCW, or NVG_CW).
		// Angles are specified in radians.
		// nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
		nvgArc(vg, /*cx*/ oradius, /*cy*/ oradius, /*radius*/ innerRadius, 
			/*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
		nvgStrokeWidth(vg, oradius - innerRadius);
		borderColor = baseColor;
		borderColor.a *= 0.7;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		// Outer glow
		nvgBeginPath(vg);
		nvgArc(vg, /*cx*/ oradius, /*cy*/ oradius, innerRadius - 3, 
			 /*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
	
		NVGpaint paint;
		NVGcolor icol = color;
		icol.a *= 0.8;
		NVGcolor ocol = color;
		ocol.a = 0.0;
		paint = nvgRadialGradient(vg, oradius, oradius, innerRadius, oradius, icol, ocol);
		nvgStrokeWidth(vg, oradius - innerRadius + 3);	
		nvgStrokePaint(vg, paint);
		nvgStroke(vg);
					
		if (numericValue != NULL)
		{
			nvgBeginPath(vg);
			nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER);//Restore to default.
			NVGcolor textColor = COLOR_WHITE;
			nvgFontSize(vg, fontSize); 	
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			float v = valueMode->GetOutputValue(*numericValue);
			valueMode->GetDisplayString(v, lightString);
			nvgFillColor(vg, textColor);
			nvgText(vg, oradius, oradius, lightString, NULL);
		}
		return;
	}
}; // end TS_LightArc

//------------------------------------------------------------------------------------------------
// TS_LightedKnob - Knob to be used with light arcs. (not actually lit itself)
//------------------------------------------------------------------------------------------------
struct TS_LightedKnob : SVGKnob {
	float currentAngle;
	float differentialAngle;
	const float zeroAnglePoint = TROWA_ANGLE_STRAIGHT_UP_RADIANS;
	int id = 0; // For debugging.

	TS_LightedKnob() {
		minAngle = -0.83*NVG_PI;
		maxAngle = 0.83*NVG_PI;
		box.size = Vec(50, 50);
		currentAngle = 0;
		minValue = -10;
		maxValue = 10;
		snap = false;
		return;
	}
	TS_LightedKnob(int id) : TS_LightedKnob() {
		this->id = id;
	}
	void randomize() override { return; }	
	void setKnobValue(float val)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
		float prevVal = value;
#endif
		value = val;
		differentialAngle = rescale(value, minValue, maxValue, minAngle, maxAngle);
		currentAngle = zeroAnglePoint + differentialAngle;		
		this->dirty = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
		debug("Knob %d: Set value to %.2f (from %.2f). Current Value is now %.2f.", id, val, prevVal, value);
#endif
		return;
	}
	void step() override {
		// Re-transform TransformWidget if dirty
		if (dirty) {
			differentialAngle = rescale(value, minValue, maxValue, minAngle, maxAngle);
			currentAngle = zeroAnglePoint + differentialAngle;
			tw->identity();
			// Scale SVG to box
			tw->scale(box.size.div(sw->box.size));
			// Rotate SVG
			Vec center = sw->box.getCenter();
			tw->translate(center);
			tw->rotate(currentAngle);
			tw->translate(center.neg());
		}
		FramebufferWidget::step();
	}
}; // end TS_LightedKnob

//--------------------------------------------------------------
// TS_LightString - A light with a string (message/text).
//--------------------------------------------------------------
struct TS_LightString : ColorValueLight
{
	const char * lightString;
	float cornerRadius = 3.0;	
	std::shared_ptr<Font> font;
	int fontSize;
	TS_LightString()
	{
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 14;
		bgColor = nvgRGBAf(0.2, 0.2, 0.2, /*alpha */ 1);
		baseColor = COLOR_WHITE;		
	}
	void draw(NVGcontext *vg) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + 20.0;
		float radiusY = box.size.y / 2.0;
		float oradiusY = radiusY + 20.0;

		NVGcolor outerColor = color;
		
		// Solid
		nvgBeginPath(vg);
		// Border
		nvgStrokeWidth(vg, 1.0);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 0.5;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgFillColor(vg, color);
		nvgFill(vg);

		// Outer glow
		nvgBeginPath(vg);
		nvgRoundedRect(vg, /*x*/ radius - oradius, /*y*/ radiusY - oradiusY, /*w*/ 3*oradius, /*h*/ 2*oradiusY, cornerRadius);
		NVGpaint paint;
		NVGcolor icol = outerColor;// color;
		icol.a *= 0.5;
		NVGcolor ocol = outerColor;// color;
		ocol.a = 0.0;
		float feather = 3;
		// Feather defines how blurry the border of the rectangle is.
		paint = nvgBoxGradient(vg, /*x*/ 0, /*y*/ 0, /*w*/ box.size.x, /*h*/ oradiusY - 10, 
			/*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			/*inner color*/ icol, /*outer color */ ocol);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
		
		nvgBeginPath(vg);
		nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER);//Restore to default.
		NVGcolor textColor = baseColor;
		nvgFillColor(vg, textColor);
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, box.size.x / 2, box.size.y / 2, lightString, NULL);
		return;
	}	
}; // end TS_LightString

//--------------------------------------------------------------
// TS_LightSquare - Square light. 
//--------------------------------------------------------------
struct TS_LightSquare : ColorValueLight 
{
	// Radius on corners
	float cornerRadius = 5.0;
	TS_LightSquare()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.5);
		baseColor = COLOR_WHITE;
	}
	void draw(NVGcontext *vg) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius*1.1;

		NVGcolor backColor = bgColor;
		NVGcolor outerColor = color;
		// Solid
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		nvgFillColor(vg, backColor);
		nvgFill(vg);

		// Border
		nvgStrokeWidth(vg, 1.0);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 0.5;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgFillColor(vg, color);
		nvgFill(vg);

		// Outer glow
		nvgBeginPath(vg);
		nvgRoundedRect(vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2*oradius, /*h*/ 2*oradius, cornerRadius);
		NVGpaint paint;
		NVGcolor icol = outerColor;// color;
		icol.a *= 0.25;
		NVGcolor ocol = outerColor;// color;
		ocol.a = 0.0;
		float feather = 2;
		// Feather defines how blurry the border of the rectangle is. // Fixed 01/19/2018, made it too tiny before
		paint = nvgBoxGradient(vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2 * oradius, /*h*/ 2 * oradius,  //vg, /*x*/ -5, /*y*/ -5, /*w*/ 2*oradius + 10, /*h*/ 2*oradius + 10, 
			/*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			/*inner color*/ icol, /*outer color */ ocol);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
		return;
	}
}; // end TS_LightSquare

//--------------------------------------------------------------
// TS_LightRing - Light to be used around ports.
//--------------------------------------------------------------
struct TS_LightRing : ColorValueLight 
{
	// The inner radius 
	float innerRadius = 6.8;
	
	TS_LightRing()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.2);
		baseColor = COLOR_WHITE;
	}
	void draw(NVGcontext *vg) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + 10.0;
		

		// Solid
		nvgBeginPath(vg);
		nvgCircle(vg, radius, radius, radius);

		// Border
		nvgStrokeWidth(vg, radius - innerRadius);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 1.0;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		//nvgFillColor(vg, color);
		//nvgFill(vg);

		// Outer glow
		nvgBeginPath(vg);
		nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
		NVGpaint paint;
		NVGcolor icol = color;
		icol.a *= (module != NULL) ? module->lights[firstLightId].value : 0;
		//icol.a *= value;
		NVGcolor ocol = color;
		ocol.a = 0.0;
		paint = nvgRadialGradient(vg, radius, radius, innerRadius, oradius, icol, ocol);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
		return;
	}
};

//--------------------------------------------------------------
// TS_TinyBlackKnob - 20x20 RoundBlackKnob
//--------------------------------------------------------------
struct TS_TinyBlackKnob : RoundKnob {
	const int size = 20;
	 TS_TinyBlackKnob() {
		 box.size = Vec(size, size);
		 setSVG(SVG::load(assetPlugin(plugin, "res/ComponentLibrary/TS_RoundBlackKnob_20.svg")));
	 }
 };
//--------------------------------------------------------------
// TS_15_BlackKnob - 15x15 RoundBlackKnob
// This is a little too tiny
//--------------------------------------------------------------
struct TS_15_BlackKnob : RoundKnob {
	const int size = 15;
	 TS_15_BlackKnob() {
		 box.size = Vec(size, size);
		 setSVG(SVG::load(assetPlugin(plugin, "res/ComponentLibrary/TS_RoundBlackKnob_15.svg")));
	 }
 };
//--------------------------------------------------------------
// TS_20_BlackEncoder - 20x20 Encoder
// Pseudo continuous.... Still enforces the limits but allows the knob rotate 360.
//--------------------------------------------------------------
struct TS_20_BlackEncoder : RoundKnob {
	const int size = 20;
	int c = 0;
	float rotationRangeMin = -1.f;
	float rotationRangeMax = 1.f;
	float fineControlMult = 1. / 16.f;
	float coarseControlMult = 16.f;

	TS_20_BlackEncoder() {
		box.size = Vec(size, size);
		setSVG(SVG::load(assetPlugin(plugin, "res/ComponentLibrary/TS_RoundBlackEncoder_20.svg")));
		minAngle = 0;
		maxAngle = 2 * M_PI;
		return;
	}
	// Set the amount a full rotation should yield.
	void setRotationAmount(float amt) {
		rotationRangeMin = -0.5f*amt;
		rotationRangeMax = 0.5f*amt;
		return;
	}
	// Override to allow pseudo endless encoding (still bound by some real values).
	void onDragMove(EventDragMove &e) override {
		float range = rotationRangeMax - rotationRangeMin;
		float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed * range;
		// Drag slower if Mod is held
		if (windowIsModPressed())
			delta *= fineControlMult; // Finer control
		else if (windowIsShiftPressed())
			delta *= coarseControlMult; // Coarser control
		dragValue += delta;
		dragValue = clamp2(dragValue, minValue, maxValue);
		if (snap)
			setValue(roundf(dragValue));
		else
			setValue(dragValue);
		return;
	}
	void step() override {
		// Re-transform TransformWidget if dirty
		if (dirty) {
			// Allow rotations 360 degrees:
			float angle = rescale(value, rotationRangeMin, rotationRangeMax, minAngle, maxAngle);
			angle = fmodf(angle, 2 * M_PI);

			tw->identity();
			// Rotate SVG
			Vec center = sw->box.getCenter();
			tw->translate(center);
			tw->rotate(angle);
			tw->translate(center.neg());
		}
		FramebufferWidget::step();
		return;
	}
};

//--------------------------------------------------------------
// TS_Port - Smaller port with set light color and light disable
// (by just making the lights transparent... TODO: get rid of light completely.)
//--------------------------------------------------------------
struct TS_Port : SVGPort {
	NVGcolor negColor;
	NVGcolor posColor;
	
	TS_Port() : SVGPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/ComponentLibrary/TS_Port.svg"));
		background->wrap();
		box.size = background->box.size;
		if (plugLight)
		{
			negColor = plugLight->baseColors[1];
			posColor = plugLight->baseColors[0];
		}
	}
	void disableLights()
	{
		// Save our colors:
		if (plugLight)
		{
			negColor = plugLight->baseColors[1];
			posColor = plugLight->baseColors[0];		
			plugLight->baseColors[0] = nvgRGBAf(0,0,0,0);
			plugLight->baseColors[1] = nvgRGBAf(0,0,0,0);		
		}
	}
	void enableLights()
	{
		if (plugLight)
		{
			plugLight->baseColors[1] = negColor;
			plugLight->baseColors[0] = posColor;		
		}
	}
	void setLightColor(NVGcolor color)
	{		
		negColor = color;
		posColor = color;
		if (plugLight)
		{
			plugLight->baseColors[0] = color;
			plugLight->baseColors[1] = color;
		}
	}
	void setLightColor(NVGcolor negativeColor, NVGcolor positiveColor)
	{
		negColor = negativeColor;
		posColor = positiveColor;
		if (plugLight)
		{
			plugLight->baseColors[1] = negativeColor;
			plugLight->baseColors[2] = positiveColor;			
		}
	}	
};

//--------------------------------------------------------------
// TS_Panel - Panel with controllable borders on all sides.
//--------------------------------------------------------------
struct TS_Panel : Panel 
{
	NVGcolor originalBackgroundColor;
	NVGcolor borderColor = COLOR_BLACK;
	float borderWidth = 0;
	float borderTop = 0;
	float borderLeft = 0;
	float borderRight = 0;
	float borderBottom = 0;
	
	void setBorderWidth(float top, float right, float bottom, float left)
	{
		borderTop = top;
		borderLeft = left;
		borderRight = right;
		borderBottom = bottom;
		return;
	}
	//void invertBackgroundColor()
	//{
	//	backgroundColor = ColorInvertToNegative(originalBackgroundColor);
	//	return;
	//}	
	void draw(NVGcontext *vg) override
	{
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

		// Background color
		if (backgroundColor.a > 0) {
			nvgFillColor(vg, backgroundColor);
			nvgFill(vg);
		}

		// Background image
		if (backgroundImage) {
			int width, height;
			nvgImageSize(vg, backgroundImage->handle, &width, &height);
			NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0, width, height, 0.0, backgroundImage->handle, 1.0);
			nvgFillPaint(vg, paint);
			nvgFill(vg);
		}

		// Border		
		if (borderWidth > 0)
		{
			nvgBeginPath(vg);
			nvgRect(vg, borderWidth / 2.0, borderWidth / 2.0, box.size.x - borderWidth, box.size.y - borderWidth);
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderWidth);
			nvgStroke(vg);			
		}
		int x, y;
		
		if (borderTop > 0)
		{
			// Line at top
			nvgBeginPath(vg);
			x = 0;
			y = borderTop / 2.0;
			nvgMoveTo(vg, /*start x*/ x, /*start y*/ y); // Top Left
			x = box.size.x;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Top Right
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderTop);
			nvgStroke(vg);				
		}
		if (borderRight > 0)
		{
			nvgBeginPath(vg);
			x = box.size.x - borderRight / 2.0;
			y = 0;
			nvgMoveTo(vg, /*x*/ x, /*y*/ y); // Top Right					
			y = box.size.y;// - borderRight;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Bottom Right								
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderRight);
			nvgStroke(vg);							
		}
		if (borderBottom > 0)
		{
			nvgBeginPath(vg);
			x = box.size.x;// - borderBottom;
			y = box.size.y - borderBottom / 2.0;// - borderBottom;
			nvgMoveTo(vg, /*x*/ x, /*y*/ y); // Bottom Right					
			x = 0;// borderBottom / 2.0;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Bottom Left			
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderBottom);
			nvgStroke(vg);										
		}
		if (borderLeft > 0)
		{
			nvgBeginPath(vg);
			x = borderLeft / 2.0;
			y = box.size.y;// - borderLeft;
			nvgMoveTo(vg, /*x*/ x, /*y*/ y); // Bottom Left					
			y = 0;//borderLeft / 2.0;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Top Left						
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderLeft);
			nvgStroke(vg);													
		}
		Widget::draw(vg);
	} // end draw()
}; // end TS_Panel

//--------------------------------------------------------------
// TS_SVGPanel - SVG Panel without mandatory border on LHS
//--------------------------------------------------------------
struct TS_SVGPanel : SVGPanel
{
	NVGcolor borderColor = COLOR_BLACK;
	float borderTop = 0;
	float borderLeft = 0;
	float borderRight = 0;
	float borderBottom = 0;
	
	TS_SVGPanel() : SVGPanel()
	{
		return;
	}
	TS_SVGPanel(float borderTop, float borderRight, float borderBottom, float borderLeft) : TS_SVGPanel()
	{
		this->borderTop = borderTop;
		this->borderRight = borderRight;
		this->borderBottom = borderBottom;
		this->borderLeft = borderLeft;
		return;
	}	
	void setBackground(std::shared_ptr<SVG> svg)  {
		SVGWidget *sw = new SVGWidget();
		sw->setSVG(svg);
		addChild(sw);

		// Set size
		box.size = sw->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);

		if (borderTop + borderRight + borderLeft + borderBottom > 0)
		{
			TS_Panel* pb = new TS_Panel();
			pb->setBorderWidth(borderTop, borderRight, borderBottom, borderLeft);
			pb->borderColor = this->borderColor;
			pb->box.size = box.size;
			addChild(pb);
		}
		
		// PanelBorder *pb = new PanelBorder();
		// pb->box.size = box.size;
		// addChild(pb);
	} // end setBackground()
}; // end TS_SVGPanel

//--------------------------------------------------------------
// TS_ColorSlider - Horizontal color slider control 'knob'.
// Meant for picking colors via Hue, Saturation, Lightness.
//--------------------------------------------------------------
struct TS_ColorSlider : Knob {
	// If this control should be rendered
	bool visible = true;
	// Starting color.
	TSColorHSL startColorHSL;
	// Ending color.
	TSColorHSL endColorHSL;
	// HSL representation of the selected color.
	TSColorHSL selectedColorHSL;
	// NVGcolor of the selected color (RGBA)
	NVGcolor selectedColor;
	// Number of stops for the gradients (doing them manually since nanovg only lets you have 2 colors)
	int numStops = 24;
	float handleWidth = 15.0;
	float handleMargin = 3.0;
	TS_ColorSlider() : Knob()
	{
		minValue = 0.0;
		maxValue = 1.0;
		startColorHSL.h = 0.0;
		startColorHSL.s = 1.0;
		startColorHSL.lum = 0.5;
		endColorHSL.h = 1.0;
		endColorHSL.s = 1.0;
		endColorHSL.lum = 0.5;
		selectedColorHSL.h = 1.0;
		selectedColorHSL.s = 1.0;
		selectedColorHSL.lum = 0.5;
		return;
	}
	TS_ColorSlider(Vec size) : TS_ColorSlider()
	{
		box.size = size;
		return;
	}
	// Set the component value for start and end
	void setComponent(int index, float val)
	{
		startColorHSL.hsl[index] = val;
		endColorHSL.hsl[index] = val;
		return;
	}
	void onDragStart(EventDragStart &e) override {
		if (visible)
			Knob::onDragStart(e);
	}
	void onDragMove(EventDragMove &e) override {
		if (visible)
		{
			// Drag slower if Mod
			float delta = KNOB_SENSITIVITY * (maxValue - minValue) * e.mouseRel.x;
			if (windowIsModPressed())
				delta /= 16.0;
			dragValue += delta;
			if (snap)
				setValue(roundf(dragValue));
			else
				setValue(dragValue);

		}
		return;
	}
	void onDragEnd(EventDragEnd &e) override {
		if (visible)
			Knob::onDragEnd(e);
	}
	void onChange(EventChange &e) override {
		if (visible)
			Knob::onChange(e);
	}
	//void step() override {
	//	return;
	//}
	//void onChange(EventChange &e) override {
	//	//dirty = true;
	//	ParamWidget::onChange(e);
	//}
	void draw(NVGcontext *vg) override {
		if (!visible)
			return;

		// Draw the background:
		float x = 0;
		float y = 0;
		float dx = box.size.x / numStops;
		float deltaComponents[3];
		// Calculate the delta/interval for each component (HSL)
		for (int i = 0; i < 3; i++)
		{
			deltaComponents[i] = (endColorHSL.hsl[i] - startColorHSL.hsl[i]) / numStops;
		}
		float hue, sat, lht;
		hue = startColorHSL.h; sat = startColorHSL.s; lht = startColorHSL.lum;
		NVGcolor sColor;
		NVGcolor eColor = nvgHSLA(hue, sat, lht, 0xFF);
		y = box.size.y / 2.0;
		for (int i = 0; i < numStops; i++)
		{
			sColor = eColor;
			eColor = nvgHSLA(hue += deltaComponents[0], sat += deltaComponents[1], lht += deltaComponents[2], 0xFF);
			nvgBeginPath(vg);
			nvgRect(vg, x, 0, dx + 1, box.size.y);
			nvgStrokeWidth(vg, 0.0);
			NVGpaint paint = nvgLinearGradient(vg, x, y, x + dx + 1, y, sColor, eColor);
			nvgFillPaint(vg, paint);
			nvgFill(vg);
			x += dx;
		}
		for (int i = 0; i < 3; i++)
		{
			selectedColorHSL.hsl[i] = startColorHSL.hsl[i] + (endColorHSL.hsl[i] - startColorHSL.hsl[i]) * value;
		}
		selectedColor = nvgHSL(selectedColorHSL.hsl[0], selectedColorHSL.hsl[1], selectedColorHSL.hsl[2]);
		float handleHeight = box.size.y + 2 * handleMargin;
		float handleX = rescale(value, minValue, maxValue, 0, box.size.x) - handleWidth / 2.0;
		float handleY = -handleMargin;// rescale(value, minValue, maxValue, minHandlePos.y, maxHandlePos.y);
		// Draw handle
		nvgBeginPath(vg);
		nvgRoundedRect(vg, handleX, handleY, handleWidth, handleHeight, 5);
		nvgFillColor(vg, selectedColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.0);
		NVGcolor strokeColor = ((value < 0.5) ? COLOR_WHITE : COLOR_BLACK);
		nvgStrokeColor(vg, strokeColor);
		nvgStroke(vg);

		/*nvgFontSize(vg, 9.0);
		nvgFillColor(vg, COLOR_WHITE);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		char buffer[20];
		sprintf(buffer, "V: %.2f, W: %.2f", value, box.size.x);
		nvgText(vg, 0, 0, buffer, NULL);*/
	}
}; // end TS_ColorSlider

//:::-:::-:::-:::- Helpers -:::-:::-:::-:::
template <class TModuleLightWidget>
ColorValueLight * TS_createColorValueLight(Vec pos,  Module *module, int lightId, Vec size, NVGcolor lightColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;	
	//light->value = value;
	light->box.size = size;
	light->setColor(lightColor);
	//light->baseColor = lightColor;
	return light;
}
template <class TModuleLightWidget>
ColorValueLight * TS_createColorValueLight(Vec pos, Module *module, int lightId, Vec size, NVGcolor lightColor, NVGcolor backColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;	
	light->box.size = size;
	light->setColor(lightColor);	
	light->bgColor = backColor;
	return light;
}

template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	port->disableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, NVGcolor negColor, NVGcolor posColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	port->setLightColor(negColor, posColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, bool disableLight) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, bool disableLight, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}


template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	port->disableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, NVGcolor negColor, NVGcolor posColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	port->setLightColor(negColor, posColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, bool disableLight) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, bool disableLight, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}



#endif // end if not defined
