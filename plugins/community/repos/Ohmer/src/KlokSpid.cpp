//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// KlokSpid is a 8 HP module, initially designed to divide/multiply an external clock frequency (clock  ////
//// modulator), but it can work as standalone (BPM-based) clock generator, too!                          ////
//// - 2 input jacks:                                                                                     ////
////   - external clock (CLK), to work as divider/multiplier (standalone clock generator if not patched). ////
////   - multipurpose CV-RATIO/TRIG. jack:                                                                ////
////     - when running as clock multiplier/divider: CV-controllable ratio (full range /64 to x64).       ////
////     - when running as BPM-clock generator: trigger input (BPM start/stop, or BPM reset).             ////
//// - 4 output jacks: gates (default +5V/0V Square waveform). Other gates (%) and 1ms/2ms/5ms triggers   ////
////   (fixed duration pulses) are possible, via SETUP.                                                   ////
////                                                                                                      ////
//// As standalone (BPM-based) clock generator only:                                                      ////
//// - any jack may have its custom ratio (via SETUP) - default is x1, for all jacks.                     ////
//// - when set as "Custom" for first time, proposed default ratios are, per jack: /4, x1, x2, and x4.    ////
//// - jack #4 can be set (via SETUP) to send LFO-based waveform (instead of square/pulse) @ x1 only.     ////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"
#include <dsp/digital.hpp>
#include <string>

namespace rack_plugin_Ohmer {

// Dedicated LFO (based on LFO-1 stuff from Fundamental, but simplified as required).
// It will be used - if enabled via SETUP - to output specific waveform to jack #4. Disabled by default.
// LFO can be enabled to jack #4 but only if this jack is set at default ratio x1.

struct LFO {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool offset = false;
	bool invert = false;

	LFO() {}

	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}

	float sin() {
		if (offset)
			return 1.0f - cosf(2*M_PI * phase) * (invert ? -1.0f : 1.0f);
			else return sinf(2.0f*M_PI * phase) * (invert ? -1.0f : 1.0f);
	}

	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}

	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5f : phase);
			else return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
	}

	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}

	float saw() {
		if (offset)
			return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
			else return saw(phase) * (invert ? -1.0f : 1.0f);
	}

};


// KlokSpid module architecture.
struct KlokSpidModule : Module {
	enum ParamIds {
		PARAM_ENCODER,
    PARAM_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_CLOCK,
		INPUT_CV_TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_CLK,
		LED_CV_TRIG,
		LED_CVMODE,
		LED_TRIGMODE,
		LED_SYNC_GREEN,
		LED_SYNC_RED,
		NUM_LIGHTS
	};

	// Pointer to encoder (handled as knob).
	Knob *klokspdEncoder;

	// Optional LFO for jack #4.
	LFO LFOjack4;

