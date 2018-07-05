#ifndef WIDGET_OSCCV_HPP
#define WIDGET_OSCCV_HPP

#include "widgets.hpp"
#include "TSSModuleWidgetBase.hpp"
#include "TSOSCConfigWidget.hpp"
#include "Module_oscCV.hpp"
#include "rack.hpp"
#include <vector>
using namespace rack;

#define TROWA_SCROLLING_MSG_TOTAL_SIZE		256
#define TROWA_OSCCV_NUM_COLORS				  8

struct oscCV;
struct TSOscCVTopDisplay;
struct TSOscCVMiddleDisplay;
struct TSOscCVLabels;
struct TSOscCVChannelConfigScreen;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVWidget
// Open Sound Control <==> Control Voltage widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct oscCVWidget : TSSModuleWidgetBase {
	// OSC configuration widget.
	TSOSCConfigWidget* oscConfigurationScreen;
	// The top display.
	TSOscCVTopDisplay* display;
	// The middle display
	TSOscCVMiddleDisplay* middleDisplay;
	// Screen for the channel configuration.
	TSOscCVChannelConfigScreen* oscChannelConfigScreen;
	// Number of channels. Should be in the module instance, but since we are no longer guaranteed a non-NULL reference, we will store the # channels here.
	int numberChannels;
	// Rack CV Input (osc trans messge path)
	std::vector<TSTextField*> tbOscInputPaths;
	// Rack CV Output (osc recv message path)
	std::vector<TSTextField*> tbOscOutputPaths;
	// If we should color the channels.
	bool colorizeChannels = true;
	// Advanced channel configs
	std::vector<TS_ScreenBtn*> btnDrawInputAdvChConfig;
	// Advanced channel configs
	std::vector<TS_ScreenBtn*> btnDrawOutputAdvChConfig;



	// Channel colors
	/// TODO: Move this to someplace else
	static const NVGcolor CHANNEL_COLORS[TROWA_OSCCV_NUM_COLORS];
	// Plug lights
	bool plugLightsEnabled = true;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVWidget()
	// Instantiate a oscCV widget. v0.60 must have module as param.
	// @oscModule : (IN) Pointer to the osc module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVWidget(oscCV* oscModule);

	~oscCVWidget()
	{
		oscConfigurationScreen = NULL;
		display = NULL;
		middleDisplay = NULL;
		oscChannelConfigScreen = NULL;
		btnDrawInputAdvChConfig.clear();
		btnDrawOutputAdvChConfig.clear();
		return;
	}
	// Step
	void step() override;

	// Show or hide the channel configuration
	void toggleChannelPathConfig(bool show);
	// Read the channel path configs and store in module's channels.
	void readChannelPathConfig();
	// Set the channel path text boxes.
	void setChannelPathConfig();
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Labels for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVLabels : TransparentWidget {
	std::shared_ptr<Font> font;
	int fontSize;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVLabels(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVLabels()
	{
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	// Draw labels on our widget.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;
}; // end TSOscCVLabels

// Button to chose OSC data type
struct TSOscCVDataTypeSelectBtn : ChoiceButton {
	// The selected value.
	int selectedVal;
	int selectedIndex;
	int numVals;
	int* itemVals;
	std::string* itemStrs;
	bool visible = false;
	std::shared_ptr<Font> font;
	Vec textOffset;
	NVGcolor color;
	// Font size
	float fontSize;
	std::string displayStr;
	int borderWidth = 0;
	NVGcolor borderColor;
	NVGcolor backgroundColor;
	int showNumChars = 15;
	TSOscCVChannelConfigScreen* parentScreen = NULL;

	TSOscCVDataTypeSelectBtn(int numVals, int* itemVals, std::string* itemStrs, int selVal) {
		font = Font::load(assetPlugin(plugin, TROWA_MONOSPACE_FONT));
		fontSize = 14.0f;
		backgroundColor = FORMS_DEFAULT_BG_COLOR;
		color = FORMS_DEFAULT_TEXT_COLOR;
		textOffset = Vec(5, 3);
		borderWidth = 1;
		borderColor = FORMS_DEFAULT_BORDER_COLOR;

		this->numVals = numVals;
		this->selectedVal = selVal;
		this->itemVals = itemVals;
		this->itemStrs = itemStrs;
		for (int i = 0; i < numVals; i++)
		{
			if (itemVals[i] == selectedVal)
				selectedIndex = i;
		}
		return;
	}
	// When the selected index changes.
	void onSelectedIndexChanged();

	// Set the selected value.
	void setSelectedValue(int selVal) {
		this->selectedVal = selVal;
		for (int i = 0; i < numVals; i++)
		{
			if (itemVals[i] == selectedVal)
				selectedIndex = i;
		}
		onSelectedIndexChanged();
		return;
	}
	// Set the selected index.
	void setSelectedIndex(int selIx) {
		this->selectedIndex = selIx;
		this->selectedVal = itemVals[selectedIndex];
		onSelectedIndexChanged();
		return;
	}
	void step() override {
		text = stringEllipsize(itemStrs[selectedIndex], showNumChars);
	}
	void onAction(EventAction &e) override;
	// Draw if visible
	void draw(NVGcontext *vg) override;
};
// An OSC client option in dropdown.
struct TSOscCVDataTypeItem : MenuItem {
	int itemVal;
	int index;
	TSOscCVDataTypeSelectBtn* parentButton;
	TSOscCVDataTypeItem(TSOscCVDataTypeSelectBtn* parent, int index)
	{
		parentButton = parent;
		this->index = index;
		return;
	}
	void onAction(EventAction &e) override {
		parentButton->setSelectedIndex(index);
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Single Channel configuration (advanced) widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVChannelConfigScreen : OpaqueWidget {
	oscCVWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	bool visible = false;
	// If this is an input or output channel.
	bool isInput = false;
	enum TextBoxIx {
		// Minimum CV voltage
		MinCVVolt,
		// Maximum CV voltage
		MaxCVVolt,
		// Minimum OSC value
		MinOSCVal,
		// Maximum OSC value
		MaxOSCVal,
		NumTextBoxes
	};

	// The text boxes for min/max values.
	TSTextField* tbNumericBounds[TextBoxIx::NumTextBoxes];
	std::string tbErrors[TextBoxIx::NumTextBoxes];

	const int numDataTypes = 3;
	TSOSCCVChannel::ArgDataType oscDataTypeVals[3] = { TSOSCCVChannel::ArgDataType::OscFloat, TSOSCCVChannel::ArgDataType::OscInt, TSOSCCVChannel::ArgDataType::OscBool };
	std::string oscDataTypeStr[3] = { std::string("Float"), std::string("Int"), std::string("Bool") };
	// The selected data type.
	TSOSCCVChannel::ArgDataType selectedDataType = TSOSCCVChannel::ArgDataType::OscFloat;
	// Label buffer
	char buffer[50];

	// OSC Data Type select/dropdown
	TSOscCVDataTypeSelectBtn* btnSelectDataType;
	// Turn on / off translating values.
	bool translateValsEnabled = false;
	//HideableLEDButton* btnToggleTranslateVals;
	TS_ScreenCheckBox* btnToggleTranslateVals;
	//ColorValueLight* lightTranslateVals;
	SchmittTrigger translateTrigger;

	// Save button
	TS_ScreenBtn* btnSave;
	// Cancel button
	TS_ScreenBtn* btnCancel;
	SchmittTrigger saveTrigger;
	SchmittTrigger cancelTrigger;

	// Pointer to the current channel information.
	TSOSCCVChannel* currentChannelPtr = NULL;

	int startX = 6;
	int startY = 6;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVChannelConfigScreen(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVChannelConfigScreen() : TSOscCVChannelConfigScreen(NULL, Vec(0,0), Vec(300, 300)) {
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVChannelConfigScreen()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVChannelConfigScreen(oscCVWidget* widget, Vec pos, Vec boxSize);

	~TSOscCVChannelConfigScreen()
	{
		parentWidget = NULL;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Set visible or not.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setVisibility(bool visible) {
		//debug("setVisibility(%d)", visible);
		this->visible = visible;
		try
		{
			//debug("setVisibility(%d) - btn", visible);
			if (btnToggleTranslateVals)
				btnToggleTranslateVals->visible = visible;
			//debug("setVisibility(%d) - light", visible);
			//if (lightTranslateVals)
			//	lightTranslateVals->visible = visible;
			//debug("setVisibility(%d) - dropdown", visible);
			this->btnSelectDataType->visible = visible;

			//debug("setVisibility(%d) - buttons", visible);
			btnSave->visible = visible;
			btnCancel->visible = visible;
			//debug("setVisibility(%d) - text boxes", visible);
			for (int i = 0; i < TextBoxIx::NumTextBoxes; i++)
			{
				tbNumericBounds[i]->visible = visible;
			}
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
		//debug("setVisibility(%d) - Done", visible);
		return;
	} // end setVisibility()

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Save the values to the ptr.
	// @channelPtr : (OUT) Place to save the values.
	// @returns : True if saved, false if there was an error.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	bool saveValues(/*out*/ TSOSCCVChannel* channelPtr);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Save the values to current ptr.
	// @returns : True if saved, false if there was an error.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	bool saveValues() {
		return saveValues(this->currentChannelPtr);		
	}

	void setDataType(TSOSCCVChannel::ArgDataType dataType)
	{
		if (dataType == TSOSCCVChannel::ArgDataType::OscBool)
		{
			// Bools have to be false/true....
			tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = false;
			tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = false;
			tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string("0");
			tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string("1");
		}
		else
		{
			tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = true;
			tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = true;
		}
		return;
	}

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// validateValues(void)
	// @returns : True if valid, false if not.
	// POST CONDITION: tbErrors is set and errors may be displayed on the screen.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	bool validateValues();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// showControl()
	// @channel: (IN) The channel which we are configuring.
	// @isInput: (IN) If this an input or output.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void showControl(TSOSCCVChannel* channel, bool isInput);

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Process
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Top Display for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVTopDisplay : TransparentWidget {
	oscCVWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	bool showDisplay = true;
	char scrollingMsg[TROWA_SCROLLING_MSG_TOTAL_SIZE];
	int scrollIx = 0;
	std::string lastIp = std::string("");
	float dt = 0.0f;
	float scrollTime_sec = 0.05f;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVTopDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVTopDisplay() : TSOscCVTopDisplay(NULL) {
		return;
	}
	~TSOscCVTopDisplay() {
		parentWidget = NULL;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVMiddleDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVTopDisplay(oscCVWidget* widget)
	{
		parentWidget = widget;
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		memset(messageStr, '\0', sizeof(char)*TROWA_DISP_MSG_SIZE);
		memset(scrollingMsg, '\0', sizeof(char)*TROWA_SCROLLING_MSG_TOTAL_SIZE);
		showDisplay = true;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// step()
	// Calculate scrolling and stuff?
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;

}; // end struct TSOscCVTopDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Middle Display for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVMiddleDisplay : TransparentWidget {
	oscCVWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	//bool showDisplay = true;

	enum DisplayMode {
		None = 0,
		Default = 1
	};

	DisplayMode displayMode = DisplayMode::Default;
	// Current channel path position (for paths that are too large and need some scrolling).
	float chPathPosition = 0.0f;
	// Amt of time that has passed.
	float dt = 0.0f;
	// Scrolling time
	float scrollTime = 0.05f;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVMiddleDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVMiddleDisplay(oscCVWidget* widget)
	{
		parentWidget = widget;
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
		return;
	}
	~TSOscCVMiddleDisplay() {
		parentWidget = NULL;
		return;
	}

	void setDisplayMode(DisplayMode mode) {
		displayMode = mode;
		if (displayMode == DisplayMode::Default)
		{
			chPathPosition = 0.0f; // reset
			dt = 0.0f; // reset
		}
		return;
	}

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Process
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawChannelChart()
	// Draw the channel data.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawChannelChart(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,  /*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height, /*in*/ NVGcolor lineColor);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawChannelBar()
	// Draw the channel data.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawChannelBar(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,  /*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height, /*in*/ NVGcolor lineColor);

}; // end struct TSOscCVMiddleDisplay

#endif // !WIDGET_OSCCV_HPP
