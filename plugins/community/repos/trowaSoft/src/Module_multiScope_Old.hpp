#ifndef MODULE_MULTISCOPE_OLD_HPP
#define MODULE_MULTISCOPE_OLD_HPP

#include "Features.hpp"

#if !USE_NEW_SCOPE


#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "dsp/digital.hpp"

#define TROWA_SCOPE_NUM_WAVEFORMS	3


// Laying out controls
#define TROWA_SCOPE_CONTROL_START_X			47  // 47
#define TROWA_SCOPE_CONTROL_START_Y			40  // 94
#define TROWA_SCOPE_CONTROL_DX					35 // 35
#define TROWA_SCOPE_CONTROL_DY					30 // 26
#define TROWA_SCOPE_CONTROL_SHAPE_SPACING		16 // 8


// Labels for each waveform / shape
#define TROWA_SCOPE_SHAPE_FORMAT_STRING		"Shp %d"

#define BUFFER_SIZE 					512
#define TROWA_SCOPE_USE_COLOR_LIGHTS	  0

// X and Y Knobs:
#define TROWA_SCOPE_POS_KNOB_MIN	-30.0
#define TROWA_SCOPE_POS_KNOB_MAX	 30.0
#define TROWA_SCOPE_POS_X_KNOB_DEF	  0.0 // -10
#define TROWA_SCOPE_POS_Y_KNOB_DEF	  0.0 // 10
#define TROWA_SCOPE_SCALE_KNOB_MIN	-10.0
#define TROWA_SCOPE_SCALE_KNOB_MAX	 10.0

// Time Knob:
#define TROWA_SCOPE_TIME_KNOB_MIN	 -6.0
#define TROWA_SCOPE_TIME_KNOB_MAX	-16.0
#define TROWA_SCOPE_TIME_KNOB_DEF	-14.0	// Default value
// Hue Knob:
#define TROWA_SCOPE_HUE_KNOB_MIN	-10
#define TROWA_SCOPE_HUE_KNOB_MAX	 10
#define TROWA_SCOPE_HUE_INPUT_MIN_V	  0
#define TROWA_SCOPE_HUE_INPUT_MAX_V	  5
#define TROWA_SCOPE_COLOR_KNOB_Y_OFFSET	6

// Opacity:
#define TROWA_SCOPE_MIN_OPACITY		0.0
#define TROWA_SCOPE_MAX_OPACITY		1.0
#define TROWA_SCOPE_OPACITY_INPUT_MIN	 0.0 // Min Voltage in
#define TROWA_SCOPE_OPACITY_INPUT_MAX	 5.0 // Max Voltage in

// Rotation Knob:
#define TROWA_SCOPE_ROT_KNOB_MIN	-10
#define TROWA_SCOPE_ROT_KNOB_MAX	 10
#define TROWA_SCOPE_ROUND_FORMAT	"%.2f"	// Output string format
#define TROWA_SCOPE_ROUND_VALUE		100		// Rounding
#define TROWA_SCOPE_ABS_ROT_ON_COLOR			COLOR_TS_BLUE	// Color to signal Absolution Rotation mode is on.
#define TROWA_SCOPE_LINK_XY_SCALE_ON_COLOR		COLOR_MAGENTA
#define TROWA_SCOPE_INFO_DISPLAY_ON_COLOR		COLOR_TS_ORANGE
#define TROWA_SCOPE_LISSAJOUS_ON_COLOR			COLOR_YELLOW