	// Module interface definitions, such parameters (encoder, button), input ports, output ports and LEDs.
	KlokSpidModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		onRandomize();
	}

	void onReset() override {
		if (!avoidOnResetReentry) {
			encoderPrevious = 0;
			onFirstInitCounter = (long)(engineGetSampleRate() / 4.0f);
			currentStep = 0;
			avoidOnResetReentry = true;
		}
	}

	// Inhibit "Randomize" from context-menu / Ctrl+R / Command+R keyboard shortcut over module.
	void onRandomize() override {
	}

	//// GENERAL PURPOSE VARIABLES/FLAGS/TABLES.
	int onFirstInitCounter = 0;
	bool avoidOnResetReentry = false; // When true, this avoid OnReset() reentry.

	//// CLOCK MODULATOR RATIOS.

	// Real clock ratios (global) list/array. Preset ratios while KlokSpid module runs as clock modulator (can be selected via encoder exclusively).
	float list_fRatio[31] = {64.0f, 32.0f, 24.0f, 16.0f, 15.0f, 12.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.5f, 1.0f/3.0f, 0.25f, 0.2f, 1.0f/6.0f, 1.0f/7.0f, 0.125f, 1.0f/9.0f, 0.1f, 1.0f/12.0f, 1.0f/15.0f, 0.0625f, 1.0f/24.0f, 0.03125f, 0.015625f};

	//// MODEL (GUI THEME).

	// Current selected KlokSpid model (GUI theme).
	int Theme = 0;
	// DMD-font color (default is "Classic" beige model).
	NVGcolor DMDtextColor = nvgRGB(0x08, 0x08, 0x08);

	//// Main DMD and small displays (near output jacks).
	char dmdMessage1[24] = "";
	char dmdMessage2[24] = "";
	int xOffsetValue = 0; // Horizontal offset on DMD to display for second line.
	char dispOut1[4] = "";
	int xdispOutOffset1 = 0;
	char dispOut2[4] = "";
	int xdispOutOffset2 = 0;
	char dispOut3[4] = "";
	int xdispOutOffset3 = 0;
	char dispOut4[4] = "";
	int xdispOutOffset4 = 0;

	// Strings for running modes.
	const std::string runningMode[3] = {"Clk Generator", "Clk Modulator", "Clk CV-Ratio"};

	//// STEP-RELATED (REALTIME) COUNTERS/GAPS.

	// Step related variables: used to determine the frequency of source signal, and when KlokSpid must sends relevant pulses to output(s).
	long long int currentStep = 0;
	long long int previousStep = 0;
	long long int expectedStep = 0;
	long stepGap = 0;
	long stepGapPrevious = 0;
	long long int nextPulseStep[NUM_OUTPUTS] = {0, 0, 0, 0};

	// Current jacks states, voltages on input jacks, and button state.
	bool activeCLK = false;
	bool activeCLKPrevious = true;
	bool activeCV = false;
	bool activeCVPrevious = true;
	float voltageOnCV = 0.0f;
	bool buttonPressed = false;

	// Encoder (registered position to be used on next step for relative move).
	int encoderCurrent = 0;
	int encoderPrevious = 0; // Encoder "absolute" (saved to jSon)...
	int encoderDelta = 0; // 0 if not moved, -1 if counter-clockwise (decrement), 1 if clockwise (increment).

	// Ratio (clock modulator).
	int svRatio = 15; // saved value.
	int rateRatioByEncoder = 15; // Assuming encoder is, by default "centered" to "x1" (= 15).

	// Clock modulator modes.
	enum ClkModModeIds {
		X1,	// work at x1.
		DIV,	// divider mode.
		MULT	// muliplier mode.
	};

	// Clock modulator mode, assuming default is X1.
	int clkModulatorMode = X1;

	//// SCHMITT TRIGGERS.

	// Schmitt trigger to check thresholds on CLK input connector.
	SchmittTrigger CLKInputPort;
	// Schmitt trigger to handle BPM start/stop state (only when KlokSpid is acting as clock generator) via button.
	SchmittTrigger runButton;
	// Schmitt trigger to handle the start/stop toggle button (also used for SETUP to confirm menu/parameter) - via CV/TRIG input port (if configured as "Start/Stop").
	SchmittTrigger runTriggerPort;

	//// RATIO-BY-CV VARIABLES/FLAGS.

	// Incoming CV may be bipolar (true) or unipolar (false).
	bool bipolarCV = true;
	// Is CV used to modulate ratio?
	bool isRatioCVmod = false;
	// Real ratio, given by current CV voltage.
	float rateRatioCV = 0.0f;
	// Real ratio, given by current CV voltage, integer is required only for display into DMD (to avoid "decimals" cosmetic issues, at the right side of DMD!).
	int rateRatioCVi = 0;

	//// BPM-RELATED VARIABLES (STANDALONE CLOCK GENERATOR).

	// Default BPM (when KlokSpid is acting as clock generator). Default is 120 BPM (centered knob).
	int svBPM = 120; // saved value.
	int BPM = 120;
	// Previous registed BPM (when KlokSpid is acting as clock generator), from previous step.
	int previousBPM = 120;

	// Custom jacks ratios (per output jack). By default false, all are X1 (original setting for KlokSpid). True means each jack can receive an optional ratio.
	bool defOutRatios = true;
	int outputRatio[4] = {9, 12, 13, 15};
	int outputRatioInUse[4] = {12, 12, 12, 12};
	float list_outRatiof[25] = {64.0f, 32.0f, 24.0f, 16.0f, 12.0f, 9.0f, 8.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.5f, 1.0f/3.0f, 0.25f, 0.2f, 1.0f/6.0f, 0.125f, 1.0f/9.0f, 1.0f/12.0f, 0.0625f, 1.0f/24.0f, 0.03125f, 0.015625f};

	// Indicates if "CV-RATIO/TRIG." input port (used as trigger, standalone BPM-clock mode only) is a transport trigger (true = toggle start/stop) or reset (false, default) useful for "re-sync" between modules.
	bool transportTrig = false;
	// Standalone clock generator mode only: indicates if BPM is running or stopped.
	bool isBPMRunning = true;
	bool runBPMOnInit = true;

	//// SETUP-RELATED VARIABLES/TABLES.

	// Enumeration of SETUP menu entries.
	enum setupMenuEntries {
		SETUP_WELCOME_MESSAGE,	// SETUP menu entry for #0 is always dedicated to welcome message ("*- SETUP -*") displayed on DMD.
		SETUP_CVPOLARITY,	// SETUP menu entry for CV polarity (bipolar, or unipolar).
		SETUP_DURATION,	// SETUP menu entry for pulse duration (fixed and gate-based parameters).
		SETUP_OUTVOLTAGE,	// SETUP menu entry for output voltage.
		SETUP_OUTSRATIOS, // SETUP menu entry for custom ratio concerning all jacks (all at x1, or custom).
		SETUP_OUT1RATIO, // SETUP menu entry for custom ratio concerning output jack #1.
		SETUP_OUT2RATIO, // SETUP menu entry for custom ratio concerning output jack #2.
		SETUP_OUT3RATIO, // SETUP menu entry for custom ratio concerning output jack #3.
		SETUP_OUT4RATIO, // SETUP menu entry for custom ratio concerning output jack #4.
		SETUP_OUT4LFO,	// SETUP menu entry for LFO to output jack #4.
		SETUP_OUT4LFOPOLARITY,	// SETUP menu entry for LFO polarity to output jack #4 (bipolar, or unipolar).
		SETUP_CVTRIG,	// SETUP menu entry describing how CV/TRIG input port is working (as start/stop toggle, or as "RST" input).
		SETUP_EXIT,	// Lastest menu entry is always used to exit SETUP (options are "Save/Exit", "Canc/Exit", "Review" or "Factory").
		NUM_SETUP_ENTRIES // This position indicates how many entries the KlokSpid's SETUP menu have.
	};

	// Strings for SETUP entries.
	const std::string setupMenuName[NUM_SETUP_ENTRIES] = {"*-SETUP-*", "CV Polarity", "Pulse Durat.", "Out. Voltage", "Outp. Ratios", "Out. 1 Ratio", "Out. 2 Ratio", "Out. 3 Ratio", "Out. 4 Ratio", "Out. 4 LFO", "LFO Polarity", "TRIG. Jack", "Exit SETUP"};
	// Strings for SETUP possible parameters.
	std::string setupParamName[NUM_SETUP_ENTRIES][25];
	// Related horizontal offsets (second line of DMD).
	int setupParamXOffset[NUM_SETUP_ENTRIES][25];

	// This flag indicates if KlokSpid module is currently running SETUP, or not.
	bool isSetupRunning = false;
	// This flag indicates if KlokSpid module is entering SETUP (2 seconds delay), or not.
	bool isEnteringSetup = false;
	// This flag indicates if KlokSpid module is exiting SETUP (2 seconds delay), or not.
	bool isExitingSetup = false;
	// This flag is designed to avoid continuous SETUP entries/exits while button is continously held.
	bool allowedButtonHeld = false;
	// Item index (edited parameter number).
	int setup_ParamIdx = 0;
	// Current edited value for selected parameter.
	int setup_CurrentValue = 0;
	// Table containing number of possible values for each parameter.
	int setup_NumValue[NUM_SETUP_ENTRIES] = {0, 2, 9, 4, 2, 25, 25, 25, 25, 7, 2, 2, 4};
	// Default factory values for each parameter.
	int setup_Factory[NUM_SETUP_ENTRIES] = {0, 0, 5, 0, 0, 9, 12, 13, 15, 0, 0, 1, 1};
	// Table containing current values for all parameters.
	int setup_Current[NUM_SETUP_ENTRIES] = {0, 0, 5, 0, 0, 9, 12, 13, 15, 0, 0, 1, 1};
	// Table containing edited parameters during SETUP (will be filled when entering SETUP).
	int setup_Edited[NUM_SETUP_ENTRIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
	// Table containing backup edited parameters during SETUP (will be filled when entering SETUP).
	int setup_Backup[NUM_SETUP_ENTRIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
	// Counter (as "delay") used to enter and (optionally) to saved/exit SETUP quickly on long press.
	long setupCounter = 0;

	//// PULSE TO OUTPUT RELATED VARIABLES AND PULSE GENERATORS.

	// Enumeration of possible pulse durations: fixed 1 ms, fixed 2 ms, fixed 5 ms, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%.
	enum PulseDurations {
		FIXED1MS,	// Fixed 1 ms.
		FIXED2MS,	// Fixed 2 ms.
		FIXED5MS,	// Fixed 5 ms.
		GATE25,	// Gate 1/4 (25%).
		GATE33,	// Gate 1/3 (33%).
		SQUARE,	// Square waveform.
		GATE66,	// Gate 2/3 (66%).
		GATE75,	// Gate 3/4 (75%).
		GATE95,	// Gate 95%.
	};

	// Pulse counter for divider mode (set at max divider value, minus 1).
	int pulseDivCounter[NUM_OUTPUTS] = {63, 63, 63, 63};
	// Pulse counter for multiplier mode, to avoid continuous pulse when no more receiving (set at max divider value, minus 1). Kind of "timeout".
	int pulseMultCounter[NUM_OUTPUTS] = {0, 0, 0, 0};
	// Pulse generators, to send "pulses" to output jacks (one pulse generator per output jack).
	PulseGenerator sendPulse[NUM_OUTPUTS];
	// These flags are related to pulse generators (current pulse state).
	bool sendingOutput[NUM_OUTPUTS] = {false, false, false, false};
	// This flag indicates if sending pulse (one per output jack) is allowed (true) or not (false).
	bool canPulse[NUM_OUTPUTS] = {false, false, false, false};
	// Current pulse duration (time in second). Default is fixed 1 ms at start. Operational can be changed via SETUP.
	float pulseDuration[NUM_OUTPUTS] = {0.001f, 0.001f, 0.001f, 0.001f};
	// Extension of "pulseDuration" value (for square and gate modes), set as square wave (50 %) by default, can be changed via SETUP.
	int pulseDurationExt = SQUARE;
	// Voltage for outputs (pulses/gates), default is +5V, can be changed to +10V, +12V (+11.7V) or +2V instead, via SETUP.
	float outVoltage = 5.0f;
	// Special LFO output on jack #4.
	int jack4LFO = 0;
	bool jack4LFObipolar = true;
	bool resetPhase = true;

	//////////////////////////////////////
	//// CLK LED AFTERGLOW.           ////
	//////////////////////////////////////

	// Counter used for red CLK LED afterglow (used together with "ledClkAfterglow" boolean flag).
	long ledClkDelay = 0; // long is required for highest engine samplerates!
	// This flag controls CLK (red) LED afterglow (active or not).
	bool ledClkAfterglow = false;

	//////////////////////////////////////
	//// SYNC STATUS.                 ////
	//////////////////////////////////////

	// Assuming clock generator isn't synchronized (sync'd) with source clock on initialization.
	bool isSync = false;

	//////////////////////////////////////
	//// FUNCTIONS & METHODS (VOIDS). ////
	//////////////////////////////////////

	void step() override;

	// Convert a string to char pointer (char*).
	char* stringToPchar(std::string str) {
		char *cstr = new char[str.length() + 1];
		strcpy(cstr, str.c_str());
		return cstr;
		delete [] cstr;
	}

	void updateDisplayJack(int jackID) {
		if (activeCLK) {
			// Clock modulator mode. For now, all ports are at x1.
			xdispOutOffset1 = 5;
			strcpy(dispOut1, stringToPchar("x1"));
			xdispOutOffset2 = 5;
			strcpy(dispOut2, stringToPchar("x1"));
			xdispOutOffset3 = 5;
			strcpy(dispOut3, stringToPchar("x1"));
			xdispOutOffset4 = 5;
			strcpy(dispOut4, stringToPchar("x1"));
		}
		else {

			// Clock generator mode.
			switch (jackID) {
				case 0:
					xdispOutOffset1 = 0;
					if ((outputRatioInUse[0] > 4) && (outputRatioInUse[0] < 12))
						xdispOutOffset1 = 4;
						else if ((outputRatioInUse[0] > 11) && (outputRatioInUse[0] < 20))
							xdispOutOffset1 = 5;
							else if (outputRatioInUse[0] > 19)
								 xdispOutOffset2 = 1;
					strcpy(dispOut1, stringToPchar(setupParamName[SETUP_OUT1RATIO][outputRatioInUse[0]]));
					break;
				case 1:
					xdispOutOffset2 = 0;
					if ((outputRatioInUse[1] > 4) && (outputRatioInUse[1] < 12))
						xdispOutOffset2 = 4;
						else if ((outputRatioInUse[1] > 11) && (outputRatioInUse[1] < 20))
							xdispOutOffset2 = 5;
							else if (outputRatioInUse[1] > 19)
								 xdispOutOffset2 = 1;
					strcpy(dispOut2, stringToPchar(setupParamName[SETUP_OUT2RATIO][outputRatioInUse[1]]));
					break;
				case 2:
					xdispOutOffset3 = 0;
					if ((outputRatioInUse[2] > 4) && (outputRatioInUse[2] < 12))
						xdispOutOffset3 = 4;
						else if ((outputRatioInUse[2] > 11) && (outputRatioInUse[2] < 20))
							xdispOutOffset3 = 5;
							else if (outputRatioInUse[2] > 19)
								 xdispOutOffset3 = 1;
					strcpy(dispOut3, stringToPchar(setupParamName[SETUP_OUT3RATIO][outputRatioInUse[2]]));
					break;
				case 3:
					if (outputRatioInUse[3] == 12) {
						if (jack4LFO != 0) {
							xdispOutOffset4 = 0;
							switch (jack4LFO) {
								case 1:
								case 2:
									strcpy(dispOut4, stringToPchar("SIN"));
									break;
								case 3:
								case 4:
									strcpy(dispOut4, stringToPchar("TRI"));
									break;
								case 5:
								case 6:
									strcpy(dispOut4, stringToPchar("SAW"));
							}
						}
						else {
							xdispOutOffset4 = 5;
							strcpy(dispOut4, stringToPchar("x1"));
						}
					}
					else {
						xdispOutOffset4 = 0;
						if ((outputRatioInUse[3] > 4) && (outputRatioInUse[3] < 12))
							xdispOutOffset4 = 4;
							else if ((outputRatioInUse[3] > 11) && (outputRatioInUse[3] < 20))
								xdispOutOffset4 = 5;
								else if (outputRatioInUse[3] > 19)
									 xdispOutOffset4 = 1;
						strcpy(dispOut4, stringToPchar(setupParamName[SETUP_OUT4RATIO][outputRatioInUse[3]]));
					}
					break;
			}
		}
	}

	// Set the DMD, regarding current mode (0 = BPM generator, 1 = clock modulator by encoder).
	void updateDMDtoRunningMode(int currMode) {
		// Update small displays for each output jacks.
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
			updateDisplayJack(i);
		switch (currMode) {
			case 0:
				// BPM clock generator.
				// Update main DMD.
				if (!isSetupRunning) {
					strcpy(dmdMessage1, stringToPchar(runningMode[0]));
					if (BPM < 10)
						xOffsetValue = 19;
						else if (BPM < 100)
							xOffsetValue = 13;
							else xOffsetValue = 7;
					strcpy(dmdMessage2, stringToPchar(std::to_string(BPM) + " BPM"));
				}
				break;
			case 1:
				// Clock modulator.
				if (inputs[INPUT_CV_TRIG].active) {
					// Ratio is modulated by CV.
					voltageOnCV = inputs[INPUT_CV_TRIG].value;
					if (bipolarCV)
						rateRatioCV = round(clamp(static_cast<float>(voltageOnCV), -5.0f, 5.0f) * 12.6f); // By bipolar voltage (-5V/+5V).
						else rateRatioCV = round((clamp(static_cast<float>(voltageOnCV), 0.0f, 10.0f) - 5.0f) * 12.6f); // By unipolar voltage (0V/+10V).
					// Required to display ratio without artifacts!
					rateRatioCVi = static_cast<int>(rateRatioCV);
					if (round(rateRatioCV) == 0.0f) {
						clkModulatorMode = X1;
						rateRatioCV = 1.0f; // Real ratio becomes... 1.0f because it's multiplied by 1.
					}
					else if (round(rateRatioCV) > 0.0f) {
						clkModulatorMode = MULT;
						rateRatioCV = round(rateRatioCV + 1.0f);
					}
					else {
						clkModulatorMode = DIV;
						rateRatioCV = 1.0f / round(1.0f - rateRatioCV);
					}
					if (!isSetupRunning) {
						// Clock modulator (free ratio by CV).
						strcpy(dmdMessage1, stringToPchar(runningMode[2]));
						xOffsetValue = 2;
						std::string sSign = "x";
						if (rateRatioCVi >= 0) {
							rateRatioCVi = rateRatioCVi + 1;
						}
						else {
							rateRatioCVi = 1 - rateRatioCVi;
							sSign = "/";
						}
						strcpy(dmdMessage2, stringToPchar("Rate: " + sSign + std::to_string(rateRatioCVi)));
					}
				}
				else {
					// Clock modulator (preset ratio by encoder).
					// Ratio is selected from encoder.
					if (!isSetupRunning) {
						// Related multiplier/divider mode.
						clkModulatorMode = DIV;
						if (rateRatioByEncoder == 15)
							clkModulatorMode = X1;
							else if (rateRatioByEncoder > 15)
								clkModulatorMode = MULT;
						static const int list_iRatio[31] = {64, 32, 24, 16, 15, 12, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 24, 32, 64};
						strcpy(dmdMessage1, stringToPchar(runningMode[1]));
						xOffsetValue = 2;
						std::string sSign = "x";
						if (rateRatioByEncoder < 15)
							sSign = "/";
						strcpy(dmdMessage2, stringToPchar("Rate: " + sSign + std::to_string(list_iRatio[rateRatioByEncoder])));
					}
				}
		}
	}

	// This custom function applies current settings (useful after SETUP operation, also "on-the-fly" altered parameter during Setup - useful to experiment).
	void UpdateKlokSpidSettings(bool allowJsonUpdate) {
		// SETUP parameter SETUP_CVPOLARITY: CV polarity (bipolar or unipolar CV-Ratio).
		bipolarCV = (setup_Current[SETUP_CVPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		if (allowJsonUpdate)
			this->bipolarCV = (setup_Current[SETUP_CVPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_DURATION: possible pulse durations (1 ms, 2 ms, 5 ms, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%). Keept for compatibility with v0.5.2 .vcv patches!
		switch (setup_Current[SETUP_DURATION]) {
			case FIXED1MS:
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					pulseDuration[i] = 0.001f;
				break;
			case FIXED2MS:
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					pulseDuration[i] = 0.002f;
				break;
			case FIXED5MS:
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					pulseDuration[i] = 0.005f;
		}
		// Extension for pulse duration parameter (it's a kind of "descriptor" for non-fixed durations).
		pulseDurationExt  = setup_Current[SETUP_DURATION];
		if (allowJsonUpdate)
			this->pulseDurationExt  = setup_Current[SETUP_DURATION]; // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_OUTVOLTAGE: output voltage: +2V, +5V, +10V or +12V (+11.7V).
		switch (setup_Current[SETUP_OUTVOLTAGE]) {
			case 0:
				outVoltage = 5.0f;
				if (allowJsonUpdate)
					this->outVoltage = 5.0f; // First setting is +5V, also factory (default) setting. json persistence (only if SETUP isn't running).
				break;
			case 1:
				outVoltage = 10.0f;
				if (allowJsonUpdate)
					this->outVoltage = 10.0f; // Second setting is +10V. json persistence (only if SETUP isn't running).
				break;
			case 2:
				outVoltage = 11.7f;
				if (allowJsonUpdate)
					this->outVoltage = 11.7f; // Third setting is +12V (real +11.7 V). json persistence (only if SETUP isn't running).
				break;
			case 3:
				outVoltage = 2.0f;
				if (allowJsonUpdate)
					this->outVoltage = 2.0f; // Last setting (introduced from v0.5.5/v0.6.0.4-beta): +2V. json persistence (only if SETUP isn't running).
		}
		// SETUP parameter SETUP_OUTSRATIOS: all output jacks at default x1, or custom (useful to bypass all 4 jack rations during SETUP, if let at default).
		defOutRatios = (setup_Current[SETUP_OUTSRATIOS] == 0); // json persistence (only if SETUP isn't running).
		if (allowJsonUpdate)
			this->defOutRatios = (setup_Current[SETUP_OUTSRATIOS] == 0); // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_OUT1RATIO: optional BPM rate applied on output jack #1.
		outputRatio[0] = setup_Current[SETUP_OUT1RATIO];
		if (allowJsonUpdate)
			this->outputRatio[0] = setup_Current[SETUP_OUT1RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[0] = 12;
			else outputRatioInUse[0] = outputRatio[0];
		// SETUP parameter SETUP_OUT2RATIO: optional BPM rate applied on output jack #2.
		outputRatio[1] = setup_Current[SETUP_OUT2RATIO];
		if (allowJsonUpdate)
			this->outputRatio[1] = setup_Current[SETUP_OUT2RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[1] = 12;
			else outputRatioInUse[1] = outputRatio[1];
		// SETUP parameter SETUP_OUT3RATIO: optional BPM rate applied on output jack #3.
		outputRatio[2] = setup_Current[SETUP_OUT3RATIO];
		if (allowJsonUpdate)
			this->outputRatio[2] = setup_Current[SETUP_OUT3RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[2] = 12;
			else outputRatioInUse[2] = outputRatio[2];
		// SETUP parameter SETUP_OUT4RATIO: optional BPM rate applied on output jack #4.
		outputRatio[3] = setup_Current[SETUP_OUT4RATIO];
		if (allowJsonUpdate)
			this->outputRatio[3] = setup_Current[SETUP_OUT4RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[3] = 12;
			else outputRatioInUse[3] = outputRatio[3];
		// SETUP parameter SETUP_OUT4LFO: optional LFO on output jack #4: Disabled, Sine, Triangle, Saw, Inverse Sine, Inverse Triangle, Inverse Saw.
		// Introduced from v0.6.1, but remaining to do.
		jack4LFO = setup_Current[SETUP_OUT4LFO];
		if (allowJsonUpdate)
			this->jack4LFO = setup_Current[SETUP_OUT4LFO]; // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_OUT4LFOPOLARITY: LFO polarity (bipolar or unipolar).
		jack4LFObipolar = (setup_Current[SETUP_OUT4LFOPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		if (allowJsonUpdate)
			this->jack4LFObipolar = (setup_Current[SETUP_OUT4LFOPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_CVTRIG: CV-RATIO/TRIG. input port behavior (standalone clock generator only, this port is TRIG.).
		// - "true" is meaning the TRIG. input port acts as "start/stop toggle".
		// - "false" is meaning the TRIG. input port acts as "BPM reset" (useful to "re-sync" BPM from an external/reference source clock, for example).
		transportTrig = (setup_Current[SETUP_CVTRIG] == 0);
		if (allowJsonUpdate)
			this->transportTrig = (setup_Current[SETUP_CVTRIG] == 0); // json persistence (only if SETUP isn't running).
		// Update small displays for each output jacks.
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
			updateDisplayJack(i);
	}

	// This custom function returns pulse duration (ms), regardling number of samples (long int) and pulsation duration parameter (SETUP).
	float GetPulsingTime(long int stepGap, float rate) {
		float pTime = 0.001; // As default pulse duration is set to 1ms (also can be set to "fixed 1ms" via SETUP).
		if (stepGap == 0) {
			// No reference duration (number of samples is zero).
			switch (setup_Current[SETUP_DURATION]) {
				case FIXED2MS:
					pTime = 0.002f;	// Fixed 2 ms pulse.
					break;
				case FIXED5MS:
					pTime = 0.005f;	// Fixed 5 ms pulse.
			}
		}
		else {
			// Reference duration in number of samples (when known stepGap). Variable-length pulse duration can be defined.
			switch (setup_Current[SETUP_DURATION]) {
				case FIXED2MS:
					pTime = 0.002f;	// Fixed 2 ms pulse.
					break;
				case FIXED5MS:
					pTime = 0.005f;	// Fixed 5 ms pulse.
					break;
				case GATE25:
					pTime = rate * 0.25f * (stepGap / engineGetSampleRate());	// Gate 1/4 (25%)
					break;
				case GATE33:
					pTime = rate * (1.0f / 3.0f) * (stepGap / engineGetSampleRate());	// Gate 1/3 (33%)
					break;
				case SQUARE:
					pTime = rate * 0.5f * (stepGap / engineGetSampleRate());	// Square wave (50%)
					break;
				case GATE66:
					pTime = rate * (2.0f / 3.0f) * (stepGap / engineGetSampleRate());	// Gate 2/3 (66%)
					break;
				case GATE75:
					pTime = rate * 0.75f * (stepGap / engineGetSampleRate());	// Gate 3/4 (75%)
					break;
				case GATE95:
					pTime = rate * 0.95f * (stepGap / engineGetSampleRate());	// Gate 95%
			}
		}
		return pTime;
	}

	// Persistence for extra datas via json functions (in particular parameters defined via KlokSpid's SETUP, also BPM state).
	// These extra datas are saved to .vcv file (including "autosave.vcv") also are "transfered" when you duplicate the module.

	json_t *toJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		json_object_set_new(rootJ, "bipolarCV", json_boolean(bipolarCV));
		json_object_set_new(rootJ, "pulseDurationExt", json_integer(pulseDurationExt));
		json_object_set_new(rootJ, "outVoltage", json_real(outVoltage));
		json_object_set_new(rootJ, "defOutRatios", json_boolean(defOutRatios)); // When true, all jacks are x1. Otherwise (false) any jack may have its specific ratio.
		json_object_set_new(rootJ, "out1Ratio", json_integer(outputRatio[0]));
		json_object_set_new(rootJ, "out2Ratio", json_integer(outputRatio[1]));
		json_object_set_new(rootJ, "out3Ratio", json_integer(outputRatio[2]));
		json_object_set_new(rootJ, "out4Ratio", json_integer(outputRatio[3]));
		json_object_set_new(rootJ, "jack4LFO", json_integer(jack4LFO));
		json_object_set_new(rootJ, "jack4LFObipolar", json_boolean(jack4LFObipolar));
		json_object_set_new(rootJ, "transportTrig", json_boolean(transportTrig)); // CV-RATIO/TRIG. port may be used as BPM "start/stop" toggle or as BPM-reset. BPM-reset is default factory (false).
		json_object_set_new(rootJ, "Ratio", json_integer(rateRatioByEncoder)); // Ratio set by encoder.
		json_object_set_new(rootJ, "BPM", json_integer(BPM)); // BPM set by encoder.
		json_object_set_new(rootJ, "runBPMOnInit", json_boolean(runBPMOnInit)); // State of BPM pulsing or stopped.
		return rootJ;
	}

	void fromJson(json_t *rootJ) override	{
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
		// Retrieving bipolar or unipolar mode (for CV when running as clock multiplier/divider).
		json_t *bipolarCVJ = json_object_get(rootJ, "bipolarCV");
		if (bipolarCVJ)
			bipolarCV = json_is_true(bipolarCVJ);
		// Retrieving pulse duration "mode" data. Introducted since v0.5.3. Thanks Yoann for this idea!
		json_t *pulseDurationExtJ = json_object_get(rootJ, "pulseDurationExt");
		if (pulseDurationExtJ)
			pulseDurationExt = json_integer_value(pulseDurationExtJ);
		// Retrieving output voltage data (real/float value).
		json_t *outVoltageJ = json_object_get(rootJ, "outVoltage");
		if (outVoltageJ)
			outVoltage = json_real_value(outVoltageJ);
		// Retrieving if ratio par jack is disabled (all at x1), or enabled (each having its ratio).
		json_t *defOutRatiosJ = json_object_get(rootJ, "defOutRatios");
		if (defOutRatiosJ)
			defOutRatios = json_is_true(defOutRatiosJ);
		// Retrieving ratio for output jack #1 (when loading .vcv and cloning module).
		json_t *jack1BPMRateJ = json_object_get(rootJ, "out1Ratio");
		if (jack1BPMRateJ)
			outputRatio[0] = json_integer_value(jack1BPMRateJ);
		// Retrieving ratio for output jack #2 (when loading .vcv and cloning module).
		json_t *jack2BPMRateJ = json_object_get(rootJ, "out2Ratio");
		if (jack2BPMRateJ)
			outputRatio[1] = json_integer_value(jack2BPMRateJ);
		// Retrieving ratio for output jack #3 (when loading .vcv and cloning module).
		json_t *jack3BPMRateJ = json_object_get(rootJ, "out3Ratio");
		if (jack3BPMRateJ)
			outputRatio[2] = json_integer_value(jack3BPMRateJ);
		// Retrieving ratio for output jack #4 (when loading .vcv and cloning module).
		json_t *jack4BPMRateJ = json_object_get(rootJ, "out4Ratio");
		if (jack4BPMRateJ)
			outputRatio[3] = json_integer_value(jack4BPMRateJ);
		// Retrieving output jack #4 LFO mode (when loading .vcv and cloning module).
		json_t *jack4LFOJ = json_object_get(rootJ, "jack4LFO");
		if (jack4LFOJ)
			jack4LFO = json_integer_value(jack4LFOJ);
		// Retrieving bipolar or unipolar for jack #4 LFO.
		json_t *jack4LFObipolarJ = json_object_get(rootJ, "jack4LFObipolar");
		if (jack4LFObipolarJ)
			jack4LFObipolar = json_is_true(jack4LFObipolarJ);
		// Retrieving usage of TRIG. input port: start/stop toggle (true) or BPM-reset (false).
		json_t *transportTrigJ = json_object_get(rootJ, "transportTrig");
		if (transportTrigJ)
			transportTrig = json_is_true(transportTrigJ);
		// Retrieving ratio (clock modulator) set by encoder (when loading .vcv and cloning module).
		json_t *svRatioJ = json_object_get(rootJ, "Ratio");
		if (svRatioJ)
			svRatio = json_integer_value(svRatioJ);
		// Retrieving BPM (when loading .vcv and cloning module).
		json_t *svBPMJ = json_object_get(rootJ, "BPM");
		if (svBPMJ)
			svBPM = json_integer_value(svBPMJ);
		// Retrieving last saved BPM-clocking state (it was running or stopped).
		json_t *runBPMOnInitJ = json_object_get(rootJ, "runBPMOnInit");
		if (runBPMOnInitJ)
			runBPMOnInit = json_is_true(runBPMOnInitJ);
	}
};


void KlokSpidModule::step() {
	// step() function is the right place for DSP processing!
	// Depending current KlokSpid model (theme), set the relevant DMD-text color.
	DMDtextColor = tblDMDtextColor[Theme];
	// Bypass early steps, due to encoder/knob behaviors on init (from 0.0f... to json saved value!).
	if (onFirstInitCounter > 0) {
		currentStep++;
		// Small displays near output jacks.
		xdispOutOffset1 = 0;
		strcpy(dispOut1, "");
		xdispOutOffset2 = 0;
		strcpy(dispOut2, "");
		xdispOutOffset3 = 0;
		strcpy(dispOut3, "");
		xdispOutOffset4 = 0;
		strcpy(dispOut4, "");
		// Flashing calibration message on DMD.
		if ((currentStep % 8000) > 4000)
			strcpy(dmdMessage1, "Calibrating...");
			else strcpy(dmdMessage1, "");
		xOffsetValue = 2;
		strcpy(dmdMessage2, "");
		// Encoder calibration.
		encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].value);
		if (encoderPrevious != encoderCurrent) {
			onFirstInitCounter = onFirstInitCounter + (int)(engineGetSampleRate() / 512.0f);
			encoderPrevious = encoderCurrent;
		}
		else onFirstInitCounter--;
		// Last step for initialization.
		if (onFirstInitCounter == 1) {
			// This is the lastest step of initialization.
			// Filling table containing current SETUP parameters.
			// SETUP parameter SETUP_CVPOLARITY: bipolar or unipolar CV.
			setup_Current[SETUP_CVPOLARITY] = bipolarCV ? 0 : 1;
			// SETUP parameter SETUP_DURATION: Pulse duration (extended, to keep compatibility with previous v0.5.2).
			// Parameter #2: possible pulse durations (fixed 1 ms, 2 ms or 5 ms durations, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%).
			setup_Current[SETUP_DURATION] = pulseDurationExt;
			switch (pulseDurationExt) {
				case FIXED1MS:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.001f;
					break;
				case FIXED2MS:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.002f;
					break;
				case FIXED5MS:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.005f;
					break;
				default:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.001f; // It's a default value, but gates are defined in realtime (later).
			}
			// If output voltage is above +11V, assuming +11.7V.
			if (round(outVoltage * 10) > 110)
				outVoltage = 11.7f;
			// Assuming +5V is default output voltage.
			setup_Current[SETUP_OUTVOLTAGE] = 0; // +5V.
			// SETUP parameter SETUP_OUTVOLTAGE: Output voltage.
			if (round(outVoltage * 10) == 20)
				setup_Current[SETUP_OUTVOLTAGE] = 3; // +2V. Lastest value (instead of "inserted" at first, to preserve compatibility!).
				else if (round(outVoltage * 10) == 100)
					setup_Current[SETUP_OUTVOLTAGE] = 1; // +10V.
					else if (round(outVoltage * 10) == 117)
						setup_Current[SETUP_OUTVOLTAGE] = 2; // +11.7V (indicated +12V in module's SETUP).
			// SETUP parameter SETUP_OUTSRATIOS: enabled or disabled custom ratios (for all output jacks).
			setup_Current[SETUP_OUTSRATIOS] = defOutRatios ? 0 : 1;
			// SETUP parameter SETUP_OUT1RATIO to SETUP_OUT4RATIO: optional BPM rate applied on output jacks #1~#4.
			for (int i = 0; i < NUM_OUTPUTS; i++) {
				setup_Current[SETUP_OUT1RATIO + i] = outputRatio[i];
				if (defOutRatios)
					outputRatioInUse[i] = 12;
					else outputRatioInUse[i] = outputRatio[i];
			}
			// SETUP parameter SETUP_OUT4LFO: optional LFO on output jack #4.
			setup_Current[SETUP_OUT4LFO] = jack4LFO;
			// SETUP parameter SETUP_OUT4LFOPOLARITY: bipolar or unipolar LFO.
			setup_Current[SETUP_OUT4LFOPOLARITY] = jack4LFObipolar ? 0 : 1;
			// SETUP parameter SETUP_CVTRIG: CV/TRIG port, as trigger input when running as standalone clock generator (only).
			setup_Current[SETUP_CVTRIG] = transportTrig ? 0 : 1;
			// Parameter's value is, by default 1 for default "Save/Exit".
			setup_Current[SETUP_EXIT] = 1;
			// Is standalone clock is running at init, or not (previous state).
			isBPMRunning = this->runBPMOnInit;
			// Strings construction for SETUP.
			// SETUP_WELCOME_MESSAGE (unique option).
			setupParamName[SETUP_WELCOME_MESSAGE][0] = "Press Btn!";
			setupParamXOffset[SETUP_WELCOME_MESSAGE][0] = -1;
			for (int i = 1; i < 22; i++) {
				setupParamName[SETUP_WELCOME_MESSAGE][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_WELCOME_MESSAGE][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_CVPOLARITY: having 2 possible parameters.
			setupParamName[SETUP_CVPOLARITY][0] = "Bipolar";
			setupParamXOffset[SETUP_CVPOLARITY][0] = 2;
			setupParamName[SETUP_CVPOLARITY][1] = "Unipolar";
			setupParamXOffset[SETUP_CVPOLARITY][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_CVPOLARITY][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_CVPOLARITY][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_DURATION: having 9 possible parameters.
			setupParamName[SETUP_DURATION][FIXED1MS] = "Fixed 1ms";
			setupParamXOffset[SETUP_DURATION][FIXED1MS] = 1;
			setupParamName[SETUP_DURATION][FIXED2MS] = "Fixed 2ms";
			setupParamXOffset[SETUP_DURATION][FIXED2MS] = 1;
			setupParamName[SETUP_DURATION][FIXED5MS] = "Fixed 5ms";
			setupParamXOffset[SETUP_DURATION][FIXED5MS] = 1;
			setupParamName[SETUP_DURATION][GATE25] = "Gate 25%";
			setupParamXOffset[SETUP_DURATION][GATE25] = 2;
			setupParamName[SETUP_DURATION][GATE33] = "Gate 33%";
			setupParamXOffset[SETUP_DURATION][GATE33] = 2;
			setupParamName[SETUP_DURATION][SQUARE] = "Square W.";
			setupParamXOffset[SETUP_DURATION][SQUARE] = 2;
			setupParamName[SETUP_DURATION][GATE66] = "Gate 66%";
			setupParamXOffset[SETUP_DURATION][GATE66] = 2;
			setupParamName[SETUP_DURATION][GATE75] = "Gate 75%";
			setupParamXOffset[SETUP_DURATION][GATE75] = 2;
			setupParamName[SETUP_DURATION][GATE95] = "Gate 95%";
			setupParamXOffset[SETUP_DURATION][GATE95] = 2;
			// SETUP_OUTVOLTAGE: having 4 possible parameters.
			setupParamName[SETUP_OUTVOLTAGE][0] = "+5V";
			setupParamXOffset[SETUP_OUTVOLTAGE][0] = 2;
			setupParamName[SETUP_OUTVOLTAGE][1] = "+10V";
			setupParamXOffset[SETUP_OUTVOLTAGE][1] = 2;
			setupParamName[SETUP_OUTVOLTAGE][2] = "+11.7V";
			setupParamXOffset[SETUP_OUTVOLTAGE][2] = 2;
			setupParamName[SETUP_OUTVOLTAGE][3] = "+2V";
			setupParamXOffset[SETUP_OUTVOLTAGE][3] = 2;
			for (int i = 4; i < 22; i++) {
				setupParamName[SETUP_OUTVOLTAGE][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUTVOLTAGE][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_OUTSRATIOS: having 2 possible parameters.
			setupParamName[SETUP_OUTSRATIOS][0] = "All @ x1";
			setupParamXOffset[SETUP_OUTSRATIOS][0] = 2;
			setupParamName[SETUP_OUTSRATIOS][1] = "Custom";
			setupParamXOffset[SETUP_OUTSRATIOS][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_OUTSRATIOS][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUTSRATIOS][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_OUT1RATIO to SETUP_OUT4RATIO: each ratio for any output jack have 25 possible parameters.
			for (int i = 0; i < 4; i++) {
				setupParamName[SETUP_OUT1RATIO + i][0] = "/64";
				setupParamXOffset[SETUP_OUT1RATIO + i][0] = 32;
				setupParamName[SETUP_OUT1RATIO + i][1] = "/32";
				setupParamXOffset[SETUP_OUT1RATIO + i][1] = 32;
				setupParamName[SETUP_OUT1RATIO + i][2] = "/24";
				setupParamXOffset[SETUP_OUT1RATIO + i][2] = 32;
				setupParamName[SETUP_OUT1RATIO + i][3] = "/16";
				setupParamXOffset[SETUP_OUT1RATIO + i][3] = 32;
				setupParamName[SETUP_OUT1RATIO + i][4] = "/12";
				setupParamXOffset[SETUP_OUT1RATIO + i][4] = 32;
				setupParamName[SETUP_OUT1RATIO + i][5] = "/9";
				setupParamXOffset[SETUP_OUT1RATIO + i][5] = 38;
				setupParamName[SETUP_OUT1RATIO + i][6] = "/8";
				setupParamXOffset[SETUP_OUT1RATIO + i][6] = 38;
				setupParamName[SETUP_OUT1RATIO + i][7] = "/6";
				setupParamXOffset[SETUP_OUT1RATIO + i][7] = 38;
				setupParamName[SETUP_OUT1RATIO + i][8] = "/5";
				setupParamXOffset[SETUP_OUT1RATIO + i][8] = 38;
				setupParamName[SETUP_OUT1RATIO + i][9] = "/4";
				setupParamXOffset[SETUP_OUT1RATIO + i][9] = 38;
				setupParamName[SETUP_OUT1RATIO + i][10] = "/3";
				setupParamXOffset[SETUP_OUT1RATIO + i][10] = 38;
				setupParamName[SETUP_OUT1RATIO + i][11] = "/2";
				setupParamXOffset[SETUP_OUT1RATIO + i][11] = 38;
				setupParamName[SETUP_OUT1RATIO + i][12] = "x1";
				setupParamXOffset[SETUP_OUT1RATIO + i][12] = 38;
				setupParamName[SETUP_OUT1RATIO + i][13] = "x2";
				setupParamXOffset[SETUP_OUT1RATIO + i][13] = 38;
				setupParamName[SETUP_OUT1RATIO + i][14] = "x3";
				setupParamXOffset[SETUP_OUT1RATIO + i][14] = 38;
				setupParamName[SETUP_OUT1RATIO + i][15] = "x4";
				setupParamXOffset[SETUP_OUT1RATIO + i][15] = 38;
				setupParamName[SETUP_OUT1RATIO + i][16] = "x5";
				setupParamXOffset[SETUP_OUT1RATIO + i][16] = 38;
				setupParamName[SETUP_OUT1RATIO + i][17] = "x6";
				setupParamXOffset[SETUP_OUT1RATIO + i][17] = 38;
				setupParamName[SETUP_OUT1RATIO + i][18] = "x8";
				setupParamXOffset[SETUP_OUT1RATIO + i][18] = 38;
				setupParamName[SETUP_OUT1RATIO + i][19] = "x9";
				setupParamXOffset[SETUP_OUT1RATIO + i][19] = 38;
				setupParamName[SETUP_OUT1RATIO + i][20] = "x12";
				setupParamXOffset[SETUP_OUT1RATIO + i][20] = 32;
				setupParamName[SETUP_OUT1RATIO + i][21] = "x16";
				setupParamXOffset[SETUP_OUT1RATIO + i][21] = 32;
				setupParamName[SETUP_OUT1RATIO + i][22] = "x24";
				setupParamXOffset[SETUP_OUT1RATIO + i][22] = 32;
				setupParamName[SETUP_OUT1RATIO + i][23] = "x32";
				setupParamXOffset[SETUP_OUT1RATIO + i][23] = 32;
				setupParamName[SETUP_OUT1RATIO + i][24] = "x64";
				setupParamXOffset[SETUP_OUT1RATIO + i][24] = 32;
			}
			// SETUP_OUT4LFO: having 7 possible parameters.
			setupParamName[SETUP_OUT4LFO][0] = "Disabled";
			setupParamXOffset[SETUP_OUT4LFO][0] = 2;
			setupParamName[SETUP_OUT4LFO][1] = "Sine";
			setupParamXOffset[SETUP_OUT4LFO][1] = 2;
			setupParamName[SETUP_OUT4LFO][2] = "Inv. Sine";
			setupParamXOffset[SETUP_OUT4LFO][2] = 2;
			setupParamName[SETUP_OUT4LFO][3] = "Triangle";
			setupParamXOffset[SETUP_OUT4LFO][3] = 2;
			setupParamName[SETUP_OUT4LFO][4] = "Inv. Tri.";
			setupParamXOffset[SETUP_OUT4LFO][4] = 2;
			setupParamName[SETUP_OUT4LFO][5] = "Sawtooth";
			setupParamXOffset[SETUP_OUT4LFO][5] = 2;
			setupParamName[SETUP_OUT4LFO][6] = "Inv. Saw.";
			setupParamXOffset[SETUP_OUT4LFO][6] = 2;
			for (int i = 7; i < 22; i++) {
				setupParamName[SETUP_OUT4LFO][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUT4LFO][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_OUT4LFOPOLARITY: having 2 possible parameters.
			setupParamName[SETUP_OUT4LFOPOLARITY][0] = "Bipolar";
			setupParamXOffset[SETUP_OUT4LFOPOLARITY][0] = 2;
			setupParamName[SETUP_OUT4LFOPOLARITY][1] = "Unipolar";
			setupParamXOffset[SETUP_OUT4LFOPOLARITY][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_OUT4LFOPOLARITY][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUT4LFOPOLARITY][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_CVTRIG: having 2 possible parameters.
			setupParamName[SETUP_CVTRIG][0] = "Play/Stop";
			setupParamXOffset[SETUP_CVTRIG][0] = 2;
			setupParamName[SETUP_CVTRIG][1] = "Reset In.";
			setupParamXOffset[SETUP_CVTRIG][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_CVTRIG][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_CVTRIG][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_EXIT: having 4 possible parameters.
			setupParamName[SETUP_EXIT][0] = "Canc./Exit";
			setupParamXOffset[SETUP_EXIT][0] = -1;
			setupParamName[SETUP_EXIT][1] = "Save/Exit";
			setupParamXOffset[SETUP_EXIT][1] = 1;
			setupParamName[SETUP_EXIT][2] = "Review...";
			setupParamXOffset[SETUP_EXIT][2] = 2;
			setupParamName[SETUP_EXIT][3] = "Factory";
			setupParamXOffset[SETUP_EXIT][3] = 2;
			for (int i = 4; i < 22; i++) {
				setupParamName[SETUP_EXIT][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_EXIT][i] = 0; // Useless x-offsets to 0.
			}
			activeCLK = inputs[INPUT_CLOCK].active;
			activeCLKPrevious = activeCLK;
			activeCV = inputs[INPUT_CV_TRIG].active;
			activeCVPrevious = activeCV;
			// Reinit encoder reading.
			encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].value);
			encoderPrevious = encoderCurrent;
			encoderDelta = 0; // Default assuming encoder isn't moved.
			//
			currentStep = 0;
			rateRatioByEncoder = this->svRatio;
			BPM = this->svBPM;
			if (activeCLK)
				updateDMDtoRunningMode(1);
				else updateDMDtoRunningMode(0);
			onFirstInitCounter = 0;
		}
		return;
	}

	// Current state of CLK port.  Active means connected/wired.
	activeCLK = inputs[INPUT_CLOCK].active;

	// Current state and voltage (CV/TRIG port). Active means connected/wired.
	activeCV = inputs[INPUT_CV_TRIG].active;

	// Encoder behavior (moved or not).
	encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].value);
	encoderDelta = 0; // Default assuming encoder isn't moved.
	if (abs(encoderCurrent - encoderPrevious) <= 2) {
		if (encoderCurrent < encoderPrevious)
			encoderDelta = -1; // Counter-clockwise ==> decrement.
			else if (encoderCurrent > encoderPrevious)
				encoderDelta = 1; // Clockwise => increment.
	}
	// Save current encoder position to become previous (for next check).
	encoderPrevious = encoderCurrent;

	if (activeCLK != activeCLKPrevious) {
		// Is state was changed (added or removed a patch cable to/away CLK port)?
		// New state will become "previous" state.
		activeCLKPrevious = activeCLK;
		// Reset all steps counter and "gaps", not synchronized.
		currentStep = 0;
		previousStep = 0;
		expectedStep = 0;
		stepGap = 0;
		stepGapPrevious = 0;
		isSync = false;
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
			canPulse[i] = false;
			nextPulseStep[i] = 0;
		}
		if (!activeCLK)
			updateDMDtoRunningMode(0);
			else {
				activeCV = inputs[INPUT_CV_TRIG].active;
				updateDMDtoRunningMode(1);
			}
	}

	if (activeCV != activeCVPrevious) {
		// Is state was changed (added or removed a patch cable to/away CLK port)?
		// New state will become "previous" state.
		activeCVPrevious = activeCV;
		if (activeCLK)
			updateDMDtoRunningMode(1);
	}

	// Considering CV (if applicable e.g. wired!).
	voltageOnCV = 0.0f;
	isRatioCVmod = false;
	rateRatioCV = 0.0f;

	if (activeCV) {
		voltageOnCV = inputs[INPUT_CV_TRIG].value;
		if (activeCLK) {
			// Considering CV-RATIO signal to modulate ratio (doesn't matter if SETUP is running, or not).
			isRatioCVmod = true;
			if (bipolarCV)
				rateRatioCV = round(clamp(static_cast<float>(voltageOnCV), -5.0f, 5.0f) * 12.6f); // By bipolar voltage (-5V/+5V).
				else rateRatioCV = round((clamp(static_cast<float>(voltageOnCV), 0.0f, 10.0f) - 5.0f) * 12.6f); // By unipolar voltage (0V/+10V).
			// Update DMD.
			updateDMDtoRunningMode(1);
		}
		else {
			// BPM is set by encoded (except while SETUP is running).
			if (!isSetupRunning) {
				if (encoderDelta != 0) {
					BPM = BPM + encoderDelta; // May be increased or decreased.
					if (BPM < 1)
						BPM = 1; // Minimum 1 BPM.
						else if (BPM > 960)
							BPM = 960; // Maximum 960 BPM.
					this->svBPM = BPM;
					// Reset encoder move detection.
					encoderDelta = 0;
					// Update DMD.
					updateDMDtoRunningMode(0);
				}
			}
		}
	}
	else {
		if (!isSetupRunning) {
			if (activeCLK) {
				// Preset ratios are controlled by encoder.
				if (encoderDelta != 0) {
					rateRatioByEncoder = rateRatioByEncoder + encoderDelta;
					if (rateRatioByEncoder < 0)
						rateRatioByEncoder = 0; // Limiting to 0 (/64).
						else if (rateRatioByEncoder > 30)
							rateRatioByEncoder = 30; // Limiting to 30 (X64).
					this->svRatio = rateRatioByEncoder;
					// Related multiplier/divider mode.
					clkModulatorMode = DIV;
					if (rateRatioByEncoder == 15)
						clkModulatorMode = X1;
						else if (rateRatioByEncoder > 15)
							clkModulatorMode = MULT;
					// Reset encoder move detection.
					encoderDelta = 0;
					// Update DMD.
					updateDMDtoRunningMode(1);
				}
			}
			else {
				// BPM is set by encoded (except while SETUP is running).
				if (!isSetupRunning) {
					if (encoderDelta != 0) {
						BPM = BPM + encoderDelta; // May be increased or decreased.
						if (BPM < 1)
							BPM = 1; // Minimum BPM is 1.
							else if (BPM > 960)
								BPM = 960; // Maximum 960 BPM.
						this->svBPM = BPM;
						// Reset encoder move detection.
						encoderDelta = 0;
						// Update DMD.
						updateDMDtoRunningMode(0);
					}
				}
			}
		}
	}

	// Button state.
	buttonPressed = runButton.process(params[PARAM_BUTTON].value);

	// KlokSpid is working as multiplier/divider module (when CLK input port is connected - aka "active").
	if (activeCLK) {
		// Increment step number.
		currentStep++;
		// Using Schmitt trigger (SchmittTrigger is provided by dsp/digital.hpp) to detect thresholds from CLK input connector. Calibration: +1.7V (rising edge), low +0.2V (falling edge).
		if (CLKInputPort.process(rescale(inputs[INPUT_CLOCK].value, 0.2f, 1.7f, 0.0f, 1.0f))) {
			// CLK input is receiving a compliant trigger voltage (rising edge): lit and "afterglow" CLK (red) LED.
			ledClkDelay = 0;
			ledClkAfterglow = true;
			if (previousStep == 0) {
				// No "history", it's the first pulse received on CLK input after a frequency change. Not synchronized.
				expectedStep = 0;
				stepGap = 0;
				stepGapPrevious = 0;
				// stepGap at 0: the pulse duration will be 1 ms (default), or 2 ms or 5 ms (depending SETUP). Variable pulses can't be used as long as frequency remains unknown.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					if (isRatioCVmod)
						pulseDuration[i] = GetPulsingTime(0, 1.0f / rateRatioCV);  // Ratio is CV-controlled.
						else pulseDuration[i] = GetPulsingTime(0, list_fRatio[rateRatioByEncoder]);  // Ratio is controlled by encoder.
				// Not synchronized.
				isSync = false;
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
					canPulse[i] = (clkModulatorMode != MULT); // MULT needs second pulse to establish source frequency.
					pulseDivCounter[i] = 0; // Used for DIV mode exclusively!
					pulseMultCounter[i] = 0; // Used for MULT mode exclusively!
				}
				previousStep = currentStep;
			}
			else {
				// It's the second pulse received on CLK input after a frequency change.
				stepGapPrevious = stepGap;
				stepGap = currentStep - previousStep;
				expectedStep = currentStep + stepGap;
				// The frequency is known, we can determine the pulse duration (defined by SETUP).
				// The pulse duration also depends of clocking ratio, such "X1", multiplied or divided, and its ratio.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					if (isRatioCVmod)
						pulseDuration[i] = GetPulsingTime(stepGap, 1.0f / rateRatioCV); // Ratio is CV-controlled.
						else pulseDuration[i] = GetPulsingTime(stepGap, list_fRatio[rateRatioByEncoder]); // Ratio is controlled by encoder.
				isSync = true;
				if (stepGap > stepGapPrevious)
					isSync = ((stepGap - stepGapPrevious) < 2);
					else if (stepGap < stepGapPrevious)
						isSync = ((stepGapPrevious - stepGap) < 2);
				if (isSync) {
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						canPulse[i] = (clkModulatorMode != DIV);
				}
				else {
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						canPulse[i] = (clkModulatorMode == X1);
				}
				previousStep = currentStep;
			}

			switch (clkModulatorMode) {
				case X1:
					// Ratio is x1, following source clock, the easiest scenario! (always sync'd).
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						canPulse[i] = true;
					break;
				case DIV:
					// Divider mode scenario.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
						if (pulseDivCounter[i] == 0) {

							if (isRatioCVmod)
								pulseDivCounter[i] = int(1.0f / rateRatioCV) - 1; // Ratio is CV-controlled.
								else pulseDivCounter[i] = int(list_fRatio[rateRatioByEncoder] - 1); // Ratio is controlled by knob.
							canPulse[i] = true;
						}
						else {
							pulseDivCounter[i]--;
							canPulse[i] = false;
						}
					}
					break;
				case MULT:
					// Multiplier mode scenario: pulsing only when source frequency is established.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
						if (isSync) {
							// Next step for pulsing in multiplier mode.
							if (isRatioCVmod) {
								// Ratio is CV-controlled.
								nextPulseStep[i] = currentStep + round(stepGap / rateRatioCV);
								pulseMultCounter[i] = int(rateRatioCV) - 1;
							}
							else {
							// Ratio is controlled by knob.
								nextPulseStep[i] = currentStep + round(stepGap * list_fRatio[rateRatioByEncoder]);
								pulseMultCounter[i] = round(1.0f / list_fRatio[rateRatioByEncoder]) - 1;
							}
							canPulse[i] = true;
						}
					}
			}
		}
		else {
			// At this point, it's not a rising edge!
			// When running as multiplier, may pulse here too during low voltages on CLK input!
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
				if (isSync && (nextPulseStep[i] == currentStep) && (clkModulatorMode == MULT)) {
					if (isRatioCVmod)
						nextPulseStep[i] = currentStep + round(stepGap / rateRatioCV); // Ratio is CV-controlled.
						else nextPulseStep[i] = currentStep + round(stepGap * list_fRatio[rateRatioByEncoder]); // Ratio is controlled by knob.
					// This block is to avoid continuous pulsing if no more receiving incoming signal.
					if (pulseMultCounter[i] > 0) {
						pulseMultCounter[i]--;
						canPulse[i] = true;
					}
					else {
						canPulse[i] = false;
						isSync = false;
					}
				}
			}
		}
	}
	else {
		// CLK input port isn't connected (not active): KlokSpid is working as clock generator.
		ledClkAfterglow = false;
		if (previousBPM == BPM) {
			// CV-RATIO/TRIG. input port is used as TRIG. to reset clock generator or to toggle BPM-clocking, while voltage is +1.7 V (or above) - rising edge.
			if (activeCV) {
				if (runTriggerPort.process(rescale(voltageOnCV, 0.2f, 1.7f, 0.0f, 1.0f))) {
					// On +1.7 V trigger (rising edge), the clock generator state if toggled (started or stopped).
					if (transportTrig) {
						// CV-RATIO/TRIG. input port (TRIG.) is configured as "play/stop toggle".
						isBPMRunning = !isBPMRunning;
						// BPM state persistence (json).
						this->runBPMOnInit = isBPMRunning;
						for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
							nextPulseStep[i] = 0;
						currentStep = 0;
						// Reset phase for LFO jack #4.
						resetPhase = true;
					}
					else {
						// CV-RATIO/TRIG. input port (TRIG.) is configured as RESET input (default factory): assuming it's an incoming reset signal!
						currentStep = 0;
						for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
							nextPulseStep[i] = 0;
						// Reset phase for LFO jack #4.
						resetPhase = true;
					}
				}
			}
			// Incrementing step counter...
			currentStep++;
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
				if (isBPMRunning) {
					if (currentStep >= nextPulseStep[i])
						canPulse[i] = true;
					if (canPulse[i]) {
						// Setting pulse...
						// Define the step for next pulse. Time reference is given by (current) engine samplerate setting.
						nextPulseStep[i] = currentStep + round(60.0f * engineGetSampleRate() * list_outRatiof[outputRatioInUse[i]] / BPM);
						// Define the pulse duration (fixed or variable-length).
						pulseDuration[i] = GetPulsingTime(engineGetSampleRate(), 60.0f / BPM * list_outRatiof[outputRatioInUse[i]]);
						if (i == OUTPUT_4)
							resetPhase = true;
					}
				}
				else {
					// BPM clock is stopped.
					canPulse[i] = false;
					nextPulseStep[i] = 0;
					currentStep = 0;
					// Reset phase for LFO jack #4.
					resetPhase = true;
				}
			}
		}
		else {
			// Update DMD (number of BPM).
			updateDMDtoRunningMode(0);
			// Altered BPM: reset phase for LFO jack #4.
			resetPhase = true;
		}
		previousBPM = BPM;
	}

	// Using pulse generator to output to all ports.
	for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
		if (canPulse[i]) {
			if (i == OUTPUT_4) {
				if (resetPhase) {
					LFOjack4.phase = 0.0f;
					resetPhase = false;
				}
			}
			// Sending pulse, using pulse generator.
			sendPulse[i].triggerDuration = pulseDuration[i];
	  	sendPulse[i].trigger(pulseDuration[i]);
			canPulse[i] = false;
		}
		sendingOutput[i] = sendPulse[i].process(engineGetSampleTime());
		if (i < OUTPUT_4)
	  	outputs[i].value = sendingOutput[i] ? outVoltage : 0.0f;
			else {
				// Jack #4 specific (LFO feature to output jack #4, but: clock generator mode only, and if jack ratio is set at "x1" only).
				if ((!activeCLK) && (jack4LFO != 0) && (outputRatioInUse[OUTPUT_4] == 12)) {
					LFOjack4.invert = ((jack4LFO % 2) == 0);
					LFOjack4.freq = (float)BPM / 60.0f;
					LFOjack4.step(engineGetSampleTime());
					if (jack4LFObipolar)
						LFOjack4.offset = 0.0f;
						else LFOjack4.offset = outVoltage / 2.0f;
					switch (jack4LFO) {
						case 1:
						case 2:
							// LFO for jack #4 is a sine-based waveform.
							outputs[OUTPUT_4].value = isBPMRunning ? outVoltage / 2.0f * LFOjack4.sin() : 0.0f;
							break;
						case 3:
						case 4:
							// LFO for jack #4 is a triangle-based waveform.
							outputs[OUTPUT_4].value = isBPMRunning ? outVoltage / 2.0f * LFOjack4.tri() : 0.0f;
							break;
						case 5:
						case 6:
							// LFO for jack #4 is a sawtooth-based waveform.
							outputs[OUTPUT_4].value = isBPMRunning ? outVoltage / 2.0f * LFOjack4.saw() : 0.0f;
					}
				}
				else outputs[OUTPUT_4].value = sendingOutput[OUTPUT_4] ? outVoltage : 0.0f;
			}
	}

	// Afterglow for CLK (red) LED.
	if (ledClkAfterglow) {
		if (inputs[INPUT_CLOCK].value < 1.7f) {
			ledClkDelay++;
			if (ledClkDelay > round(engineGetSampleRate() / 16)) {
				ledClkAfterglow = false;
				ledClkDelay = 0;
			}
		}
	}

	// Handling the button (it's a momentary button, handled by a dedicated Schmitt trigger).
	// - Short presses toggles BPM clock start/stops (when released).
	// - Long press to enter SETUP.
	// - When SETUP is running, press to advance to next parameter.
	if (buttonPressed) {
		if (!isSetupRunning && !isEnteringSetup) {
			// Try to enter SETUP... starting delay counter for 2 seconds.
			isEnteringSetup = true;
			setupCounter = 0;
			allowedButtonHeld = true; // Allow to keep button held.
		}
		else if (isSetupRunning && !isExitingSetup) {
			// Try to quick save/exit SETUP... starting delay counter for 2 seconds.
			isExitingSetup = true;
			setupCounter = 0;
			// Necessary to avoid continuous entry/exit SETUP while button is held.
			allowedButtonHeld = true; // Allow to keep button held.
		}
		else allowedButtonHeld = false; // Button must be released.
	} // Don't add "else" clause from here, otherwise be sure it doesn't work!

	if (buttonPressed && isSetupRunning) {
		// SETUP is running: when (shortly) pressed, advance to next parameter.
		// Storing previous edited parameter into "edited" table prior to advance to next SETUP parameter.
		setup_Edited[setup_ParamIdx] = setup_CurrentValue;
		// Advance to next SETUP parameter (conditional).
		if ((setup_ParamIdx == SETUP_OUTSRATIOS) && (setup_Edited[SETUP_OUTSRATIOS] == 0))
			setup_ParamIdx = SETUP_OUT4LFO; // Bypass all four jack ratios, and go directly to jack #4 LFO.
			else if ((setup_ParamIdx == SETUP_OUT4RATIO) && (setup_Edited[SETUP_OUTSRATIOS] == 1) && (setup_Edited[SETUP_OUT4RATIO] != 12))
				setup_ParamIdx = SETUP_CVTRIG; // In case custom output jack ratios, jack #4 ratio isn't x1, bypass jack #4 LFO & polarity, and go directly to CV/TRIG.
				else if ((setup_ParamIdx == SETUP_OUT4LFO) && (setup_Edited[SETUP_OUT4LFO] == 0))
					setup_ParamIdx = SETUP_CVTRIG; // Bypass LFO polarity entry is LFO mode is disabled, go directly to CV/TRIG.
					else setup_ParamIdx++; // Advance to next, in all other cases.
		// These variables are used to cycle display (parameter name, then its value). DEPRECATED!
		setupCounter = 0;
		if (setup_ParamIdx > SETUP_EXIT) {
			// Exiting SETUP. From here, all required actions on exit SETUP (such save, cancel changes, reset to default factory etc), except "Review" option!
			switch (setup_Edited[SETUP_EXIT]) {
				case 0:
					// Cancel/Exit: all changes from SETUP are ignored (changes are cancelled).
					// all previous (backuped) will be restored (any change is ignored).
					for (int i = 1; i < SETUP_EXIT; i++)
						setup_Current[i] = setup_Backup[i];
					// Restored pre-SETUP settings, so it's useless to save them as "json" persistent.
					UpdateKlokSpidSettings(false);
					break;
				case 1:
					// Save/Exit: all parameters from SETUP will be saved.
					for (int i = 1; i < SETUP_EXIT; i++)
						setup_Current[i] = setup_Edited[i];
					// Using new settings (because edited are saved), so it's mandatory to save them as "json" persistent datas.
					UpdateKlokSpidSettings(true);
					break;
				case 2:
					// Review: return to first parameter (don't exit "SETUP" in this choice is selected).
					setup_ParamIdx = 1;
					setup_CurrentValue = setup_Edited[1]; // Bypass the welcome message and edit first parameter.
					strcpy(dmdMessage1, stringToPchar(setupMenuName[1]));
					xOffsetValue = setupParamXOffset[1][setup_CurrentValue];
					strcpy(dmdMessage2, stringToPchar(setupParamName[1][setup_CurrentValue]));
					break;
				case 3:
					// Factory: restore all factory default parameters.
					for (int i = 1; i < SETUP_EXIT; i++)
						setup_Current[i] = setup_Factory[i];
					// Using new settings (because restored as default factory), like "Save/Exit", it's mandatory to save them as "json" persistent datas.
					UpdateKlokSpidSettings(true);
					break;
			}
			// Exit SETUP, except if "Review" was selected.
			if (setup_Edited[SETUP_EXIT] != 2) {
				// Exit SETUP (except if "Review" was selected).
				setupCounter = 0;
				// Clearing flag because now exit SETUP.
				isSetupRunning = false;
				// Update DMD to current running mode.
				if (activeCLK)
					updateDMDtoRunningMode(1); // Ratio (clock modulator mode).
					else updateDMDtoRunningMode(0); // Number of BPM.
			}
		}
		else if (setup_ParamIdx == SETUP_EXIT) {
			// Last default proposed parameter will be "Save and Exit" (SETUP).
			setup_CurrentValue = 1;
			// Update DMD.
			strcpy(dmdMessage1, stringToPchar(setupMenuName[setup_ParamIdx]));
			xOffsetValue = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
			strcpy(dmdMessage2, stringToPchar(setupParamName[setup_ParamIdx][setup_CurrentValue]));
		}
		else {
			// Set currently displayed (on DMD) value as current (edited) parameter.
			setup_CurrentValue = setup_Edited[setup_ParamIdx];
			// Update DMD.
			strcpy(dmdMessage1, stringToPchar(setupMenuName[setup_ParamIdx]));
			xOffsetValue = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
			strcpy(dmdMessage2, stringToPchar(setupParamName[setup_ParamIdx][setup_CurrentValue]));
		}
	}

	if (runButton.isHigh()) {
		// Button is held, don't matter if SETUP is running or not.
		// Always increment the counter.
		setupCounter++;
		if (isSetupRunning && isExitingSetup) {
			if (setupCounter >= 2 * engineGetSampleRate()) {
				// Button was held during 2 seconds (while SETUP is running): now KlokSpid module exit SETUP (doing auto "Save/Exit").
				//
				isExitingSetup = false;
				setupCounter = 0;
				allowedButtonHeld = false; // Button must be released (retrigger is required).
				for (int i=0; i<SETUP_EXIT; i++)
					setup_Current[i] = setup_Edited[i];
				// Quick SETUP-exit, doing automatic "Save/exit", by this way using new settings (because edited are saved), so it's mandatory to save them as "json" persistent datas.
				UpdateKlokSpidSettings(true);
				// Clearing flag because now exit SETUP.
				isSetupRunning = false;
				// Update DMD regadling current running mode.
				if (activeCLK)
					updateDMDtoRunningMode(1); // Ratio (clock modulator mode).
					else updateDMDtoRunningMode(0); // Number of BPM.
			}
		}
		else {
			if (isEnteringSetup && (setupCounter >= 2 * engineGetSampleRate())) {
				// Button was finally held during 2 seconds: now KlokSpid module runs its SETUP. Initializing some variables/arrays/flags first.
				//
				isEnteringSetup = false;
				setupCounter = 0;
				// Button must be released (retrigger is required).
				allowedButtonHeld = false;
				// Menu entry #0 is used to display "- SETUP -" as welcome message (don't have parameters, so be sure "parameter" is set to 0).
				setup_Current[SETUP_WELCOME_MESSAGE] = 0;
				// Copy current parameters (since initialization or previous SETUP) to "edited" parameters before entering SETUP.
				// Also use a "backup" table in case of "Cancel/Exit" choice.
				for (int i = 0; i < SETUP_EXIT; i++) {
					setup_Backup[i] = setup_Current[i];
					setup_Edited[i] = setup_Current[i];
				}
				// Lastest menu entry is used to exit SETUP menu, by default with save.
				setup_Edited[SETUP_EXIT] = 1;
				// Select first parameter will ne displayed. In fact, the welcome message "- SETUP -" on DMD when entered SETUP.
				setup_ParamIdx = 0;
				// Update DMD.
				strcpy(dmdMessage1, stringToPchar(setupMenuName[SETUP_WELCOME_MESSAGE]));
				xOffsetValue = setupParamXOffset[SETUP_WELCOME_MESSAGE][0];
				strcpy(dmdMessage2, stringToPchar(setupParamName[SETUP_WELCOME_MESSAGE][0]));
				// This flag indicates SETUP is running.
				isSetupRunning = true;
			}
		}
	}
	else {
		if (isEnteringSetup) {
			// Abort entering SETUP.
			isEnteringSetup = false;
			// Button works as BPM start/stop toggle: inverting state.
			isBPMRunning = !isBPMRunning;
			// Persistence for current BPM-state (toJson).
			this->runBPMOnInit = isBPMRunning;
		}
		else if (isSetupRunning && isExitingSetup) {
			// Abort quick exit SETUP.
			isExitingSetup = false;
		}
		setupCounter = 0;
		allowedButtonHeld = false; // button must be "retriggered", to avoid continuous entry/exit SETUP while held.
	}

	// KlokSpid module's SETUP.
	if (isSetupRunning) {
		// SETUP is running.
		if (encoderDelta > 0) {
			// Incremented encoder (rotated clockwise).
			setup_CurrentValue++;
			if (setup_CurrentValue >= setup_NumValue[setup_ParamIdx])
				setup_CurrentValue = 0; // End of values list: return to first value.
			// Update DMD.
			strcpy(dmdMessage1, stringToPchar(setupMenuName[setup_ParamIdx]));
			xOffsetValue = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
			strcpy(dmdMessage2, stringToPchar(setupParamName[setup_ParamIdx][setup_CurrentValue]));
			// Update current parameter "in realtime".
			setup_Current[setup_ParamIdx] = setup_CurrentValue;
			// Update parameters, but without "jSon" persistence while SETUP is running!
			UpdateKlokSpidSettings(false);
			// Reset encoder move detection.
			encoderDelta = 0;
		}
		else if (encoderDelta < 0) {
			// Decremented encoder (rotated counter-clockwise).
			if (setup_NumValue[setup_ParamIdx] != 0) {
				if (setup_CurrentValue == 0)
					setup_CurrentValue = setup_NumValue[setup_ParamIdx] - 1; // Return to last possible value.
					else setup_CurrentValue--; //Previous value.
			}
			// Update DMD.
			strcpy(dmdMessage1, stringToPchar(setupMenuName[setup_ParamIdx]));
			xOffsetValue = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
			strcpy(dmdMessage2, stringToPchar(setupParamName[setup_ParamIdx][setup_CurrentValue]));
			// Update current parameter "in realtime".
			setup_Current[setup_ParamIdx] = setup_CurrentValue;
			// Update parameters, but without "jSon" persistence while SETUP is running!
			UpdateKlokSpidSettings(false);
			// Reset encoder move detection.
			encoderDelta = 0;
		}
	}

	// Handling LEDs on KlokSpid module (at the end of step).
  lights[LED_SYNC_GREEN].value = ((activeCLK && (isSync || (clkModulatorMode == X1))) || (!activeCLK && isBPMRunning)) ? 1.0 : 0.0; // Unique "SYNC" LED: will be lit green color when sync'd / BPM is running.
  lights[LED_SYNC_RED].value = ((activeCLK && (isSync || (clkModulatorMode == X1))) || (!activeCLK && isBPMRunning)) ? 0.0 : 1.0; // Unique "SYNC" LED: will be lit red color (opposite cases).
  lights[LED_CLK].value = (isSetupRunning || ledClkAfterglow) ? 1.0 : 0.0;
	lights[LED_CV_TRIG].value = (isSetupRunning || activeCV) ? 1.0 : 0.0; // TODO -- MUST BE ENHANCED!
	lights[LED_CVMODE].value = (isSetupRunning || activeCLK) ? 1.0 : 0.0;
	lights[LED_TRIGMODE].value = (isSetupRunning || !activeCLK) ? 1.0 : 0.0;

}	// End of KlokSpid module's step() function.


// Dot-matrix display (DMD) and small displays (near output jack) handler.
struct KlokSpidDMD : TransparentWidget {
	KlokSpidModule *module;
	std::shared_ptr<Font> font;
	KlokSpidDMD() {
		font = Font::load(assetPlugin(plugin, "res/fonts/LEDCounter7.ttf"));
	}

	void updateDMD1(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, char* dmdMessage1) {
		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);
		nvgFillColor(vg, nvgTransRGBA(DMDtextColor, 0xff));
		nvgText(vg, pos.x, pos.y, dmdMessage1, NULL);
	}

	void updateDMD2(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, int xOffsetValue, char* dmdMessage2) {
		nvgFontSize(vg, 20);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(DMDtextColor, 0xff));
		nvgText(vg, pos.x + xOffsetValue, pos.y, dmdMessage2, NULL);
	}

	void updateDispOut1(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, int xdispOutOffset1, char* dispOut1) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(DMDtextColor, 0xff));
		nvgText(vg, pos.x + xdispOutOffset1, pos.y, dispOut1, NULL);
	}

	void updateDispOut2(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, int xdispOutOffset2, char* dispOut2) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(DMDtextColor, 0xff));
		nvgText(vg, pos.x + xdispOutOffset2, pos.y, dispOut2, NULL);
	}

	void updateDispOut3(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, int xdispOutOffset3, char* dispOut3) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(DMDtextColor, 0xff));
		nvgText(vg, pos.x + xdispOutOffset3, pos.y, dispOut3, NULL);
	}

	void updateDispOut4(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, int xdispOutOffset4, char* dispOut4) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(DMDtextColor, 0xff));
		nvgText(vg, pos.x + xdispOutOffset4, pos.y, dispOut4, NULL);
	}

	void draw(NVGcontext *vg) override {
		// Main DMD.
		updateDMD1(vg, Vec(14, box.size.y - 174), module->DMDtextColor, module->dmdMessage1);
		updateDMD2(vg, Vec(12, box.size.y - 152), module->DMDtextColor, module->xOffsetValue, module->dmdMessage2);
		// Display between output jacks.
		updateDispOut1(vg, Vec(35, box.size.y + 61), module->DMDtextColor, module->xdispOutOffset1, module->dispOut1);
		updateDispOut2(vg, Vec(62.5, box.size.y + 61), module->DMDtextColor, module->xdispOutOffset2, module->dispOut2);
		updateDispOut3(vg, Vec(35, box.size.y + 75), module->DMDtextColor, module->xdispOutOffset3, module->dispOut3);
		updateDispOut4(vg, Vec(62.5, box.size.y + 75), module->DMDtextColor, module->xdispOutOffset4, module->dispOut4);
	}

};

