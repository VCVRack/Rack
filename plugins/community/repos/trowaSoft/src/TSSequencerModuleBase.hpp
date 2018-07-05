#ifndef TROWASOFT_MODULE_TSSEQUENCERBASE_HPP
#define TROWASOFT_MODULE_TSSEQUENCERBASE_HPP

#include "rack.hpp"
using namespace rack;

#include <thread> // std::thread
#include <mutex>
#include <queue>
#include <vector>
#include <string.h>
#include <stdio.h>
//#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include <chrono>
#include "TSTempoBPM.hpp"
#include "TSExternalControlMessage.hpp"
#include "TSOSCCommon.hpp"
#include "TSOSCSequencerListener.hpp"
#include "TSOSCCommunicator.hpp"
#include "TSOSCSequencerOutputMessages.hpp"
#include "TSSequencerWidgetBase.hpp"

#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

#define TROWA_SEQ_NUM_CHNLS		16	// Num of channels/triggers/voices
#define TROWA_SEQ_NUM_STEPS		16  // Num of steps per gate/voice
#define TROWA_SEQ_MAX_NUM_STEPS	64  // Maximum number of steps

#define N64_NUM_STEPS	64
#define N64_NUM_ROWS	 8
#define N64_NUM_COLS	 (N64_NUM_STEPS/N64_NUM_ROWS)


// Default OSC outgoing address (Tx). 127.0.0.1.
#define OSC_ADDRESS_DEF		"127.0.0.1"
// Default OSC outgoing port (Tx). 7000.
#define OSC_OUTPORT_DEF		7000
// Default OSC incoming port (Rx). 7001.
#define OSC_INPORT_DEF		7001
// Default namespace for OSC
#define OSC_DEFAULT_NS				"/tsseq"
#define OSC_OUTPUT_BUFFER_SIZE		(1024*TROWA_SEQ_MAX_NUM_STEPS)
#define OSC_ADDRESS_BUFFER_SIZE		50
// If we should update the current step pointer to OSC (turn off prev step, highlight current step).
// This gets slow though during testing.
#define OSC_UPDATE_CURRENT_STEP_LED		1

// We only show 4x4 grid of steps at time.
#define TROWA_SEQ_STEP_NUM_ROWS	4	// Num of rows for display of the Steps (single Gate displayed at a time)
#define TROWA_SEQ_STEP_NUM_COLS	(TROWA_SEQ_NUM_STEPS/TROWA_SEQ_STEP_NUM_ROWS)

#define TROWA_SEQ_NUM_MODES		3
#define TROWA_SEQ_STEPS_MIN_V	TROWA_SEQ_PATTERN_MIN_V   // Min voltage input / output for controlling # steps
#define TROWA_SEQ_STEPS_MAX_V	TROWA_SEQ_PATTERN_MAX_V   // Max voltage input / output for controlling # steps
#define TROWA_SEQ_BPM_KNOB_MIN		-2	
#define TROWA_SEQ_BPM_KNOB_MAX		 6
#define TROWA_SEQ_SWING_ADJ_MIN			-0.5
#define TROWA_SEQ_SWING_ADJ_MAX		     0.5
#define TROWA_SEQ_SWING_STEPS		4
// 0 WILL BE NO SWING

// To copy all gates/triggers in the selected target Pattern
#define TROWA_SEQ_COPY_CHANNELIX_ALL		TROWA_INDEX_UNDEFINED 