// multiScope model.
extern Model *modelMultiScope;


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSWaveform
// Store data about a waveform.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSWaveform
{
	float bufferX[BUFFER_SIZE] = {};
	float bufferY[BUFFER_SIZE] = {};
	bool bufferPenOn[BUFFER_SIZE] = {};

	int bufferIndex;
	float frameIndex;

	bool lissajous = true;
	SchmittTrigger lissajousTrigger;

	// Link X and Y scale ::::::::::::::::::::::::::::::::::::::::::::::
	bool linkXYScales; // Just forces scaleX = scaleY (only 1:1 aspect ratio).
	SchmittTrigger linkXYScalesTrigger;
	float lastXYScaleValue; // Last value when they are synched

	// Rotation ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	SchmittTrigger rotModeTrigger;
	// True for absolute angular position, false if constant angular change
	bool rotMode;
	float rotKnobValue; // Value from rotation knob
	float rotAbsValue;  // Translated to ABS position [radians]
	float rotDiffValue; // Translated to differential position [radians] (rate)

	// Colors ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	NVGcolor waveColor;
	float waveHue;
#if TROWA_SCOPE_USE_COLOR_LIGHTS
	// References to our lights (typed)
	ColorValueLight* waveLight;
#endif
	bool colorChanged;
	float waveOpacity;

	TSWaveform()
	{
		bufferIndex = 0;
		frameIndex = 0;
		memset(bufferPenOn, true, BUFFER_SIZE);
		colorChanged = true;
		rotMode = false;
		rotKnobValue = 0;
		rotAbsValue = 0;
		rotDiffValue = 0;
		linkXYScales = false;
		waveOpacity = TROWA_SCOPE_MAX_OPACITY;
#if TROWA_SCOPE_USE_COLOR_LIGHTS
		waveLight = NULL;
#endif
		return;
	}

	void setHue(float hue)
	{
		waveHue = hue;
		waveColor = HueToColor(waveHue);
	}

	void setHueFromKnob(float hueKnobValue)
	{
		setHue(rescale(hueKnobValue, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, 0.0, 1.0));
		return;
	}
};

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScope
// Scope module that draws multiple waveforms.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct multiScope : Module {
	// Should probably make this linearly growing in case of course, we add more freaken waveforms...
	// TODO: Refactor to make variable num waveforms.
	enum ParamIds {
		COLOR_PARAM,
		EXTERNAL_PARAM = COLOR_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		ROTATION_PARAM = EXTERNAL_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		ROTATION_MODE_PARAM = ROTATION_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		TIME_PARAM = ROTATION_MODE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		TRIG_PARAM = TIME_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		X_POS_PARAM = TRIG_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		X_SCALE_PARAM = X_POS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		Y_POS_PARAM = X_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		Y_SCALE_PARAM = Y_POS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		LINK_XY_SCALE_PARAM = Y_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,	   // Force Scale X = Scale Y.
		OPACITY_PARAM = LINK_XY_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,	   // Alpha channel
		LISSAJOUS_PARAM = OPACITY_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,		   // For now always true
		INFO_DISPLAY_TOGGLE_PARAM = LISSAJOUS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		NUM_PARAMS = INFO_DISPLAY_TOGGLE_PARAM + 1
	};
	enum InputIds {
		COLOR_INPUT,
		ROTATION_INPUT = COLOR_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		TIME_INPUT = ROTATION_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		X_INPUT = TIME_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		Y_INPUT = X_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		OPACITY_INPUT = Y_INPUT + TROWA_SCOPE_NUM_WAVEFORMS, // Opacity/Alpha channel
		PEN_ON_INPUT = OPACITY_INPUT + TROWA_SCOPE_NUM_WAVEFORMS, // Turn on/off drawing lines in between points.
		NUM_INPUTS = PEN_ON_INPUT + TROWA_SCOPE_NUM_WAVEFORMS
	};

	enum LightIds {
		COLOR_LED,
		ROT_LED = COLOR_LED+TROWA_SCOPE_NUM_WAVEFORMS,
		LINK_XY_SCALE_LED = ROT_LED + TROWA_SCOPE_NUM_WAVEFORMS,
		LISSAJOUS_LED = LINK_XY_SCALE_LED + TROWA_SCOPE_NUM_WAVEFORMS,			// For now always true
		INFO_DISPLAY_TOGGLE_LED = LISSAJOUS_LED + TROWA_SCOPE_NUM_WAVEFORMS,
		NUM_LIGHTS = INFO_DISPLAY_TOGGLE_LED + 1
	};

	enum OutputIds {
		NUM_OUTPUTS
	};

	bool initialized = false;
	bool firstLoad = true;

	SchmittTrigger infoDisplayOnTrigger;

	// Information about what we are plotting. In future may become dynamically allocated.
	TSWaveform* waveForms[TROWA_SCOPE_NUM_WAVEFORMS];

	multiScope();
	~multiScope();
	void step() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// toJson(void)
	// Save to json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_t *huesJ = json_array();
		json_t *linkXYScalesJ = json_array();
		json_t* lissajousJ = json_array();
		json_t *rotModeJ = json_array();
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			json_t* itemJ = json_real(waveForms[wIx]->waveHue);
			json_array_append_new(huesJ, itemJ);

			itemJ = json_integer((int)waveForms[wIx]->linkXYScales);
			json_array_append_new(linkXYScalesJ, itemJ);

			itemJ = json_integer((int)waveForms[wIx]->lissajous);
			json_array_append_new(lissajousJ, itemJ);

			itemJ = json_integer(waveForms[wIx]->rotMode);
			json_array_append_new(rotModeJ, itemJ);
		}
		json_object_set_new(rootJ, "hues", huesJ);
		json_object_set_new(rootJ, "linkXYScales", linkXYScalesJ);
		json_object_set_new(rootJ, "lissajous", lissajousJ);
		json_object_set_new(rootJ, "rotMode", rotModeJ);
		return rootJ;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// fromJson(void)
	// Load settings.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void fromJson(json_t *rootJ) override {
		json_t *huesJ = json_object_get(rootJ, "hues");
		json_t *rotModeJ = json_object_get(rootJ, "rotMode");
		json_t *linkXYScalesJ = json_object_get(rootJ, "linkXYScales");
		json_t* lissajousJ = json_object_get(rootJ, "lissajous");
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			json_t* itemJ = json_array_get(huesJ, wIx);
			if (itemJ)
				waveForms[wIx]->setHue((float)json_real_value(itemJ));
			itemJ = NULL;
			itemJ = json_array_get(rotModeJ, wIx);
			if (itemJ)
				waveForms[wIx]->rotMode = json_integer_value(itemJ);
			itemJ = NULL;
			itemJ = json_array_get(linkXYScalesJ, wIx);
			if (itemJ)
				waveForms[wIx]->linkXYScales = (bool)json_integer_value(itemJ);
			itemJ = NULL;
			itemJ = json_array_get(lissajousJ, wIx);
			if (itemJ)
				waveForms[wIx]->lissajous = (bool)json_integer_value(itemJ);
			else
				waveForms[wIx]->lissajous = true;

			itemJ = NULL;
		}
		firstLoad = true;
		return;
	}

	void reset()  override {
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			waveForms[wIx]->lissajous = true;
		}
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSScopeDisplay
// A top digital display for trowaSoft scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSScopeDisplay : TransparentWidget {
	multiScope *module;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE];
	bool visible = true;
	int originalWidth = 240;

	TSScopeDisplay() {
		visible = true;
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
	}

	void draw(NVGcontext *vg) override {
		if (!visible)
			return; // Don't draw anything if we are not visible.

		nvgSave(vg);
		Rect b = Rect(Vec(0, 0), box.size);
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgTextLetterSpacing(vg, 1);

		// Background Colors:
		NVGcolor backgroundColor = nvgRGBA(0x20, 0x20, 0x20, 0x80);
		NVGcolor borderColor = nvgRGBA(0x10, 0x10, 0x10, 0x80);

		// Screen:
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);

		////////////// Labels /////////////////
		const int yStart = 10;
		int xStart = 10;
		const int dxRotation = 6; // 10
		int y = yStart;
		int x = xStart;
		int dx = 59; //41 // 37; // 35
		int dy = 16; //14

		//nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		nvgFillColor(vg, textColor);
		// Row Labels 
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		y = yStart + 5;
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			NVGcolor currColor = module->waveForms[wIx]->waveColor;
			nvgFillColor(vg, currColor);
			sprintf(messageStr, "S%d", wIx + 1);
			nvgText(vg, 5, y, messageStr, NULL);
			y += dy;
		}
		
		// Column Labels:		
		nvgTextAlign(vg, NVG_ALIGN_RIGHT);
		nvgFillColor(vg, textColor);

		// Column 1 (Labels)
		y = yStart;
		xStart = 35; // 40
		x = xStart + dx / 2.0;
		nvgText(vg, x, y, "X Offset", NULL);

		x += dx;
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y, "X Scale", NULL);

		x += dx;
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y, "Y Offset", NULL);

		x += dx;
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y, "Y Scale", NULL);

		// Rotation (wider)
		x += dx + dxRotation;
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y, "Rotate", NULL);

		// Values:
		y = yStart + 5;
		nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
		NVGcolor absRotColor = TROWA_SCOPE_ABS_ROT_ON_COLOR;
		absRotColor.a = 0.50;
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			// NVGcolor currColor = module->waveForms[wIx]->waveColor;
			nvgFillColor(vg, textColor);

			// X Offset
			x = xStart + dx / 2.0;
			sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, module->params[multiScope::X_POS_PARAM + wIx].value);
			nvgText(vg, x, y, messageStr, NULL);

			// X Gain
			x += dx;
			sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, module->params[multiScope::X_SCALE_PARAM + wIx].value);
			nvgText(vg, x, y, messageStr, NULL);

			// Y Offset
			x += dx;
			sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, module->params[multiScope::Y_POS_PARAM + wIx].value);
			nvgText(vg, x, y, messageStr, NULL);

			// Y Gain
			x += dx;
			sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, module->params[multiScope::Y_SCALE_PARAM + wIx].value);
			nvgText(vg, x, y, messageStr, NULL);

			// Rotation
			x += dx + dxRotation;
			float v = 0.0;
			if (module->waveForms[wIx]->rotMode)
			{
				// Absolute
				v = module->waveForms[wIx]->rotAbsValue;
				// Background:
				nvgBeginPath(vg);
				nvgRoundedRect(vg, x - dx - 3, y - 2, dx + 5, fontSize + 2, 2);
				nvgFillColor(vg, absRotColor);
				nvgFill(vg);

				// Text will be black for this
				nvgFillColor(vg, COLOR_BLACK);
				sprintf(messageStr, "%.1f", v * 180.0 / NVG_PI);
			}
			else
			{
				// Differential
				v = module->waveForms[wIx]->rotDiffValue;
				sprintf(messageStr, "%+.1f", v * 180.0 / NVG_PI);
			}
			nvgText(vg, x, y, messageStr, NULL);

			// Advance y to next 
			y += dy;
		} // end loop through wave forms

		nvgResetScissor(vg);
		nvgRestore(vg);
		return;
	}
}; // end TSScopeDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScopeDisplay
// Draws a waveform.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct multiScopeDisplay : TransparentWidget {
	multiScope *module;
	int frame = 0;
	float rot = 0;
	std::shared_ptr<Font> font;
	int wIx = 0; // Waveform index
	
	multiScopeDisplay() {
		//spoutInitSpout();
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawWaveform()
	// @vg : (IN) NVGcontext
	// @valX: (IN) Pointer to x values.
	// @valY: (IN) Pointer to y values.
	// @rotRate: (IN) Rotation rate in radians
	// @flipX: (IN) Flip along x (at x=0)
	// @flipY: (IN) Flip along y
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawWaveform(NVGcontext *vg, float *valX, float *valY, bool* penOn, float rotRate, bool flipX, bool flipY) {
		if (!valX)
			return;
		nvgSave(vg);
		Rect b = Rect(Vec(0, 0), box.size);
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgTranslate(vg, box.size.x / 2.0, box.size.y / 2.0);
		nvgRotate(vg, rot += rotRate);
		if (flipX || flipY)
		{			
			// Sets the transform to scale matrix.
			// void nvgTransformScale(float* dst, float sx, float sy);
			nvgScale(vg, ((flipX) ? -1 : 1), (flipY) ? -1 : 1); // flip
		}

		// Draw maximum display left to right
		nvgBeginPath(vg);
		nvgLineCap(vg, NVG_ROUND);
		nvgMiterLimit(vg, 2.0);
		nvgStrokeWidth(vg, 3.0);
		bool lastPointStarted = false;
		float xOffset = -box.size.x / 2.0; // Fill our screen
		float yOffset = -box.size.y / 2.0; 
		for (int i = 0; i < BUFFER_SIZE; i++) {
			if (penOn[i])
			{
				float x, y;
				if (valY) {
					x = valX[i] / 2.0 + 0.5;
					y = valY[i] / 2.0 + 0.5;
				}
				else {
					x = (float)i / (BUFFER_SIZE - 1);
					y = valX[i] / 2.0 + 0.5;
				}

				Vec p;
				p.x = b.pos.x + xOffset + b.size.x * x;
				p.y = b.pos.y + yOffset + b.size.y * (1.0 - y);

				if (!lastPointStarted)
				{
					nvgMoveTo(vg, p.x, p.y);
				}
				else
				{
					nvgLineTo(vg, p.x, p.y);
				}
				lastPointStarted = true;
				//if (firstIx < 0 || (i > 0 && !penOn[i - 1]))
				//	firstIx = i;
				//if (i == firstIx)
				//	nvgMoveTo(vg, p.x, p.y);
				//else
				//	nvgLineTo(vg, p.x, p.y);
			}
			else
			{
				lastPointStarted = false;
			}
		} // end loop through buffer
		nvgLineCap(vg, NVG_ROUND);
		nvgMiterLimit(vg, 2.0);
		nvgStrokeWidth(vg, 3.0);
		//nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgStroke(vg);
		nvgResetScissor(vg);
		nvgRestore(vg);
	} // end drawWaveform()

	void draw(NVGcontext *vg) override {
		if (!module->initialized)
			return;
		float gainX = ((int)(module->params[multiScope::X_SCALE_PARAM + wIx].value * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);
		float gainY = ((int)(module->params[multiScope::Y_SCALE_PARAM + wIx].value * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);
		float offsetX = ((int)(module->params[multiScope::X_POS_PARAM + wIx].value * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);
		float offsetY = ((int)(module->params[multiScope::Y_POS_PARAM + wIx].value * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);

		TSWaveform* waveForm = module->waveForms[wIx];
		float valuesX[BUFFER_SIZE];
		float valuesY[BUFFER_SIZE];
		bool penOn[BUFFER_SIZE];
		float multX = gainX / 10.0;
		float multY = gainY / 10.0;
		for (int i = 0; i < BUFFER_SIZE; i++) {
			int j = i;
			// Lock display to buffer if buffer update deltaTime <= 2^-11
			if (waveForm->lissajous)
				j = (i + waveForm->bufferIndex) % BUFFER_SIZE;
			valuesX[i] = (waveForm->bufferX[j] + offsetX) * multX;
			valuesY[i] = (waveForm->bufferY[j] + offsetY) * multY;
			penOn[i] = waveForm->bufferPenOn[j];
		}

		// Draw waveforms
		NVGcolor waveColor = waveForm->waveColor;
		float rotRate = 0;
		waveColor.a = waveForm->waveOpacity;
		nvgStrokeColor(vg, waveColor); // Color has already been calculated by main module
		if (waveForm->rotMode)
		{
			// Absolute position:
			rot = waveForm->rotAbsValue;
		}
		else
		{
			// Differential rotation
			rotRate = waveForm->rotDiffValue;
		}
		if (waveForm->lissajous) {
			// X x Y
			if (module->inputs[multiScope::X_INPUT + wIx].active || module->inputs[multiScope::Y_INPUT + wIx].active) {
				module->lights[multiScope::COLOR_LED + wIx].value = 1.0; // Actively drawing
				drawWaveform(vg, valuesX, valuesY, penOn, rotRate, false, false);// module->mirrorX[wIx], module->mirrorY[wIx]);
			}
			else
			{
				module->lights[multiScope::COLOR_LED + wIx].value = 0.7; // Not really drawing anything, dim the light
			}
		}
		else {
			// Y
			if (module->inputs[multiScope::Y_INPUT + wIx].active) {
				drawWaveform(vg, valuesY, NULL, penOn, rotRate, false, false);
			}
			// X
			if (module->inputs[multiScope::X_INPUT + wIx].active) {
				drawWaveform(vg, valuesX, NULL, penOn, rotRate, false, false);
			}
		}
		return;
	} // end draw()
}; // end multiScopeDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSScopeLabelArea
// Draw labels on our scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSScopeLabelArea : TransparentWidget {
	multiScope *module;
	std::shared_ptr<Font> font;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE];

	TSScopeLabelArea() {
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 10;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawColorGrandientArc()
	// @cx : (IN) x-coordinate for center of arc.
	// @cy : (IN) y-coordinate for center of arc.
	// @radius: (IN) radius of arc.
	// @thickness: (IN) Border thickness of arc.
	// @start_radians : (IN) Arc start angle [radians].
	// @end_radians : (IN) Arc end angle [radians].
	// @startHue: (IN) Starting hue.
	// @endHue: (IN) Ending hue.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawColorGradientArc(NVGcontext* vg, float cx, float cy, float radius, float thickness, float start_radians, float end_radians, float startHue, float endHue)
	{
		NVGcolor startColor = HueToColorGradient(startHue);
		NVGcolor endColor = HueToColorGradient(endHue);
		nvgBeginPath(vg);
		nvgArc(vg, /*cx*/ cx, /*cy*/ cy, radius - thickness/2.0,
			/*a0*/ start_radians, /*a1*/ end_radians, /*dir*/ NVG_CW);

		// Creates and returns a linear gradient. Parameters (sx,sy)-(ex,ey) specify the start and end coordinates
		// of the linear gradient, icol specifies the start color and ocol the end color.
		// The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
		//float circum = radius * (end_radians - start_radians);
		float sx = cx + radius * cos(start_radians);
		float sy = cy + radius * sin(start_radians);
		float ex = cx + radius * cos(end_radians) + 1;
		float ey = cy + radius * sin(end_radians) + 1;
		NVGpaint paint = nvgLinearGradient(vg, sx, sy, ex, ey, startColor, endColor);
		nvgStrokeWidth(vg, thickness);
		nvgStrokePaint(vg, paint);
		nvgStroke(vg);
		return;
	} // end drawColorGradientArc()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawColorGrandientArc()
	// @cx : (IN) x-coordinate for center of arc.
	// @cy : (IN) y-coordinate for center of arc.
	// @radius: (IN) radius of arc.
	// @thickness: (IN) Border thickness of arc.
	// @start_radians : (IN) Arc start angle [radians].
	// @end_radians : (IN) Arc end angle [radians].
	// @startHue: (IN) Starting hue.
	// @endHue: (IN) Ending hue.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawColorGradientArc(NVGcontext* vg, float cx, float cy, float radius, float thickness)
	{
		const float startAngle = 0.67*NVG_PI;
		const float endAngle = 2.33*NVG_PI;
		const int numStops = 20;
		float dH = 1.0 / numStops;
		float dA = (endAngle - startAngle) / numStops;
		float hue = 0;
		float start_radians = startAngle;
		
		while (hue < 1.0)
		{
			drawColorGradientArc(vg, cx, cy, radius, thickness, 
				start_radians, start_radians + dA*1.2, 
				hue, hue + dH);
			start_radians += dA;
			hue += dH;
		}
		return;
	} // end drawColorGradientArc()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(NVGcontext *vg) override {

		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(vg, textColor);
		nvgFontSize(vg, fontSize);

		const int xStart = 17; // 23
		const int yStart = 6; // 10
		int knobOffset = 5;
		int dx = TROWA_SCOPE_CONTROL_DX; // 35
		int dy = TROWA_SCOPE_CONTROL_DY; // 26
		int shapeSpacingY = TROWA_SCOPE_CONTROL_SHAPE_SPACING; // 8 An extra amount between shapes
		int x, y;
		int shapeDy = dy * 3 + knobOffset + shapeSpacingY + 3;

		x = xStart;
		y = yStart;
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			nvgFontSize(vg, fontSize);
			x = xStart;
			int waveY = y;
			// Shape Label:		
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			// BG Box
			nvgBeginPath(vg);
			nvgRect(vg, x - 3, y - 2, TROWA_SCOPE_CONTROL_START_X - x + 3, fontSize + 3);
			nvgFillColor(vg, textColor);
			nvgFill(vg);
			nvgFillColor(vg, COLOR_BLACK);
			sprintf(messageStr, TROWA_SCOPE_SHAPE_FORMAT_STRING, (wIx + 1));
			nvgText(vg, x, y, messageStr, NULL);
			nvgFillColor(vg, textColor);

			// Line down lhs:
			nvgBeginPath(vg);
			nvgRect(vg, x - 3, y - 2, TROWA_SCOPE_CONTROL_START_X - x + 3, fontSize + 3);
			nvgMoveTo(vg, /*x*/ x - 3, /*y*/ y - 2);
			nvgLineTo(vg, /*x*/ x - 3, /*y*/ y + shapeDy - 10); // Go Left (to right of the text "Edit")		
			nvgStrokeWidth(vg, 1.0);
			nvgStrokeColor(vg, textColor);
			nvgStroke(vg);

			// Row Labels:
			x = TROWA_SCOPE_CONTROL_START_X - 5;
			y += fontSize + 8;
			nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
			nvgText(vg, x, y, "IN", NULL);
			y += dy;
			nvgText(vg, x, y, "OFF", NULL);
			y += dy;
			nvgText(vg, x, y, "SCL", NULL);

			const char* colLabels[] = { "X", "Y", "C", "A", "R", "T" };
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
			x = TROWA_SCOPE_CONTROL_START_X + 15;
			y = waveY;
			for (int i = 0; i < 6; i++)
			{
				nvgText(vg, x, y, colLabels[i], NULL);
				x += dx;
			}

			int x1 = TROWA_SCOPE_CONTROL_START_X + 15 + 0.5*dx;
			int y1 = waveY + dy * 2.3 + 1;
			nvgFontSize(vg, fontSize*0.8);
			nvgText(vg, x1, y1, "LNK", NULL);

			x1 += 2*dx;
			int y2 = y1 + 6;
			nvgText(vg, x1 - 8, y2, "BLANK", NULL);
			y2 += fontSize * 0.8 + 0.5;
			nvgFontSize(vg, fontSize*1.05);
			nvgText(vg, x1 - 8, y2, "<= 0", NULL);

			nvgFontSize(vg, fontSize*0.8);
			x1 += 1.5*dx;
			nvgText(vg, x1, y1, "ABS", NULL);

			x1 += dx;
			nvgText(vg, x1, y1, "X*Y", NULL);

			// Color Knob gradient:
			//x += dx * 2;
			//y += 30;
			drawColorGradientArc(vg, 
				/*cx*/ TROWA_SCOPE_CONTROL_START_X + 15 + 2*dx,
				/*cy*/ y + dy + 23 + TROWA_SCOPE_COLOR_KNOB_Y_OFFSET, 
				/*radius*/ 14, 
				/*thickness*/ 4.0);

			y += shapeDy - 3;
		} // end loop through shapes/waveforms
		return;
	} // end draw()
}; // end TSScopeLabelArea

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSScopeSideBarArea
// Draw labels on the RHS bar of our scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSScopeSideBarLabelArea : TransparentWidget {
	//multiScope *module;
	std::shared_ptr<Font> font;
	int fontSize;
	//char messageStr[TROWA_DISP_MSG_SIZE];
	TSScopeSideBarLabelArea() {
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 10;
		//for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
		//	messageStr[i] = '\0';
	}
	TSScopeSideBarLabelArea(Vec bsize) : TSScopeSideBarLabelArea() {
		this->box.size = bsize;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(NVGcontext *vg) override {
		nvgSave(vg);
		//nvgTranslate(vg, -box.size.y / 2.0, -box.size.x / 2.0);
		nvgRotate(vg, NVG_PI*0.5);

		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(vg, textColor);
		nvgFontSize(vg, fontSize);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

		float x, y;
		x = 34;// box.size.x / 2.0;
		y = -box.size.x / 2.0; // 0;// 32;
		nvgText(vg, x, y, "INFO", NULL); // Info display btn toggle

		nvgRestore(vg);
		return;
	} // end draw()
}; // end TSScopeSideBarLabelArea




#endif // end if use old scope
#endif // end if not defined