struct KlokSpidWidget : ModuleWidget {
	// Themed plates.
	SVGPanel *panelKlokSpidClassic;
	SVGPanel *panelKlokSpidStageRepro;
	SVGPanel *panelKlokSpidAbsoluteNight;
	SVGPanel *panelKlokSpidDarkSignature;
	SVGPanel *panelKlokSpidDeepBlueSignature;
	SVGPanel *panelCarbonSignature;
	// Silver Torx screws.
	SVGScrew *topLeftScrewSilver;
	SVGScrew *topRightScrewSilver;
	SVGScrew *bottomLeftScrewSilver;
	SVGScrew *bottomRightScrewSilver;
	// Gold Torx screws.
	SVGScrew *topLeftScrewGold;
	SVGScrew *topRightScrewGold;
	SVGScrew *bottomLeftScrewGold;
	SVGScrew *bottomRightScrewGold;
	// Silver button.
	SVGSwitch *buttonSilver;
	// Gold button.
	SVGSwitch *buttonGold;
	//
	KlokSpidWidget(KlokSpidModule *module);
	void step() override;

	// Action for "Initialize", from context-menu, is (for now) bypassed.
	void reset() override {
		return;
	};

	// Action for "Randomize", from context-menu, is (for now) bypassed.
	void randomize() override {
		return;
	};

