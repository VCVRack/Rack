#ifndef TSSCOPEMODULEBASE_HPP
#define TSSCOPEMODULEBASE_HPP
#include <string.h>
#include <stdint.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "util/math.hpp"
#include "dsp/digital.hpp"

#define BUFFER_SIZE 					512
#define TROWA_SCOPE_USE_COLOR_LIGHTS	  0

// X and Y Knobs:
#define TROWA_SCOPE_POS_KNOB_MIN	-30.0	// Min pos value
#define TROWA_SCOPE_POS_KNOB_MAX	 30.0	// Max Pos value
#define TROWA_SCOPE_POS_X_KNOB_DEF	  0.0 
#define TROWA_SCOPE_POS_Y_KNOB_DEF	  0.0 
#define TROWA_SCOPE_SCALE_KNOB_MIN	-10.0	// Min Scale value
#define TROWA_SCOPE_SCALE_KNOB_MAX	 10.0	// Max Scale value
#define TROWA_SCOPE_SCALE_POS_INPUT_MIN_V	-10.0	// Min input voltage for Scale & Offset
#define TROWA_SCOPE_SCALE_POS_INPUT_MAX_V	 10.0	// Max input voltage for Scale & Offset


// Time Knob:
#define TROWA_SCOPE_TIME_KNOB_MIN	 -6.0
#define TROWA_SCOPE_TIME_KNOB_MAX	-16.0
#define TROWA_SCOPE_TIME_KNOB_DEF	-14.0	// Default value

// Effect Knob
// Number of effects (max index is this - 1) for our scope
#define TROWA_SCOPE_NUM_EFFECTS			TROWA_NUM_GLOBAL_EFFECTS // 4
// Effect Knob min value (0)
#define TROWA_SCOPE_EFFECT_KNOB_MIN		0
// Effect Knob max value
#define TROWA_SCOPE_EFFECT_KNOB_MAX		(TROWA_SCOPE_NUM_EFFECTS-1) 
#define TROWA_SCOPE_EFFECT_KNOB_DEF		TROWA_SCOPE_EFFECT_KNOB_MIN	// Default value

//-- From original multiScope ---
// Hue Knob:
#define TROWA_SCOPE_HUE_KNOB_MIN	-10	// Not used anymore
#define TROWA_SCOPE_HUE_KNOB_MAX	 10 // Not used anymore
#define TROWA_SCOPE_HUE_INPUT_MIN_V	  0 // Not used anymore
#define TROWA_SCOPE_HUE_INPUT_MAX_V	  5 // Not used anymore
#define TROWA_SCOPE_COLOR_KNOB_Y_OFFSET	0 // 6
// Opacity:
#define TROWA_SCOPE_MIN_OPACITY			0.0 // Not used anymore
#define TROWA_SCOPE_MAX_OPACITY			1.0 // Not used anymore
#define TROWA_SCOPE_OPACITY_INPUT_MIN	 0.0 // (Not used anymore) Min Voltage in
#define TROWA_SCOPE_OPACITY_INPUT_MAX	 5.0 // (Not used anymore) Max Voltage in

// Color Knobs and Inputs (HSLA) - All of these components will use the same values. Knobs should use the base value directly.
#define TROWA_SCOPE_HSLA_MIN				0.0	// Min value for Hue, Sat, Lum, or Alpha/Opacity
#define TROWA_SCOPE_HSLA_MAX				1.0	// Max value for Hue, Sat, Lum, or Alpha/Opacity
#define TROWA_SCOPE_HSLA_INPUT_MIN_V		 -5 // Min Voltage in for Hue, Sat, Lum, or Alpha/Opacity
#define TROWA_SCOPE_HSLA_INPUT_MAX_V		  5 // Max Voltage in for Hue, Sat, Lum, or Alpha/Opacity

// Rotation Knob:
#define TROWA_SCOPE_ROT_KNOB_MIN	-10
#define TROWA_SCOPE_ROT_KNOB_MAX	 10
#define TROWA_SCOPE_ROUND_FORMAT	"%.2f"	// Output string format
#define TROWA_SCOPE_ROUND_VALUE		100		// Rounding
#define TROWA_SCOPE_ABS_ROT_ON_COLOR			COLOR_TS_BLUE	// Color to signal Absolute Rotation mode is on.

// Colors:
#define TROWA_SCOPE_LINK_XY_SCALE_ON_COLOR		COLOR_MAGENTA
#define TROWA_SCOPE_INFO_DISPLAY_ON_COLOR		COLOR_TS_ORANGE
#define TROWA_SCOPE_LISSAJOUS_ON_COLOR			COLOR_YELLOW
#define TROWA_SCOPE_FILL_ON_COLOR				nvgRGB(0xDD,0xDD,0xDD) // COLOR_WHITE -- Very bright