#define TROWA_SEQ_NUM_RANDOM_PATTERNS			27
#define TROWA_SEQ_BOOLEAN_NUM_RANDOM_PATTERNS	 7	// Num of patterns that actually apply to a boolean sequencer. List should always be sorted by #Unique Values. Only #Unique vals 1-2 should be chosen.
// Random Structure
// From feature request: https ://github.com/j4s0n-c/trowaSoft-VCV/issues/10
struct RandStructure {
	uint8_t numDiffVals;
	std::vector<uint8_t> pattern;
};

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerModuleBase
// Sequencer Base Class
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSequencerModuleBase : Module {
	enum ParamIds {
		// BPM Knob
		BPM_PARAM,
		// Run toggle		
		RUN_PARAM,
		// Reset Trigger (Momentary)		
		RESET_PARAM,
		// Step length
		STEPS_PARAM,
		SELECTED_PATTERN_PLAY_PARAM,  // What pattern we are playing
		SELECTED_PATTERN_EDIT_PARAM,  // What pattern we are editing
		SELECTED_CHANNEL_PARAM,	 // Which gate is selected for editing		
		SELECTED_OUTPUT_VALUE_MODE_PARAM,     // Which value mode we are doing	
		SWING_ADJ_PARAM, // Amount of swing adjustment (-0.1 to 0.1)
		COPY_PATTERN_PARAM, // Copy the current editing Pattern
		COPY_CHANNEL_PARAM, // Copy the current Channel/gate/trigger in the current Pattern only.
		PASTE_PARAM, // Paste what is on our clip board to the now current editing.
		SELECTED_BPM_MULT_IX_PARAM, // Selected index into our BPM calculation multipliers (for 1/4, 1/8, 1/8T, 1/16 note calcs)
		OSC_SAVE_CONF_PARAM, // ENABLE and Save the configuration for OSC
		OSC_AUTO_RECONNECT_PARAM,   // Auto-reconnect OSC on load from save file.
		OSC_SHOW_CONF_PARAM, // Configure OSC toggle
		CHANNEL_PARAM, // Edit Channel/Step Buttons/Knobs
		NUM_PARAMS = CHANNEL_PARAM // Add the number of steps separately...
	};
	enum InputIds {
		// BPM Input
		BPM_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		SELECTED_PATTERN_PLAY_INPUT,  // What pattern we are playing		
		SELECTED_PATTERN_EDIT_INPUT,  // What pattern we are editing
		UNUSED_INPUT,
		NUM_INPUTS
	};
	// Each of the 16 voices need a gate output
	enum OutputIds {
		CHANNELS_OUTPUT, // Output Channel ports
		NUM_OUTPUTS = CHANNELS_OUTPUT + TROWA_SEQ_NUM_CHNLS
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
		COPY_PATTERN_LIGHT, // Copy pattern
		COPY_CHANNEL_LIGHT, // Copy channel
		PASTE_LIGHT,	// Paste light
		SELECTED_BPM_MULT_IX_LIGHT,	// BPM multiplier/note index
		OSC_CONFIGURE_LIGHT, // The light for configuring OSC.
		OSC_ENABLED_LIGHT, // Light for OSC enabled and currently running/active.
		CHANNEL_LIGHTS, // Channel output lights.		
		PAD_LIGHTS = CHANNEL_LIGHTS + TROWA_SEQ_NUM_CHNLS, // Lights for the steps/pads for the currently editing Channel
														   // Not the number of lights yet, add the # of steps (maxSteps)
														   NUM_LIGHTS = PAD_LIGHTS // Add the number of steps separately...
	};

	// The random structures/patterns
	static RandStructure RandomPatterns[TROWA_SEQ_NUM_RANDOM_PATTERNS];

	// If the module has been fully initialized or not.
	bool initialized = false;
	// If reset is pressed while paused, when we play, we should fire step 0.
	bool resetPaused = false;
	// [03/30/2015] A  reset has been queued for the next step. (https://github.com/j4s0n-c/trowaSoft-VCV/issues/11)
	// So now reset is not immediate, but will wait for the next step.
	bool resetQueued = false;
	// If this module is running.
	bool running = true;
	SchmittTrigger clockTrigger; 		// for external clock
	SchmittTrigger runningTrigger;		// Detect running btn press
	SchmittTrigger resetTrigger;		// Detect reset btn press
	float realPhase = 0.0;
	// Index into the sequence (step)
	int index = 0;
	// Last index we played (for OSC)
	int prevIndex = TROWA_INDEX_UNDEFINED;
	// Next index in to jump to in the sequence (step) if any. (for external controls)
	int nextIndex = TROWA_INDEX_UNDEFINED;
	// Flag if values are being changed outside of step().
	bool valuesChanging = false;

	enum GateMode : short {
		TRIGGER = 0,
		RETRIGGER = 1,
		CONTINUOUS = 2,
	};
	GateMode gateMode = TRIGGER;
	PulseGenerator gatePulse;

	enum ValueMode : short {
		VALUE_TRIGGER = 0,
		VALUE_RETRIGGER = 1,
		VALUE_CONTINUOUS = 2,
		VALUE_VOLT = 0,
		VALUE_MIDINOTE = 1,
		VALUE_PATTERN = 2,
		MIN_VALUE_MODE = 0,
		MAX_VALUE_MODE = 2,
		NUM_VALUE_MODES = MAX_VALUE_MODE + 1
	};
	// Selected output value mode.
	ValueMode selectedOutputValueMode = VALUE_TRIGGER;
	ValueMode lastOutputValueMode = VALUE_TRIGGER;

	// Maximum number of steps for this sequencer.
	int maxSteps = 16;
	// The number of rows for steps (for layout).
	int numRows = 4;
	// The number of columns for steps (for layout).
	int numCols = 4;
	// Step data for each pattern and channel.
	float * triggerState[TROWA_SEQ_NUM_PATTERNS][TROWA_SEQ_NUM_CHNLS];
	SchmittTrigger* gateTriggers;

	// Knob indices for top control knobs.
	enum KnobIx {
		// Playing pattern knob ix
		PlayPatternKnob = 0,
		// Playing BPM knob ix
		BPMKnob,
		// Playing Step Length ix
		StepLengthKnob,
		// Output mode knob ix
		OutputModeKnob,
		// Edit pattern knob ix
		EditPatternKnob,
		// Edic channel knob ix
		EditChannelKnob,
		// Number of Control Knobs
		NumKnobs
	};
	// References to input knobs (top row of knobs)
	SVGKnob* controlKnobs[NumKnobs];

	// Another flag to reload the matrix.
	bool reloadEditMatrix = false;
	// Keep track of the pattern that was playing last step (for OSC)
	int lastPatternPlayingIx = -1;
	// Index of which pattern we are playing
	int currentPatternEditingIx = 0;
	// Index of which pattern we are editing 
	int currentPatternPlayingIx = 0;
	// Index of which channel (trigger/gate/voice) is currently displayed/edited.
	int currentChannelEditingIx = 0;
	/// TODO: Perhaps change this to setting for each pattern or each pattern-channel.
	// The current number of steps to play
	int currentNumberSteps = TROWA_SEQ_NUM_STEPS;
	// Calculated current BPM
	float currentBPM = 0.0f;
	// If the last step was the external clock
	bool lastStepWasExternalClock = false;
	// Currently stored pattern (for external control like OSC clients that can not store values themselves, the controls can set a 'stored' value
	// and then have some button click fire off the SetPlayPattern message with -1 as argument and we'll use this.
	int storedPatternPlayingIx = 0;
	// Currently stored length (for external control like OSC clients that can not store values themselves, the controls can set a 'stored' value
	// and then have some button click fire off the SetPlayLength message with -1 as argument and we'll use this.
	int storedNumberSteps = TROWA_SEQ_NUM_STEPS;
	// Currently stored BPM (for external control like OSC clients that can not store values themselves, the controls can set a 'stored' value
	// and then have some button click fire off the SetPlayBPM message with -1 as argument and we'll use this.
	int storedBPM = 120;
	//// Last time of the external step
	//std::chrono::high_resolution_clock::time_point lastExternalStepTime;

	// Pad/Knob lights - Step On
	float** stepLights; /// TODO: Just make linear
	float** gateLights; /// TODO: Just make linear

						// Default values for our pads/knobs:
	float defaultStateValue = 0.0;

	// References to our pad lights
	ColorValueLight*** padLightPtrs; /// TODO: Just make linear

	// Output lights (for triggers/gate jacks)
	float gateLightsOut[TROWA_SEQ_NUM_CHNLS];
	// Colors for each channel
	NVGcolor voiceColors[TROWA_SEQ_NUM_CHNLS] = {
		COLOR_TS_RED, COLOR_DARK_ORANGE, COLOR_YELLOW, COLOR_TS_GREEN,
		COLOR_CYAN, COLOR_TS_BLUE, COLOR_PURPLE, COLOR_PINK,
		COLOR_TS_RED, COLOR_DARK_ORANGE, COLOR_YELLOW, COLOR_TS_GREEN,
		COLOR_CYAN, COLOR_TS_BLUE, COLOR_PURPLE, COLOR_PINK
	};

	// Swing ////////////////////////////////
	float swingAdjustment = 0.0; // Amount of swing adjustment (i.e. -0.1 to 0.1)
	const int swingResetSteps = TROWA_SEQ_SWING_STEPS; // These many steps need to be adjusted.
	float swingAdjustedPhase = 0.0;
	int swingRealSteps = 0;

	// Copy & Paste /////////////////////////
	// Source pattern to copy
	int copySourcePatternIx = -1;
	// Source channel to copy (or TROWA_SEQ_COPY_CHANNELIX_ALL for all).
	int copySourceChannelIx = TROWA_SEQ_COPY_CHANNELIX_ALL;
	// Copy buffer
	float* copyBuffer[TROWA_SEQ_NUM_CHNLS];
	SchmittTrigger copyPatternTrigger;
	SchmittTrigger copyGateTrigger;
	SchmittTrigger pasteTrigger;
	/// TODO: Maybe eventually separate UI controls from module (UI changes in Widget not Module).
	// Light for paste button
	TS_LightString* pasteLight;
	// Light for copy pattern button
	ColorValueLight* copyPatternLight;
	// Light for copy channel button
	ColorValueLight* copyGateLight;

	// BPM Calculation //////////////
	// Index into the array BPMOptions
	int selectedBPMNoteIx = 1; // 1/8th
	SchmittTrigger selectedBPMNoteTrigger;

	// External Messages ///////////////////////////////////////////////
	// Message queue for external (to Rack) control messages
	std::queue<TSExternalControlMessage> ctlMsgQueue;

	enum ExternalControllerMode {
		// Edit Mode : Send to control what we are editing.
		EditMode = 0,
		// Performance/Play mode : Send to controller what we are playing
		// SetStepValue messages should be interupted as SetPlayingStep (Jump)
		PerformanceMode = 1
	};
	// The current control mode (i.e. Edit Mode or Play / Performance Mode)
	ExternalControllerMode currentCtlMode = ExternalControllerMode::EditMode;

	// OSC Messaging ////////////////
	// If we allow osc or not.
	bool allowOSC = true;
	// Flag if we should use OSC or not.
	bool useOSC = true;
	// An OSC id.
	int oscId = 0;
	// Mutex for osc messaging.
	std::mutex oscMutex;
	// Current OSC IP address and port settings.
	TSOSCConnectionInfo currentOSCSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Configure trigger
	SchmittTrigger oscConfigTrigger;
	SchmittTrigger oscConnectTrigger;
	SchmittTrigger oscDisconnectTrigger;
	// Show the OSC configuration screen or not.
	bool oscShowConfigurationScreen = false;
	// Flag to reconnect at load. IFF true and oscInitialized is also true.
	bool oscReconnectAtLoad = false;
	// Flag if OSC objects have been initialized
	bool oscInitialized = false;
	// If there is an osc error.
	bool oscError = false;
	// OSC output buffer.
	char* oscBuffer = NULL;
	// OSC namespace to use
	std::string oscNamespace = OSC_DEFAULT_NS;
	// Sending OSC socket
	UdpTransmitSocket* oscTxSocket = NULL;
	// OSC message listener
	TSOSCSequencerListener* oscListener = NULL;
	// Receiving OSC socket
	UdpListeningReceiveSocket* oscRxSocket = NULL;
	// The OSC listener thread
	std::thread oscListenerThread;
	// Osc address buffer. 
	char oscAddrBuffer[SeqOSCOutputMsg::NUM_OSC_OUTPUT_MSGS][OSC_ADDRESS_BUFFER_SIZE];
	// Prev step that was last turned off (when going to a new step).
	int oscLastPrevStepUpdated = TROWA_INDEX_UNDEFINED;
	// Settings for new OSC.
	TSOSCInfo oscNewSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Mode action (i.e. Enable, Disable)
	enum OSCAction {
		None,
		Disable,
		Enable
	};
	// Flag for our module to either enable or disable osc.
	OSCAction oscCurrentAction = OSCAction::None;
	// The current osc client. Clients such as touchOSC and Lemur are limited and need special treatment.
	OSCClient oscCurrentClient = OSCClient::GenericClient;

	// Mode /////////////////////////	
	// The mode string.
	const char* modeString;
	// Mode strings
	const char* modeStrings[3];

	// If it is the first load this session
	bool firstLoad = true;
	// If this was loaded from a save, what version
	int saveVersion = -1;
	const float lightLambda = 0.05;
	// The number of structured random patterns to actually use. Should be <= TROWA_SEQ_NUM_RANDOM_PATTERNS.
	int numStructuredRandomPatterns = TROWA_SEQ_BOOLEAN_NUM_RANDOM_PATTERNS;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSequencerModuleBase()
	// Instantiate the abstract base class.
	// @numSteps: (IN) Maximum number of steps
	// @numRows: (IN) The number of rows (for layout).
	// @numCols: (IN) The number of columns (for layout).
	// @numRows * @numCols = @numSteps
	// @defStateVal : (IN) The default state value (i.e. 0/false for a boolean step sequencer or whatever float value you want).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSequencerModuleBase(/*in*/ int numSteps, /*in*/ int numRows, /*in*/ int numCols, /*in*/ float defStateVal);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Delete our goodies.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	~TSSequencerModuleBase();
	// Get the inputs for this step.
	void getStepInputs(bool* pulse, bool* reloadMatrix, bool* valueModeChanged);
	// Paste the clipboard pattern and/or specific gate to current selected pattern and/or gate.
	bool paste();
	// Copy the contents:
	void copy(int patternIx, int channelIx);
	// Set a single step value
	virtual void setStepValue(int step, float val, int channel, int pattern);
	// Get the toggle step value
	virtual float getToggleStepValue(int step, float val, int channel, int pattern) = 0;
	// Calculate a representation of all channels for this step
	virtual float getPlayingStepValue(int step, int pattern) = 0;

	// Initialize OSC on the given ip and ports.
	void initOSC(const char* ipAddress, int outputPort, int inputPort);
	// Clean up OSC.
	void cleanupOSC();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Set the OSC namespace.
	// @oscNs: (IN) The namespace for OSC.
	// Sets the command address strings too.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setOSCNamespace(const char* oscNs);


	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// reset(void)
	// Reset ALL step values to default.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void reset() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomize(void)
	// Only randomize the current gate/trigger steps.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void randomize() override
	{
		randomize(currentPatternEditingIx, currentChannelEditingIx, false);
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomize()
	// @patternIx : (IN) The index into our pattern matrix (0-63). Or TROWA_INDEX_UNDEFINED for all patterns.
	// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
	// @useStructured: (IN) Create a random sequence/pattern of random values.
	// Random all from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/8
	// Structured from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/10
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void randomize(int patternIx, int channelIx, bool useStructured);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getRandomValue()
	// Get a random value for a step in this sequencer.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	virtual float getRandomValue() {
		// Default are boolean sequencers
		return randomUniform() > 0.5;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// onShownStepChange()
	// If we changed a step that is shown on the matrix, then do something.
	// For voltSeq to adjust the knobs so we dont' read the old knob values again.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	virtual void onShownStepChange(int step, float val) {
		// DO nothing
		return;
	}

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// clearClipboard(void)
	// Shallow clear of clipboard and reset the Copy/Paste lights
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	void clearClipboard()
	{
		copySourcePatternIx = -1;
		copySourceChannelIx = TROWA_SEQ_COPY_CHANNELIX_ALL; // Which trigger we are copying, -1 for all		
		lights[COPY_CHANNEL_LIGHT].value = 0;
		pasteLight->setColor(COLOR_WHITE); // Return the paste light to white
		lights[COPY_PATTERN_LIGHT].value = 0;
		lights[PASTE_LIGHT].value = 0;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// toJson(void)
	// Save our junk to json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *toJson() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// fromJson(void)
	// Read in our junk from json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void fromJson(json_t *rootJ) override;
}; // end struct TSSequencerModuleBase

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqDisplay
// A top digital display for trowaSoft sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSeqDisplay : TransparentWidget {
	TSSequencerModuleBase *module;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	bool showDisplay = true;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSeqDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSeqDisplay() {
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
		showDisplay = true;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override {
		bool isPreview = module == NULL; // May get a NULL module for preview

		// Background Colors:
		NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

		// Screen:
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		if (!showDisplay)
			return;

		int currPlayPattern = 1;
		int currEditPattern = 1;
		int currentGate = 1;
		int currentNSteps = 16;
		float currentBPM = 120;
		NVGcolor currColor = COLOR_RED;

		if (!isPreview)
		{
			currColor = module->voiceColors[module->currentChannelEditingIx];
			currPlayPattern = module->currentPatternPlayingIx + 1;
			currEditPattern = module->currentPatternEditingIx + 1;
			currentGate = module->currentChannelEditingIx + 1;
			currentNSteps = module->currentNumberSteps;
			currentBPM = module->currentBPM;
		}

		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.5);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);

		int y1 = 42;
		int y2 = 27;
		int dx = 0;
		int x = 0;
		int spacing = 61;

		nvgTextAlign(vg, NVG_ALIGN_CENTER);

		// Current Playing Pattern
		nvgFillColor(vg, textColor);
		x = 5 + 21;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y1, "PATT", NULL);
		sprintf(messageStr, "%02d", currPlayPattern);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);
		nvgText(vg, x + dx, y2, messageStr, NULL);

		// Current Playing Speed
		nvgFillColor(vg, textColor);
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		if (isPreview)
			sprintf(messageStr, "BPM/%s", BPMOptions[1]->label);
		else
			sprintf(messageStr, "BPM/%s", BPMOptions[module->selectedBPMNoteIx]->label);
		nvgText(vg, x, y1, messageStr, NULL);
		if (module->lastStepWasExternalClock)
		{
			sprintf(messageStr, "%s", "CLK");
		}
		else
		{
			sprintf(messageStr, "%03.0f", currentBPM);
		}
		nvgFontFaceId(vg, font->handle);
		nvgFontSize(vg, fontSize * 1.5); // Large font		
		nvgText(vg, x + dx, y2, messageStr, NULL);


		// Current Playing # Steps
		nvgFillColor(vg, textColor);
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y1, "LENG", NULL);
		sprintf(messageStr, "%02d", currentNSteps);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);
		nvgText(vg, x + dx, y2, messageStr, NULL);

		// Current Mode:
		nvgFillColor(vg, nvgRGB(0xda, 0xda, 0xda));
		x += spacing + 5;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y1, "MODE", NULL);
		nvgFontSize(vg, fontSize);	// Small font
		if (module->modeString != NULL)
		{
			nvgFontFaceId(vg, font->handle);
			//nvgText(vg, x + dx -6, y2, module->modeString, NULL);					
			nvgText(vg, x + dx, y2, module->modeString, NULL);
		}

		nvgTextAlign(vg, NVG_ALIGN_CENTER);

		// Current Edit Pattern
		nvgFillColor(vg, textColor);
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y1, "PATT", NULL);
		sprintf(messageStr, "%02d", currEditPattern);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);
		nvgText(vg, x + dx, y2, messageStr, NULL);

		// Current Edit Gate/Trigger
		nvgFillColor(vg, currColor); // Match the Gate/Trigger color
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y1, "CHNL", NULL);
		sprintf(messageStr, "%02d", currentGate);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);
		nvgText(vg, x + dx, y2, messageStr, NULL);

		// [[[[[[[[[[[[[[[[ EDIT Box Group ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
		nvgTextAlign(vg, NVG_ALIGN_LEFT);
		NVGcolor groupColor = nvgRGB(0xDD, 0xDD, 0xDD);
		nvgFillColor(vg, groupColor);
		int labelX = 297;
		x = labelX; // 289
		nvgFontSize(vg, fontSize - 5); // Small font
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, 8, "EDIT", NULL);

		// Edit Label Line ---------------------------------------------------------------
		nvgBeginPath(vg);
		// Start top to the left of the text "Edit"
		int y = 5;
		nvgMoveTo(vg, /*start x*/ x - 3, /*start y*/ y);// Starts new sub-path with specified point as first point.s
		x = 256;// x - 35;//xOffset + 3 * spacing - 3 + 60;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to Left (Line Start)

		x = labelX + 22;
		y = 5;
		nvgMoveTo(vg, /*x*/ x, /*y*/ y); // Right of "Edit"
		x = box.size.x - 6;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // RHS of box

		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, groupColor);
		nvgStroke(vg);

		// [[[[[[[[[[[[[[[[ PLAYBACK Box Group ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
		groupColor = nvgRGB(0xEE, 0xEE, 0xEE);
		nvgFillColor(vg, groupColor);
		labelX = 64;
		x = labelX;
		nvgFontSize(vg, fontSize - 5); // Small font
		nvgText(vg, x, 8, "PLAYBACK", NULL);

		// Play Back Label Line ---------------------------------------------------------------
		nvgBeginPath(vg);
		// Start top to the left of the text "Play"
		y = 5;
		nvgMoveTo(vg, /*start x*/ x - 3, /*start y*/ y);// Starts new sub-path with specified point as first point.s
		x = 6;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to the left

		x = labelX + 52;
		y = 5;
		nvgMoveTo(vg, /*x*/ x, /*y*/ y); // To the Right of "Playback"
		x = 165; //x + 62 ;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go Right 

		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, groupColor);
		nvgStroke(vg);
		return;
	} // end draw()
}; // end struct TSSeqDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqLabelArea
// Draw labels on our sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSeqLabelArea : TransparentWidget {
	TSSequencerModuleBase *module;
	std::shared_ptr<Font> font;
	int fontSize;
	bool drawGridLines = false;
	char messageStr[TROWA_DISP_MSG_SIZE];
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSeqLabelArea()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSeqLabelArea() {
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 13;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void draw(NVGcontext *vg) override {
		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(vg, textColor);
		nvgFontSize(vg, fontSize);

		/// MAKE LABELS HERE
		int x = 45;
		int y = 163;
		int dy = 28;

		// Selected Pattern Playback:
		nvgText(vg, x, y, "PAT", NULL);

		// Clock		
		y += dy;
		nvgText(vg, x, y, "BPM ", NULL);

		// Steps
		y += dy;
		nvgText(vg, x, y, "LNG", NULL);

		// Ext Clock 
		y += dy;
		nvgText(vg, x, y, "CLK", NULL);

		// Reset
		y += dy;
		nvgText(vg, x, y, "RST", NULL);

		// Outputs
		nvgFontSize(vg, fontSize * 0.95);
		x = 320;
		y = 350;
		nvgText(vg, x, y, "OUTPUTS", NULL);

		// TINY btn labels
		nvgFontSize(vg, fontSize * 0.6);
		// OSC Labels
		y = 103;
		if (module->allowOSC)
		{
			x = 240;
			nvgText(vg, x, y, "OSC", NULL);
		}
		// Copy button labels:
		x = 302;
		nvgText(vg, x, y, "CPY", NULL);
		x = 362;
		nvgText(vg, x, y, "CPY", NULL);
		// BPM divisor/note label:
		x = 118;
		nvgText(vg, x, y, "DIV", NULL);


		if (drawGridLines)
		{
			NVGcolor gridColor = nvgRGB(0x44, 0x44, 0x44);
			nvgBeginPath(vg);
			x = 80;
			y = 228;
			nvgMoveTo(vg, /*start x*/ x, /*start y*/ y);// Starts new sub-path with specified point as first point
			x += 225;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to the left

			nvgStrokeWidth(vg, 1.0);
			nvgStrokeColor(vg, gridColor);
			nvgStroke(vg);


			// Vertical
			nvgBeginPath(vg);
			x = 192;
			y = 116;
			nvgMoveTo(vg, /*start x*/ x, /*start y*/ y);// Starts new sub-path with specified point as first point
			y += 225;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to the left			

			nvgStrokeWidth(vg, 1.0);
			nvgStrokeColor(vg, gridColor);
			nvgStroke(vg);

		}

		return;
	} // end draw()
}; // end struct TSSeqLabelArea

#endif