	Menu* createContextMenu() override;
};

KlokSpidWidget::KlokSpidWidget(KlokSpidModule *module) : ModuleWidget(module) {
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	// Model: Classic (beige plate, always default GUI when added from modules menu).
	panelKlokSpidClassic = new SVGPanel();
	panelKlokSpidClassic->box.size = box.size;
	panelKlokSpidClassic->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Classic.svg")));
	addChild(panelKlokSpidClassic);
	// Model: Stage Repro.
	panelKlokSpidStageRepro = new SVGPanel();
	panelKlokSpidStageRepro->box.size = box.size;
	panelKlokSpidStageRepro->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Stage_Repro.svg")));
	addChild(panelKlokSpidStageRepro);
	// Model: Absolute Night.
	panelKlokSpidAbsoluteNight = new SVGPanel();
	panelKlokSpidAbsoluteNight->box.size = box.size;
	panelKlokSpidAbsoluteNight->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Absolute_Night.svg")));
	addChild(panelKlokSpidAbsoluteNight);
	// Model: Dark "Signature".
	panelKlokSpidDarkSignature = new SVGPanel();
	panelKlokSpidDarkSignature->box.size = box.size;
	panelKlokSpidDarkSignature->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Dark_Signature.svg")));
	addChild(panelKlokSpidDarkSignature);
	// Model: Deepblue "Signature".
	panelKlokSpidDeepBlueSignature = new SVGPanel();
	panelKlokSpidDeepBlueSignature->box.size = box.size;
	panelKlokSpidDeepBlueSignature->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Deepblue_Signature.svg")));
	addChild(panelKlokSpidDeepBlueSignature);
	// Model: Carbon "Signature".
	panelCarbonSignature = new SVGPanel();
	panelCarbonSignature->box.size = box.size;
	panelCarbonSignature->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Carbon_Signature.svg")));
	addChild(panelCarbonSignature);
	// Always four screws for 8 HP module, may are silver or gold, depending model.
	// Top-left silver Torx screw.
	topLeftScrewSilver = Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0));
	addChild(topLeftScrewSilver);
	// Top-right silver Torx screw.
	topRightScrewSilver = Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
	addChild(topRightScrewSilver);
	// Bottom-left silver Torx screw.
	bottomLeftScrewSilver = Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomLeftScrewSilver);
	// Bottom-right silver Torx screw.
	bottomRightScrewSilver = Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomRightScrewSilver);
	// Top-left gold Torx screw.
	topLeftScrewGold = Widget::create<Torx_Gold>(Vec(RACK_GRID_WIDTH, 0));
	addChild(topLeftScrewGold);
	// Top-right gold Torx screw.
	topRightScrewGold = Widget::create<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
	addChild(topRightScrewGold);
	// Bottom-left gold Torx screw.
	bottomLeftScrewGold = Widget::create<Torx_Gold>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomLeftScrewGold);
	// Bottom-right gold Torx screw.
	bottomRightScrewGold = Widget::create<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomRightScrewGold);
	// DMD display.
	{
		KlokSpidDMD *display = new KlokSpidDMD();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 234);
		addChild(display);
	}
	// Ratio/BPM/SETUP encoder.
	//addParam(ParamWidget::create<KS_Encoder>(Vec(20, 106), module, KlokSpidModule::PARAM_ENCODER, -INFINITY, +INFINITY, 0.0));
	module->klokspdEncoder = dynamic_cast<Knob*>(Knob::create<KS_Encoder>(Vec(20, 106), module, KlokSpidModule::PARAM_ENCODER, -INFINITY, +INFINITY, 0.0));
	addParam(module->klokspdEncoder);
	// Push button (silver), used to toggle START/STOP, also used to enter SETUP, and to advance to next parameter in SETUP.
	buttonSilver = ParamWidget::create<KS_ButtonSilver>(Vec(94, 178), module, KlokSpidModule::PARAM_BUTTON , 0.0, 1.0, 0.0);
	addParam(buttonSilver);
	// Push button (gold), used to toggle START/STOP, also used to enter SETUP, and to advance to next parameter in SETUP.
	buttonGold = ParamWidget::create<KS_ButtonGold>(Vec(94, 178), module, KlokSpidModule::PARAM_BUTTON , 0.0, 1.0, 0.0);
	addParam(buttonGold);
	// Input ports (golden jacks).
	addInput(Port::create<PJ301M_In>(Vec(24, 215), Port::INPUT, module, KlokSpidModule::INPUT_CLOCK));
	addInput(Port::create<PJ301M_In>(Vec(72, 215), Port::INPUT, module, KlokSpidModule::INPUT_CV_TRIG));
	// Output ports (golden jacks).
	addOutput(Port::create<PJ301M_Out>(Vec(10, 261), Port::OUTPUT, module, KlokSpidModule::OUTPUT_1));
	addOutput(Port::create<PJ301M_Out>(Vec(86, 261), Port::OUTPUT, module, KlokSpidModule::OUTPUT_2));
	addOutput(Port::create<PJ301M_Out>(Vec(10, 310), Port::OUTPUT, module, KlokSpidModule::OUTPUT_3));
	addOutput(Port::create<PJ301M_Out>(Vec(86, 310), Port::OUTPUT, module, KlokSpidModule::OUTPUT_4));
	// LEDs (red for CLK input, yellow for CV-RATIO/TRIG input).
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(31.5, 242), module, KlokSpidModule::LED_CLK));
	addChild(ModuleLightWidget::create<MediumLight<KlokSpidOrangeLight>>(Vec(79.5, 242), module, KlokSpidModule::LED_CV_TRIG));
	//addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(56, 242), module, KlokSpidModule::LED_OUTPUT));


	addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(7, 96), module, KlokSpidModule::LED_SYNC_GREEN)); // Unified SYNC LED (green/red).
	// Small-sized orange LEDs near CV-RATIO/TRIG input port (when lit, each LED indicates the port role).
	addChild(ModuleLightWidget::create<SmallLight<KlokSpidOrangeLight>>(Vec(67.5, 206), module, KlokSpidModule::LED_CVMODE));
	addChild(ModuleLightWidget::create<SmallLight<KlokSpidOrangeLight>>(Vec(95, 206), module, KlokSpidModule::LED_TRIGMODE));
}