// Line Thickness
#define TROWA_SCOPE_THICKNESS_MIN		  1.0
#define TROWA_SCOPE_THICKNESS_MAX		 10.0
#define TROWA_SCOPE_THICKNESS_DEF		  3.0
#define TROWA_SCOPE_THICKNESS_INPUT_MIN	 -5.0
#define TROWA_SCOPE_THICKNESS_INPUT_MAX	  5.0

#define POINT_POS_INSIDE	0 // Point is within bounds
#define POINT_POS_LEFT		0b0001 // Point is below min X
#define POINT_POS_RIGHT		0b0010 // Point is above max X
#define POINT_POS_BOTTOM	0b0100 // Point is below min Y
#define POINT_POS_TOP		0b1000 // Point is above max Y

#define POINT_IS_IN_BOUNDS(a)		(!a)	// The given point is inside our bounds.
#define LINE_OUT_OF_BOUNDS(a,b)		(a&b)	// The line is completely outside of bounds (and would not cross our region).
#define LINE_IS_IN_BOUNDS(a,b)		(!(a|b)) // The line is inside our bounds.

// Gets where the point is.
uint8_t GetPointLocationCode(Vec pt, float minX, float maxX, float minY, float maxY);
// Gets where the point is.
uint8_t GetPointLocationCode(Vec pt, Vec minBounds, Vec maxBounds);

// Global effects array
extern const GlobalEffect* SCOPE_GLOBAL_EFFECTS[TROWA_NUM_GLOBAL_EFFECTS];

/// TODO: Waveform: Thickness control & port (1 knob, 1 port)
/// TODO: Waveform: X&Y Size and position ports (4 ports)

#define TROWA_SCOPE_USE_Z_DIMENSION		0 // If we are using Z also, this will require more transforms and crap

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
	// Lissajous mode on
	bool lissajous = true;
	SchmittTrigger lissajousTrigger;

	// Link X and Y scale ::::::::::::::::::::::::::::::::::::::::::::::
	// Force aspect ratio lock
	bool linkXYScales;
	// Trigger for linkXYScales button
	SchmittTrigger linkXYScalesTrigger;
	// Last value for scale when X and Y were synched.
	float lastXYScaleValue;
	// Aspect Ratio X/Y:
	float aspectRatioXY = 1.0;

#if TROWA_SCOPE_USE_Z_DIMENSION
	// Number of axes
	int numAxes = 3;
	// Z-values
	float bufferZ[BUFFER_SIZE] = {};
	// Master Buffer pointer
	float* buffer[3] = { &(bufferX[0]), &(bufferY[0]), &(bufferZ[0]) };
	// Aspect Ratio X/Z:
	float aspectRatioXZ = 1.0;
	// Scale values (amplitudes for X, Y, Z).
	float scaleVals[3] = { 1.0, 1.0, 1.0 };
	// Offset values for X, Y, Z
	float offsetVals[3] = { 0.0, 0.0, 0.0 };
	// If the X, Y, Z inputs are active.
	bool inputsActive[3] = { false, false, false };
#else
	// Number of axes
	int numAxes = 2;
	// Master Buffer pointer
	float* buffer[2] = { &(bufferX[0]), &(bufferY[0])};
	// Scale values (amplitudes for X, Y).
	float scaleVals[2] = { 1.0, 1.0 };
	// Offset values for X, Y.
	float offsetVals[2] = { 0.0, 0.0 };
	// If the X, Y inputs are active.
	bool inputsActive[2] = { false, false };
#endif


	// Rotation ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	SchmittTrigger rotModeTrigger;
	// True for absolute angular position, false if constant angular change
	bool rotMode;
	// Value from rotation knob
	float rotKnobValue;
	// Translated to ABS position [radians]
	float rotAbsValue;
	// Translated to differential position [radians] (rate)
	float rotDiffValue;

	// Colors ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// Actual color
	NVGcolor waveColor;
	// Color hue 0 to 1
	float waveHue = 0;
	// Color saturation 0 to 1
	float waveSat = 0.5;
	// Color light 0 to 1
	float waveLght = 0.5;
#if TROWA_SCOPE_USE_COLOR_LIGHTS
	// References to our lights (typed)
	ColorValueLight* waveLight;
