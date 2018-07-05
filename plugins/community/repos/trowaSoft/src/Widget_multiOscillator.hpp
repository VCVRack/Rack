#ifndef WIDGET_MULTIOSCILLATOR_HPP
#define WIDGET_MULTIOSCILLATOR_HPP

#include "trowaSoftComponents.hpp"
#include "widgets.hpp"
#include "TSSModuleWidgetBase.hpp"
#include "Module_multiOscillator.hpp"
#include "rack.hpp"
#include "TSParamTextField.hpp"
using namespace rack;
#include <vector>
#include <string>

struct TSOscillatorChannelWidget;
struct TSSingleOscillatorWidget;
struct TSSingleOscillatorDisplay;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiOscillator
// Multiple digitial oscillators for drawing widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct multiOscillatorWidget : TSSModuleWidgetBase {
	// Number of oscillators. Should be in the module instance, but since we are no longer guaranteed a non-NULL reference, we will store the # oscillators here.
	int numberOscillators;
	/// Number of output (phase shifted) signals. Should be in the module instance, but since we are no longer guaranteed a non-NULL reference, store here.
	int numberOutputOscillators;
	// Plug lights
	bool plugLightsEnabled = true;
	// Text boxes for each oscillator (each oscillator has Amplitude, Frequency, Phase Shift, Offset).
	// Each output signal has Phase Shift.
	std::vector<TSParamTextField*> tbOscillatorValues;
	// Oscillators
	TS_Oscillator* oscillators;
	// The channel widgets for each output oscillator channel.
	std::vector<TSOscillatorChannelWidget*> channelWidgets;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiOscillatorWidget()
	// @thisModule : (IN) Pointer to the multiOscillator module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	multiOscillatorWidget(multiOscillator* thisModule);

	~multiOscillatorWidget();
	// Step
	void step() override;


	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// serialize()
	// To be used for pre-sets/settings.
	// @returns : The settings node.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	json_t* serialize();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// deserialize()
	// To be used for pre-sets/settings.
	// @rootJ : (IN) The settings json node.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void deserialize(json_t* rootJ);

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// savePreset()
	// @presetName : (IN) The name to use for the preset.
	// Save the current state as a preset with the given name (will clobber/overwrite
	// if an existing preset exists by the same name).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void savePreset(std::string presetName);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// loadPreset()
	// @presetName : (IN) The preset to load.
	// Load the preset with the given name.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void loadPreset(std::string presetName);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getPresets()
	// Get the presets names.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void getPresets();
};



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Display for the oscillator widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSingleOscillatorDisplay : TransparentWidget
{
	multiOscillator *module;
	TSSingleOscillatorWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	bool showBackground = false;	
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	bool showDisplay = true;
	const int numTextBoxes = 4;
	TSParamTextField* textBoxes[4];
	int phaseShiftIx = TS_Oscillator::BaseParamIds::OSCWF_PHASE_SHIFT_PARAM;
	const char* labels[4] = { "AMPL (V)", "FREQ (Hz)", "PHASE ( )", "OFFSET (V)" };
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSingleOscillatorTopDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSingleOscillatorDisplay() {
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 10;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
		showDisplay = true;
		return;
	}
	~TSSingleOscillatorDisplay() {
		module = NULL;
		parentWidget = NULL;
		return;
	}

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// A single oscillator info.
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;

	/**
	Called when a mouse button is pressed over this widget
	0 for left, 1 for right, 2 for middle.
	Return `this` to accept the event.
	Return NULL to reject the event and pass it to the widget behind this one.
	*/
	void onMouseDown(EventMouseDown &e) override {
		if (showDisplay) {
			if (e.button == 0)
			{
				// Left click, check position, find which text box this would go to.
				int txtBoxIx = -1;
				const int padding = 5;
				float dx = (box.size.x - padding * 2) / numTextBoxes;
				float x1 = padding;
				int i = 0;
				while (i < numTextBoxes && txtBoxIx < 0)
				{
					float x2 = x1 + dx;
					if (e.pos.x >= x1 && e.pos.x < x2) {
						txtBoxIx = i;
					}
					x1 += dx;
					i++;
				}
				if (txtBoxIx > -1 && !textBoxes[txtBoxIx]->visible)
				{
					// Show the text box:
					textBoxes[txtBoxIx]->visible = true;
					e.target = textBoxes[txtBoxIx];
					e.consumed = true;
				}
			} // end if left click
		} // end if visible
		return;
	} // end onMouseDown()
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Single oscillator widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSingleOscillatorWidget : Widget
{
	// Master/parent widget for the module.
	//multiOscillatorWidget* parentWidget;
	// Display for the main oscillator.
	TSSingleOscillatorDisplay* oscillatorDisplay;
	// Pointer to the main oscillator.
	TS_Oscillator* oscillator;
	// The channel widgets for each output oscillator channel.
	std::vector<TSOscillatorChannelWidget*> channelWidgets;
	// Oscillator number.
	int oscillatorNumber = 0;

	NVGcolor oscillatorColor = COLOR_WHITE;

	// Base input id.
	int baseInputId = 0;
	// Base parameter id.
	int baseParamId = 0;
	// Base output id.
	int baseOutputId = 0;
	// Base Light Id.
	int baseLightId = 0;

	// Parameter text boxes.
	std::vector<TSParamTextField*> tbParamValues;

	const int screenStartX = 270;// 240, 270
	const int screenStartY = 5;
	const int screenWidth = 365;
	const int outPortOffsetX = 10;

	// Parameter text boxes (even child text bo
	std::vector<TSParamTextField*> tbAllParamValues;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSingleOscillatorWidget()
	// @parentWidget: (IN) Parent MODULE widget.
	// @osc : (IN) Pointer to the oscillator this widget represents.
	// @num : (IN) The oscillator number.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSingleOscillatorWidget(multiOscillatorWidget* parentWidget, TS_Oscillator* osc, int num);

	~TSSingleOscillatorWidget()
	{
		tbParamValues.clear();
		tbAllParamValues.clear();
		channelWidgets.clear();
		//parentWidget = NULL;
		oscillatorDisplay = NULL;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Display for an output channel.
// (Type: SIN, TRI, SQU, SAW; Aux, Phase, Mod)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscillatorChannelDisplayWidget : TransparentWidget
{
	// Parent module widget
	//multiOscillatorWidget* parentModuleWidget;
	TSOscillatorChannelWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	bool showBackground = false;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	bool showDisplay = true;
	const char* labels[4] = { "WAVE", "AUX", "PHASE ( )", "MOD (%)" };
	bool hasTextBox[4] = { false, true, true, true };
	TSParamTextField* textBoxes[4] = { NULL, NULL, NULL, NULL };
	const int numFields = 4;
	int yTbStart = 0;
	int yTbEnd = 25;
	static constexpr int phaseShiftIx = 2;
	static constexpr int amodIx = 3;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscillatorChannelDisplayWidget(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscillatorChannelDisplayWidget() {
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 10;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
		showDisplay = true;
		return;
	}
	~TSOscillatorChannelDisplayWidget() {
		parentWidget = NULL;
		for (int i = 0; i < numFields; i++)
		{
			textBoxes[i] = NULL;
		}
	}
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// A single oscillator info.
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;

	/**
	Called when a mouse button is pressed over this widget
	0 for left, 1 for right, 2 for middle.
	Return `this` to accept the event.
	Return NULL to reject the event and pass it to the widget behind this one.
	*/
	void onMouseDown(EventMouseDown &e) override;

};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Widget for an output channel.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscillatorChannelWidget : VirtualWidget
{
	// Parent MODULE widget.
	//multiOscillatorWidget* parentModuleWidget;
	// Parent oscillator widget.
	TSSingleOscillatorWidget* parentWidget;
	// Label font
	std::shared_ptr<Font> labelFont;
	// Font size
	int fontSize = 10;
	// The channel number.
	int channelNumber = 1;
	// The channel color.
	NVGcolor channelColor;
	//const int xmargin = TROWA_HORIZ_MARGIN / 2;
	//const int ymargin = TROWA_VERT_MARGIN + 20;
	//char channelLabel[20];
	// Base input id.
	int baseInputId = 0;
	// Base parameter id.
	int baseParamId = 0;
	// Base output id.
	int baseOutputId = 0;
	// Base Light Id.
	int baseLightId = 0;
	// The oscillator output object.
	TS_OscillatorOutput* oscillatorOutput = NULL;	
	// Text boxes for oscillator parameters.
	// Each output signal has AM Mix, Phase Shift, Aux.
	std::vector<TSParamTextField*> tbParamValues;
	TSOscillatorChannelWidget(multiOscillatorWidget* parentModuleWidget, TSSingleOscillatorWidget* parentOscWidget, Vec location, int chNumber, NVGcolor chColor, int bInputId, int bParamId, int bOutputId, int bLightId, TS_OscillatorOutput* oscOutput);

	~TSOscillatorChannelWidget()
	{
		parentWidget = NULL;
		oscillatorOutput = NULL;
		tbParamValues.clear();
		return;
	}

	void step() override
	{
		// AUX:
		tbParamValues[0]->canTabToThisEnabled = (oscillatorOutput->waveFormType == WaveFormType::WAVEFORM_SQR);
		VirtualWidget::step();
		return;
	}

};

#endif // !WIDGET_MULTIOSCILLATOR_HPP