//// Make one visible theme at once!

void KlokSpidWidget::step() {
	KlokSpidModule *klokspidmodule = dynamic_cast<KlokSpidModule*>(module);
	assert(klokspidmodule);
	// "Signature"-line modules are using gold parts (instead of silver).
	bool isSignatureLine = (klokspidmodule->Theme > 2);
	// Themed module plate, selected via context-menu, is visible (all others are obviously, hidden).
	panelKlokSpidClassic->visible = (klokspidmodule->Theme == 0);
	panelKlokSpidStageRepro->visible = (klokspidmodule->Theme == 1);
	panelKlokSpidAbsoluteNight->visible = (klokspidmodule->Theme == 2);
	panelKlokSpidDarkSignature->visible = (klokspidmodule->Theme == 3);
	panelKlokSpidDeepBlueSignature->visible = (klokspidmodule->Theme == 4);
	panelCarbonSignature->visible = (klokspidmodule->Theme == 5);
	// Silver Torx screws are visible only for non-"Signature" modules.
	topLeftScrewSilver->visible = !isSignatureLine;
	topRightScrewSilver->visible = !isSignatureLine;
	bottomLeftScrewSilver->visible = !isSignatureLine;
	bottomRightScrewSilver->visible = !isSignatureLine;
	// Gold Torx screws are visible only for "Signature" modules.
	topLeftScrewGold->visible = isSignatureLine;
	topRightScrewGold->visible = isSignatureLine;
	bottomLeftScrewGold->visible = isSignatureLine;
	bottomRightScrewGold->visible = isSignatureLine;
	// Silver or gold button is visible at once (opposite is, obvisouly, hidden).
	buttonSilver->visible = !isSignatureLine;
	buttonGold->visible = isSignatureLine;
	// Resume original step() method.
	ModuleWidget::step();
}