#endif
	// If the color has changed.
	bool colorChanged;
	// Alpha channel 0-1
	float waveOpacity = 1.0;

	// Fill color::::::::::::::::::::::::
	// Fill mode is on
	bool doFill = false;
	SchmittTrigger fillOnTrigger;
	// Color to use for fill.
	NVGcolor fillColor;
	// Fill hue (0-1)
	float fillHue = 0;
	// Fill saturation (0-1)
	float fillSat = 0.5;
	// Fill lum (0-1)
	float fillLght = 0.5;
	// Alpha channel 0-1.
	float fillOpacity = 1.0;


	// Rendering properties ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// Thickness of waveform line.
	float lineThickness = 3.0;
	// Negative the color.
	bool negativeImage = false;
	// Index into SCOPE_GLOBAL_EFFECTS for what effect to do.
	int gEffectIx = 0;

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
		lineThickness = 3.0;
		waveColor = HueToColor(waveHue, waveSat, waveLght);
		fillColor = HueToColor(fillHue, fillSat, fillLght);
#if TROWA_SCOPE_USE_COLOR_LIGHTS
		waveLight = NULL;
#endif
		return;
	}

	void setHue(float hue)
	{
		waveHue = hue;
		waveColor = HueToColor(waveHue, waveSat, waveLght);
	}
	void setHueFromKnob(float hueKnobValue)
	{
		setHue(rescale(hueKnobValue, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, 0.0, 1.0));
		return;
	}
	void setFillHue(float hue)
	{
		fillHue = hue;
		fillColor = HueToColor(hue, fillSat, fillLght);
	}
	void setFillHueFromKnob(float hueKnobValue)
	{
		setFillHue(rescale(hueKnobValue, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, 0.0, 1.0));
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// toJson(void)
	// Save to json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *toJson() {
		// Really should just serialize the TSWaveForm object.
		json_t* rootJ = json_object();
		json_t* itemJ;

		// Colors
		json_t* waveColorJ = json_array();
		json_t* fillColorJ = json_array();
		// -- RGB
		for (int i = 0; i < 3; i++)
		{
			itemJ = json_real(waveColor.rgba[i]);
			json_array_append_new(waveColorJ, itemJ);
			itemJ = NULL;

			itemJ = json_real(fillColor.rgba[i]);
			json_array_append_new(fillColorJ, itemJ);
			itemJ = NULL;
		}
		// -- Alpha (stored separately)
		itemJ = json_real(waveOpacity);
		json_array_append_new(waveColorJ, itemJ);
		itemJ = json_real(fillOpacity);
		json_array_append_new(fillColorJ, itemJ);
		json_object_set_new(rootJ, "waveColor", waveColorJ);
		json_object_set_new(rootJ, "fillColor", fillColorJ);

		json_object_set_new(rootJ, "fillOn", json_integer(this->doFill));
		json_object_set_new(rootJ, "lissajous", json_integer(this->lissajous));
		json_object_set_new(rootJ, "rotMode", json_integer(this->rotMode));
		json_object_set_new(rootJ, "linkXYScales", json_integer(this->linkXYScales));
		return rootJ;
	} // end toJson()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// fromJson(void)
	// Load settings.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void fromJson(json_t *rootJ) {
		json_t* itemJ;
		// Colors
		json_t* waveColorJ = json_object_get(rootJ, "waveColor");
		json_t* fillColorJ = json_object_get(rootJ, "fillColor");
		// -- RGB
		for (int i = 0; i < 3; i++)
		{
			if (waveColorJ)
			{
				itemJ = json_array_get(waveColorJ, i);
				if (itemJ)
					waveColor.rgba[i] = (float)(json_real_value(itemJ));
				itemJ = NULL;
			}
			if (fillColorJ)
			{
				itemJ = json_array_get(fillColorJ, i);
				if (itemJ)
					fillColor.rgba[i] = (float)(json_real_value(itemJ));
				itemJ = NULL;
			}
		}
		// -- Alpha
		if (waveColorJ)
		{
			itemJ = json_array_get(waveColorJ, 3);
			if (itemJ)
				waveOpacity = (float)(json_real_value(itemJ));
			itemJ = NULL;
		}
		if (fillColorJ)
		{
			itemJ = json_array_get(fillColorJ, 3);
			if (itemJ)
				fillOpacity = (float)(json_real_value(itemJ));
			itemJ = NULL;
		}
		itemJ = json_object_get(rootJ, "fillOn");
		if (itemJ)
		{
			doFill = (bool)(json_integer_value(itemJ));
			itemJ = NULL;
		}
		itemJ = json_object_get(rootJ, "lissajous");
		if (itemJ)
		{
			lissajous = (bool)(json_integer_value(itemJ));
			itemJ = NULL;
		}
		itemJ = json_object_get(rootJ, "rotMode");
		if (itemJ)
		{
			lissajous = (bool)(json_integer_value(itemJ));
			itemJ = NULL;
		}
		itemJ = json_object_get(rootJ, "linkXYScales");
		if (itemJ)
		{
			linkXYScales = (bool)(json_integer_value(itemJ));
			itemJ = NULL;
		}
		return;
	} // end fromJson()
};



#endif // !TSSCOPEMODULEBASE_HPP