//// CONTEXT-MENU (RIGHT-CLICK ON MODULE).

// Classic (default beige) module.
struct klokspidClassicMenu : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->Theme = 0;
	}
	void step() override {
		rightText = (klokspidmodule->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

// Stage Repro module.
struct klokspidStageReproMenu : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->Theme = 1;
	}
	void step() override {
		rightText = (klokspidmodule->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

// Absolute Night module.
struct klokspidAbsoluteNightMenu : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->Theme = 2;
	}
	void step() override {
		rightText = (klokspidmodule->Theme == 2) ? "✔" : "";
		MenuItem::step();
	}
};

// Dark "Signature" module.
struct klokspidDarkSignatureMenu : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->Theme = 3;
	}
	void step() override {
		rightText = (klokspidmodule->Theme == 3) ? "✔" : "";
		MenuItem::step();
	}
};

// Deepblue "Signature" module.
struct klokspidDeepblueSignatureMenu : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->Theme = 4;
	}
	void step() override {
		rightText = (klokspidmodule->Theme == 4) ? "✔" : "";
		MenuItem::step();
	}
};

// Carbon "Signature" module.
struct klokspidCarbonSignatureMenu : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->Theme = 5;
	}
	void step() override {
		rightText = (klokspidmodule->Theme == 5) ? "✔" : "";
		MenuItem::step();
	}
};

// CONTEXT-MENU CONSTRUCTION.

Menu* KlokSpidWidget::createContextMenu() {
	Menu* menu = ModuleWidget::createContextMenu();
	KlokSpidModule *klokspidmodule = dynamic_cast<KlokSpidModule*>(module);
	assert(klokspidmodule);
	menu->addChild(construct<MenuEntry>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Model:"));
	menu->addChild(construct<klokspidClassicMenu>(&klokspidClassicMenu::text, "Classic (default)", &klokspidClassicMenu::klokspidmodule, klokspidmodule));
	menu->addChild(construct<klokspidStageReproMenu>(&klokspidStageReproMenu::text, "Stage Repro", &klokspidStageReproMenu::klokspidmodule, klokspidmodule));
	menu->addChild(construct<klokspidAbsoluteNightMenu>(&klokspidAbsoluteNightMenu::text, "Absolute Night", &klokspidAbsoluteNightMenu::klokspidmodule, klokspidmodule));
	menu->addChild(construct<klokspidDarkSignatureMenu>(&klokspidDarkSignatureMenu::text, "Dark \"Signature\"", &klokspidDarkSignatureMenu::klokspidmodule, klokspidmodule));
	menu->addChild(construct<klokspidDeepblueSignatureMenu>(&klokspidDeepblueSignatureMenu::text, "Deepblue \"Signature\"", &klokspidDeepblueSignatureMenu::klokspidmodule, klokspidmodule));
	menu->addChild(construct<klokspidCarbonSignatureMenu>(&klokspidCarbonSignatureMenu::text, "Carbon \"Signature\"", &klokspidCarbonSignatureMenu::klokspidmodule, klokspidmodule));
	return menu;
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, KlokSpid) {
   Model *modelKlokSpid = Model::create<KlokSpidModule, KlokSpidWidget>("Ohmer Modules", "KlokSpid", "KlokSpid", CLOCK_TAG, CLOCK_MODULATOR_TAG); // CLOCK_MODULATOR_TAG introduced in 0.6 API.
   return modelKlokSpid;
}
