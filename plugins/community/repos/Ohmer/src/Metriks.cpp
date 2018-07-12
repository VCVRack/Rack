////////////////////////////////////////////////////////////////////////
//// Metriks is a 8 HP measuring/visual module:                    /////
//// - voltmeter (0 to 3 decimals).                                /////
//// - frequency counter / note tuner / BPM meter.                 /////
//// - pulses (ticks) counter (with adjustable voltage threshold). /////
////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"
#include <dsp/digital.hpp>
#include <string>

namespace rack_plugin_Ohmer {

// Module structure.
struct Metriks : Module {
	enum ParamIds {
		PARAM_ENCODER,
		BUTTON_MODE,
		BUTTON_PLAYPAUSE,
		BUTTON_RESET,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_SOURCE,
		INPUT_PLAYPAUSE,
		INPUT_RESET,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_THRU,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_PLAY_GREEN,
		LED_PLAY_RED,
		NUM_LIGHTS
	};

	// Pointer to encoder (handled as knob).
	Knob *mtksEncoder;

	// Module interface definitions, such parameters (encoder, button), input ports, output ports and LEDs.
	Metriks() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		onRandomize();
	}

	void onReset() override {
		if (!avoidOnResetReentry) {
			encoderPrevious = 0;
			onFirstInitCounter = (int)(engineGetSampleRate() / 4.0f);
			currentStep = 0;
			switchedModeCounter = 0;
			encoderDisplayCounter = 0;
			currentVoltage = 0.0f;
			currentFreq = -1.0f; // -1.0f means unknown frequency.
			currentPeakCount = 0;
			previousStep = 0;
			expectedStep = 0;
			bIsPlaying = false;
			bBlinkErr = false;
			bBlinkErrAlt = false;
			blinkSyncAnim = 0;
			bBlinkErrTimer = 0;
			avoidOnResetReentry = true;
		}
	}

	// Inhibit "Randomize" from context-menu / Ctrl+R / Command+R keyboard shortcut over module.
	void onRandomize() override {
	}

	//// GENERAL PURPOSE VARIABLES/FLAGS/TABLES.
	int onFirstInitCounter = 0;
	bool avoidOnResetReentry = false; // When true, this avoid OnReset() reentry.

	// Current selected Metriks model (GUI theme).
	int Theme = 0;
	// DMD-font color (default is black LCD), related to selected model.
	NVGcolor DMDtextColor = nvgRGB(0x08, 0x08, 0x08);

	// Measuring mode (0: voltmeter, 1: frequency counter/note tuner/BPM meter, 2: peak counter).
	int Mode = 0;
	int fmSubMode = 0; // Submode for frequency counter measuring (Mode == 1): 0 = frequency counter, 1 = note tuner, 2 = BPM meter.
	// Note to frequency equivalent table (octaves 0 to 9).
	float noteToFreq[120] = {16.351f, 17.324f, 18.354f, 19.445f, 20.601f, 21.827f, 23.124f, 24.499f, 25.956f, 27.500f, 29.135f, 30.868f,
		32.703f, 34.648f, 36.708f, 38.891f, 41.203f, 43.654f, 46.249f, 48.999f, 51.913f, 55.000f, 58.270f, 61.735f,
		65.406f, 69.296f, 73.416f, 77.782f, 82.407f, 87.307f, 92.499f, 97.999f, 103.826f, 110.000f, 116.541f, 123.471f,
		130.813f, 138.591f, 146.832f, 155.563f, 164.814f, 174.614f, 184.997f, 195.998f, 207.652f, 220.000f, 233.082f, 246.942f,
		261.626f, 277.183f, 293.665f, 311.127f, 329.628f, 349.228f, 369.994f, 391.995f, 415.305f, 440.000f, 466.164f, 493.883f,
		523.251f, 554.365f, 587.330f, 622.254f, 659.255f, 698.456f, 739.989f, 783.991f, 830.609f, 880.000f, 932.328f, 987.767f,
		1046.502f, 1108.731f, 1174.659f, 1244.508f, 1318.510f, 1396.913f, 1479.978f, 1567.982f, 1661.219f, 1760.000f, 1864.655f, 1975.533f,
		2093.005f, 2217.461f, 2349.318f, 2489.016f, 2637.021f, 2793.826f, 2959.955f, 3135.964f, 3322.438f, 3520.000f, 3729.310f, 3951.066f,
		4186.009f, 4434.922f, 4698.636f, 4978.032f, 5274.042f, 5587.652f, 5919.910f, 6271.928f, 6644.876f, 7040.000f, 7458.620f, 7902.132f,
		8372.018f, 8869.844f, 9397.272f, 9956.064f, 10548.084f, 11175.304f, 11839.820f, 12543.856f, 13289.752f, 14080.000f, 14917.240f, 15804.264f};
	// Note mames, including octave.
	std::string noteName[120] = {"C 0", "C#/Db 0", "D 0", "D#/Eb 0", "E 0", "F 0", "F#/Gb 0", "G 0", "G#/Ab 0", "A 0", "A#/Bb 0", "B 0",
		                           "C 1", "C#/Db 1", "D 1", "D#/Eb 1", "E 1", "F 1", "F#/Gb 1", "G 1", "G#/Ab 1", "A 1", "A#/Bb 1", "B 1",
		                           "C 2", "C#/Db 2", "D 2", "D#/Eb 2", "E 2", "F 2", "F#/Gb 2", "G 2", "G#/Ab 2", "A 2", "A#/Bb 2", "B 2",
		                           "C 3", "C#/Db 3", "D 3", "D#/Eb 3", "E 3", "F 3", "F#/Gb 3", "G 3", "G#/Ab 3", "A 3", "A#/Bb 3", "B 3",
		                           "C 4", "C#/Db 4", "D 4", "D#/Eb 4", "E 4", "F 4", "F#/Gb 4", "G 4", "G#/Ab 4", "A 4", "A#/Bb 4", "B 4",
		                           "C 5", "C#/Db 5", "D 5", "D#/Eb 5", "E 5", "F 5", "F#/Gb 5", "G 5", "G#/Ab 5", "A 5", "A#/Bb 5", "B 5",
		                           "C 6", "C#/Db 6", "D 6", "D#/Eb 6", "E 6", "F 6", "F#/Gb 6", "G 6", "G#/Ab 6", "A 6", "A#/Bb 6", "B 6",
		                           "C 7", "C#/Db 7", "D 7", "D#/Eb 7", "E 7", "F 7", "F#/Gb 7", "G 7", "G#/Ab 7", "A 7", "A#/Bb 7", "B 7",
		                           "C 8", "C#/Db 8", "D 8", "D#/Eb 8", "E 8", "F 8", "F#/Gb 8", "G 8", "G#/Ab 8", "A 8", "A#/Bb 8", "B 8",
		                           "C 9", "C#/Db 9", "D 9", "D#/Eb 9", "E 9", "F 9", "F#/Gb 9", "G 9", "G#/Ab 9", "A 9", "A#/Bb 9", "B 9"};
	// Messages displayed on DMD (dot-matrix display), using two lines..
	char dmdMessage1[20] = "";
	char dmdMessage2[20] = "";
	int xOffsetValue = 0; // Horizontal offset on DMD to display for second line.
	bool bBlinkErr = false; // When set, do blink the second line (missing input/over/unknown freq/over.
	bool bBlinkErrAlt = false; // Alternate true/false to blink second line.
	int blinkSyncAnim = 0; // Used to select animated string when determining frequency.
	int bBlinkErrTimer = 0; // Related timer
	// Encoder (registered position to be used on next step for relative move).
	int encoderCurrent = 0;
	int encoderPrevious = 0; // Encoder "absolute" (saved to jSon)...
	int encoderDelta = 0; // 0 if not moved, -1 if counter-clockwise (decrement), 1 if clockwise (increment).
	// Timers, for temporary displays.
	int switchedModeCounter = 0; // Used when switching to next mode (by MODE button).
	int encoderDisplayCounter = 0; // Used when changing a value via encoder.
	// MODE button.
	SchmittTrigger modeButton;
	// PLAY/STOP button and related (trigger) port (PLAY/STOP is used for "Pulse Counter" mode only).
	SchmittTrigger playButton;
	SchmittTrigger playPort;
	bool bIsPlaying = false;
	// RESET (RST) button and related (trigger) port.
	SchmittTrigger resetButton;
	SchmittTrigger resetPort;
	// Values can be displayed.
	int vmDecimals = 2;
	bool bSourceActive = false; // Source (input) is connected (active) or not.
	float currentVoltage = 0.0f; // Used by voltmeter.
	float currentFreq = -1.0f; // -1.0f means unknown frequency.
	unsigned long int currentPeakCount = 0;
	// Schmitt trigger used to determine frequency. Also used for peak counter mode.
	SchmittTrigger inputPort;
	float triggerVoltage = 1.7f;
	// Step-based counters (mainly used for frequency counter submodes).
	long long int currentStep = 0;
	long long int previousStep = 0;
	long long int expectedStep = 0;

	// Functions & methods (voids).
	void step() override;

	// Custom function to round a float at given decimal(s).
	float roundp(float f, int prec) {
		return (float)(round(f * (float)pow(10, prec)) / (float)pow(10, prec));
	}

	// Method to display moving "*" while source frequency isn't defined/stable.
	void playFreqFindAnimation() {
		char text[20] = "        ";
		if (blinkSyncAnim < 8)
			text[blinkSyncAnim] = '*';
			else text[14 - blinkSyncAnim] = '*';
		strcpy(dmdMessage2, text);
	}

	int getNotebyFreq(float freq) {
		int aPosition = -1;
		for (int i = 0; i < 120; i++) {
			if ((freq >= (noteToFreq[i] - (noteToFreq[i] * 0.08f))) && (freq <= (noteToFreq[i] + (noteToFreq[i] * 0.08f)))) {
				aPosition = i;
				i = 120;
			}
		}
		return aPosition;
	}

	// Persistence for extra datas, via json functions.
	// These extra datas are saved to .vcv file (including "autosave.vcv") also are "transfered" when you duplicate the module.

	json_t *toJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme)); // Save selected module theme (GUI).
		json_object_set_new(rootJ, "Mode", json_integer(Mode)); // Save current mode.
		json_object_set_new(rootJ, "fmSubMode", json_integer(fmSubMode)); // Save current submode for frequency counter.
		json_object_set_new(rootJ, "vmDecimals", json_integer(vmDecimals)); // Save number of decimals for voltmeter.
		json_object_set_new(rootJ, "triggerVoltage", json_real(triggerVoltage)); // Save trigger voltage for input port (default is +1.7V).
		return rootJ;
	}

	void fromJson(json_t *rootJ) override	{
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
		// Retrieving measuring mode.
		json_t *ModeJ = json_object_get(rootJ, "Mode");
		if (ModeJ)
			Mode = json_integer_value(ModeJ);
		// Retrieving frequency counter sub mode.
		json_t *fmSubModeJ = json_object_get(rootJ, "fmSubMode");
		if (fmSubModeJ)
			fmSubMode = json_integer_value(fmSubModeJ);
		// Retrieving number of decimal set for voltmeter.
		json_t *vmDecimalsJ = json_object_get(rootJ, "vmDecimals");
		if (vmDecimalsJ)
			vmDecimals = json_integer_value(vmDecimalsJ);
		// Retrieving trigger voltage for peak counter mode (real/float value).
		json_t *triggerVoltageJ = json_object_get(rootJ, "triggerVoltage");
		if (triggerVoltageJ)
			triggerVoltage = json_real_value(triggerVoltageJ);
	}

};

void Metriks::step() {
	// step() function is the right place for DSP processing!

	// Depending current Metriks model (theme), set the relevant DMD-text color.
	DMDtextColor = tblDMDtextColor[Theme];

	// Bypass early steps, due to encoder/knob behaviors on init (from 0.0f... to json saved value!).
	if (onFirstInitCounter > 0) {
		currentStep++;
		if ((currentStep % 8000) > 4000)
			strcpy(dmdMessage1, "Calibrating...");
			else strcpy(dmdMessage1, "");
		xOffsetValue = 2;
		strcpy(dmdMessage2, "");

		encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].value);
		if (encoderPrevious != encoderCurrent) {
			onFirstInitCounter = onFirstInitCounter + (int)(engineGetSampleRate() / 512.0f);
			encoderPrevious = encoderCurrent;
		}
		else onFirstInitCounter--;

		if (onFirstInitCounter == 1) {
			// This is the lastest step of initialization.
			// Reinit encoder reading.
			encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].value);
			encoderPrevious = encoderCurrent;
			encoderDelta = 0; // Default assuming encoder isn't moved.
			//
			currentStep = 0;
			onFirstInitCounter = 0;
		}
		return;
	}

	// Read if the encoder have moved or not.
	encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].value);
	if (encoderCurrent != encoderPrevious) {
		if (abs(encoderCurrent - encoderPrevious) <= 2) {
			// Depending current mode, this will change setting, like:
			// - Number of displayed decimals for voltmeter.
			// - Threshold voltage for peak counter.
			switch (Mode) {
				case 0:
					// Voltmeter: changing decimals.
					if (encoderCurrent < encoderPrevious) {
						// Decrementing decimals...
						vmDecimals--;
						if (vmDecimals < 0)
							vmDecimals = 0; // Min: 0.
					}
					else {
						// Incrementing decimals...
						vmDecimals++;
						if (vmDecimals > 3)
							vmDecimals = 3; // Min: 0.
					}
					// Prepare display and start timer (only if not switching mode).
					if (switchedModeCounter == 0) {
						strcpy(dmdMessage1, "Decimals");
						xOffsetValue = 42;
						snprintf(dmdMessage2, sizeof(dmdMessage2), "%i", vmDecimals);
						// Start display timer...
						encoderDisplayCounter = (int)(2 * engineGetSampleRate());
					}
					break;
				case 1:
					// Frequency counter: changing submode.
					if (encoderCurrent < encoderPrevious) {
						// Decrementing: select previous submode...
						fmSubMode--;
						if (fmSubMode < 0)
							fmSubMode = 2; // Return to BPM meter...
					}
					else {
						// Decrementing: select next submode...
						fmSubMode++;
						if (fmSubMode > 2)
							fmSubMode = 0; // Return to frequency counter...
					}
					// Prepare display and start timer (only if not switching mode).
					if (switchedModeCounter == 0) {
						switch (fmSubMode) {
							case 0:
								strcpy(dmdMessage1, "Freq. Counter");
								break;
							case 1:
								strcpy(dmdMessage1, "Note Tuner");
								break;
							case 2:
								strcpy(dmdMessage1, "BPM Meter");
						}
					}
					break;
				case 2:
					// Peak counter: changing threshold (trigger) voltage.
					if (encoderCurrent < encoderPrevious) {
						// Decrementing thresold voltage by 0.1... minimum is 0.2V.
						triggerVoltage = triggerVoltage - 0.1f;
						if (triggerVoltage < 0.2f)
							triggerVoltage = 0.2f; // Mininum voltage: 0.2V.
					}
					else {
						// Incrementing thresold voltage by 0.1...
						triggerVoltage = triggerVoltage + 0.1f;
						if (triggerVoltage > 12.0f)
							triggerVoltage = 12.0f; // Maximum voltage: 12V.
					}
					// Prepare display and start timer (only if not switching mode).
					if (switchedModeCounter == 0) {
						strcpy(dmdMessage1, "Threshold");
						triggerVoltage = round(triggerVoltage * 10.0f) / 10.0f;
						if (triggerVoltage < 10.0f)
							xOffsetValue = 36;
							else xOffsetValue = 24;
						snprintf(dmdMessage2, sizeof(dmdMessage2), "%2.1fV", triggerVoltage);
						// Start display timer...
						encoderDisplayCounter = (int)(2 * engineGetSampleRate());
					}
			}
		}
		// Save current encoder position to become previous (for next check).
		encoderPrevious = encoderCurrent;
	}

	// MODE button trigger: change mode when pressed.
	if (modeButton.process(params[BUTTON_MODE].value)) {
		modeButton.reset();
		switchedModeCounter = (int)(2 * engineGetSampleRate());
		Mode++;
		if (Mode > 2)
			Mode = 0;
	}

	bSourceActive = inputs[INPUT_SOURCE].active;
	currentVoltage = bSourceActive ? inputs[INPUT_SOURCE].value : 0.0f;

	if (bSourceActive) {
		// Transmit (as passthru) source voltage directly to output port. This may useful to insert the Metriks instrument module in chain.
		outputs[OUTPUT_THRU].value = currentVoltage;
		if (Mode == 0) {
			// Voltmeter: just unlit PLAY/PAUSE LED, that's all.
			lights[LED_PLAY_GREEN].value = 0.0;
			lights[LED_PLAY_RED].value = 0.0;
		}
		else if (Mode == 1) {
			// Frequency counter / note tuner / BPM meter.
			// Increment step number.
			currentStep++;
			// Using Schmitt trigger (SchmittTrigger is provided by dsp/digital.hpp) to detect triggers on CLK input jack.
			// It uses "triggerVoltage" value for trigger: default +1.7V, or redefined from Peak Counter mode (min +0.2V)!
			if (inputPort.process(rescale(inputs[INPUT_SOURCE].value, 0.1f, triggerVoltage, 0.0f, 1.0f))) {
				// It's a rising edge.
				if (previousStep == 0) {
					// Source CLK frequency is unknown.
					currentFreq = -1.0f; // Set to -1.0f by this way it will displayed as "Unknown" frequency.
					expectedStep = 0;
				}
				else {
					// But perhaps this rising edge comes too early (source frequency is increased).
					if (currentFreq != -1.0f) {
						if ((currentStep != expectedStep) && (currentStep != (expectedStep + 1))  && (currentStep != (expectedStep - 1))) {
							currentFreq = -1.0f; // Forced to -1.0f by this way it will displayed as "Unknown" frequency.
							expectedStep = 0;
							currentStep = 1;
						}
						else {
							// Source CLK frequency is stable.
							currentFreq = engineGetSampleRate() / (float)(currentStep - previousStep);
							expectedStep = currentStep - previousStep + currentStep;
						}
					}
					else {
						// Source CLK frequency was unknow at previous rising edge, but for now it can be established on this (next) rising edge.
						currentFreq = engineGetSampleRate() / (float)(currentStep - previousStep);
						expectedStep = currentStep - previousStep + currentStep;
					}
				}
				previousStep = currentStep;
			}
		}
		else if (Mode == 2) {
			// Peak counter mode.
			if (bIsPlaying) {
				if (inputPort.process(rescale(inputs[INPUT_SOURCE].value, 0.1f, triggerVoltage, 0.0f, 1.0f))) {
					currentPeakCount++;
					if (currentPeakCount > 99999999)
						currentPeakCount = 100000000;
				}
			}
			// Play/Pause via button or via trigger signal on related port.
			if (playButton.process(params[BUTTON_PLAYPAUSE].value) || playPort.process(rescale(inputs[INPUT_PLAYPAUSE].value, 0.2f, 1.7f, 0.0f, 1.0f))) {
				playButton.reset();
				bIsPlaying = !bIsPlaying;
			}
			lights[LED_PLAY_GREEN].value = bIsPlaying ? 1.0 : 0.0;
			lights[LED_PLAY_RED].value = bIsPlaying ? 0.0 : 1.0;
			// RST via button or via trigger signal on related port.
			if (resetButton.process(params[BUTTON_RESET].value) || resetPort.process(rescale(inputs[INPUT_RESET].value, 0.2f, 1.7f, 0.0f, 1.0f))) {
				resetButton.reset();
				// Reset Peak counter.
				currentPeakCount = 0;
			}
		}
	}
	else {
		// Input port isn't connected.
		// Sending 0V to output port.
		outputs[OUTPUT_THRU].value = 0;
		// Turn off PLAY/PAUSE LED.
		lights[LED_PLAY_GREEN].value = 0.0;
		lights[LED_PLAY_RED].value = 0.0;
		// Reset some useless counters/flags with input port isn't connected.
		currentFreq = -1.0f; // -1.0f means unknown frequency.
		currentPeakCount = 0;
		bIsPlaying = false;
		currentStep = 0;
		previousStep = 0;
		expectedStep = 0;
	}

	if (switchedModeCounter != 0) {
		// Currently switching mode: displaying new mode selected for a certain delay.
		// Cancelling other temporary displays (mode switching have higher priority).
		encoderDisplayCounter = 0;
		bBlinkErr = false;
		// Decrementing timer counter...
		switchedModeCounter--;
		// Display new mode for a given delay.
		strcpy(dmdMessage1, "Switch To >");
		switch (Mode) {
			case 0:
				// Voltmeter.
				xOffsetValue = 3;
				strcpy(dmdMessage2, "Voltmeter");
				break;
			case 1:
				// Frequency counter.
				xOffsetValue = 0;
				// Frequency counter / Note tuner / BPM meter.
				switch (fmSubMode) {
					case 0:
						strcpy(dmdMessage2, "Freq. Cnt.");
						break;
					case 1:
						strcpy(dmdMessage2, "Note Tun.");
						break;
					case 2:
						strcpy(dmdMessage2, "BPM Meter");
				}
				break;
			case 2:
				// Peak counter. //DONE
				xOffsetValue = 1;
				strcpy(dmdMessage2, "Peak Cnt.");
		}
	}
	else if (encoderDisplayCounter != 0) {
		// Parameter changed via encoder: holding display the parameter during short time.
		bBlinkErr = false;
		encoderDisplayCounter--;
	}
	else {
		// Normal display mode (depending current mode and its parameter).
		switch (Mode) {
			case 0:
				// Voltmeter.
				strcpy(dmdMessage1, "Voltmeter");
				if (bSourceActive) {
					// Doing a voltage range limitation, regardling actual number of decimals we're using.
					// Doing via "switch" will reduce CPU loads (instead of math formula).
					float vFloor;
					float vCeiling;
					switch (vmDecimals) {
						case 0:
							currentVoltage = clamp(currentVoltage, -9999.9f, 99999.4f);
							vFloor = -9999.0f;
							vCeiling = 99999.3f; // Positive, we can display one more digit!
							break;
						case 1:
							currentVoltage = clamp(currentVoltage, -999.99f, 999.94f);
							vFloor = -999.90f;
							vCeiling = 999.93f;
							break;
						case 2:
							currentVoltage = clamp(currentVoltage, -999.999f, 999.994f);
							vFloor = -999.990f;
							vCeiling = 999.993f;
							break;
						case 3:
							currentVoltage = clamp(currentVoltage, -99.9999f, 99.9994f);
							vFloor = -99.9990f;
							vCeiling = 99.9993f;
							break;
					}
					if ((currentVoltage < vFloor) || (currentVoltage > vCeiling)) {
						// Voltage is out of range (overflow).
						xOffsetValue = 3;
						if (!bBlinkErr) {
							bBlinkErr = true;
							bBlinkErrAlt = true;
							bBlinkErrTimer = round(engineGetSampleRate() / 4);
						}
						else {
							if (bBlinkErrTimer > 0) {
								bBlinkErrTimer--; // Delaying...
							}
							else {
								bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
								bBlinkErrTimer = round(engineGetSampleRate() / 4);
							}
							if (bBlinkErrAlt)
								strcpy(dmdMessage2, "!Overflow!");
								else strcpy(dmdMessage2, "");
						}
					}
					else {
						// Second stage of voltage processing (voltage is in range).
						// Doing a decimal precision rounding.
						currentVoltage = roundp(currentVoltage, vmDecimals);
						switch (vmDecimals) {
							case 0:
								// Avoiding "-0"...
								if (int(currentVoltage) == 0)
									currentVoltage = 0.0f;
								xOffsetValue = 14;
								snprintf(dmdMessage2, sizeof(dmdMessage2), "%5.0fV", currentVoltage);
								break;
							case 1:
								// Avoiding "-0.0"...
								if (int(currentVoltage * 10) == 0)
									currentVoltage = 0.0f;
								//
								xOffsetValue = 20;
								if (currentVoltage <= -100.0f)
									xOffsetValue = 8;
								snprintf(dmdMessage2, sizeof(dmdMessage2), "%5.1fV", currentVoltage);
								break;
							case 2:
								// Avoiding "-0.00"...
								if (int(currentVoltage * 100) == 0)
									currentVoltage = 0.0f;
								//
								xOffsetValue = 38;
								if (currentVoltage <= -100.0f)
									xOffsetValue = 2;
									else if (currentVoltage <= -10.0f)
										xOffsetValue = 14;
										else if (currentVoltage < 0.00f)
											xOffsetValue = 26;
											else if (currentVoltage >= 100.0f)
												xOffsetValue = 14;
												else if (currentVoltage >= 10.0f)
													xOffsetValue = 26;
								snprintf(dmdMessage2, sizeof(dmdMessage2), "%4.2fV", currentVoltage);
								break;
							case 3:
								// Avoiding "-0.000"...
								if (int(currentVoltage * 1000) == 0)
									currentVoltage = 0.0f;
								//
								xOffsetValue = 26;
								if (currentVoltage <= -10.0f)
									xOffsetValue = 2;
									else if (currentVoltage < 0.0f)
										xOffsetValue = 14;
										else if (currentVoltage >= 10.0f)
											xOffsetValue = 14;
								snprintf(dmdMessage2, sizeof(dmdMessage2), "%3.3fV", currentVoltage);
						}
					}
				}
				else {
					// Input port isn't connected (not active).
					xOffsetValue = -1;
					if (!bBlinkErr) {
						bBlinkErr = true;
						bBlinkErrAlt = true;
						bBlinkErrTimer = round(engineGetSampleRate() / 4);
					}
					else {
						if (bBlinkErrTimer > 0) {
							bBlinkErrTimer--; // Delaying...
						}
						else {
							bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
							bBlinkErrTimer = round(engineGetSampleRate() / 4);
						}
						if (bBlinkErrAlt)
							strcpy(dmdMessage2, "? Input ?");
							else strcpy(dmdMessage2, "");
					}
				}
				break;
			case 1:
				// Frequency counter / Note tuner / BPM meter.
				switch (fmSubMode) {
					case 0:
						strcpy(dmdMessage1, "Freq. Counter");
						break;
					case 1:
						strcpy(dmdMessage1, "Note Tuner");
						break;
					case 2:
						strcpy(dmdMessage1, "BPM Meter");
				}
				if (bSourceActive) {
					switch (fmSubMode) {
						case 0: // Frequency counter.
							if (currentFreq < 0.0f) {
								xOffsetValue = 1;
								if (!bBlinkErr) {
									bBlinkErr = true;
									blinkSyncAnim = 0;
									bBlinkErrTimer = round(engineGetSampleRate() / 32);
								}
								else {
									if (bBlinkErrTimer > 0) {
										bBlinkErrTimer--; // Delaying...
									}
									else {
										blinkSyncAnim++;
										if (blinkSyncAnim > 13)
											blinkSyncAnim = 0;
										bBlinkErrTimer = round(engineGetSampleRate() / 32);
									}
									playFreqFindAnimation();
								}
							}
							else if (currentFreq > 99999.8f) {
								xOffsetValue = 3;
								if (!bBlinkErr) {
									bBlinkErr = true;
									bBlinkErrAlt = true;
									bBlinkErrTimer = round(engineGetSampleRate() / 4);
								}
								else {
									if (bBlinkErrTimer > 0) {
										bBlinkErrTimer--; // Delaying...
									}
									else {
										bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
										bBlinkErrTimer = round(engineGetSampleRate() / 4);
									}
									if (bBlinkErrAlt)
										strcpy(dmdMessage2, "!Overflow!");
										else strcpy(dmdMessage2, "");
								}
							}
							else if (currentFreq >= 1000.0f) {
								xOffsetValue = 13;
								snprintf(dmdMessage2, sizeof(dmdMessage2), "%5.0fHz", currentFreq);
							}
							else {
								xOffsetValue = 19;
								snprintf(dmdMessage2, sizeof(dmdMessage2), "%5.1fHz", currentFreq);
							}
							break;
						case 1:
							// Note Tuner.
							if ((currentFreq > 15.534f) && (currentFreq < 16594.4f)) {
								int iNote = 0;
								iNote = getNotebyFreq(currentFreq);
								if (iNote == -1)
									iNote = getNotebyFreq(currentFreq + (currentFreq * 0.12f));
								if (iNote == -1)
									iNote = getNotebyFreq(currentFreq - (currentFreq * 0.12f));
								if (iNote > -1) {
									std::string str = noteName[iNote];
									char *cstr = new char[str.length() + 1];
									strcpy(cstr, str.c_str());
									if (str.length() == 3)
										xOffsetValue = 31;
										else xOffsetValue = 8;
									strcpy(dmdMessage2, cstr);
									delete [] cstr;
								}
								else {
									xOffsetValue = 32;
									strcpy(dmdMessage2, "???");
								}
							}
							else {
								// Unknown frequency...
								xOffsetValue = 32;
								strcpy(dmdMessage2, "");
							}
							break;
						case 2:
							// Submode BPM Meter...
							if (currentFreq < 0.0f) {
								xOffsetValue = 1;
								if (!bBlinkErr) {
									bBlinkErr = true;
									blinkSyncAnim = 0;
									bBlinkErrTimer = round(engineGetSampleRate() / 32);
								}
								else {
									if (bBlinkErrTimer > 0) {
										bBlinkErrTimer--; // Delaying...
									}
									else {
										blinkSyncAnim++;
										if (blinkSyncAnim > 13)
											blinkSyncAnim = 0;
										bBlinkErrTimer = round(engineGetSampleRate() / 32);
									}
									playFreqFindAnimation();
								}
							}
							else if (currentFreq > 99999.8f) {
								xOffsetValue = 3;
								if (!bBlinkErr) {
									bBlinkErr = true;
									bBlinkErrAlt = true;
									bBlinkErrTimer = round(engineGetSampleRate() / 4);
								}
								else {
									if (bBlinkErrTimer > 0) {
										bBlinkErrTimer--; // Delaying...
									}
									else {
										bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
										bBlinkErrTimer = round(engineGetSampleRate() / 4);
									}
									if (bBlinkErrAlt)
										strcpy(dmdMessage2, "!Overflow!");
										else strcpy(dmdMessage2, "");
								}
							}
							else {
								float fBPM = round(currentFreq * 600.0f) / 10.0f;
								if (fBPM < 999999.5f) {
									if (fBPM >= 100000.0f)
										xOffsetValue = 5;
										else if (fBPM >= 10000.0f)
											xOffsetValue = 17;
											else xOffsetValue = 29;
									snprintf(dmdMessage2, sizeof(dmdMessage2), "%6.1f", fBPM);
								}
								else {
									xOffsetValue = 3;
									if (!bBlinkErr) {
										bBlinkErr = true;
										bBlinkErrAlt = true;
										bBlinkErrTimer = round(engineGetSampleRate() / 4);
									}
									else {
										if (bBlinkErrTimer > 0) {
											bBlinkErrTimer--; // Delaying...
										}
										else {
											bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
											bBlinkErrTimer = round(engineGetSampleRate() / 4);
										}
										if (bBlinkErrAlt)
											strcpy(dmdMessage2, "!Overflow!");
											else strcpy(dmdMessage2, "");
									}
								}
							}
					}
				}
				else {
					xOffsetValue = -1;
					if (!bBlinkErr) {
						bBlinkErr = true;
						bBlinkErrAlt = true;
						bBlinkErrTimer = round(engineGetSampleRate() / 4);
					}
					else {
						if (bBlinkErrTimer > 0) {
							bBlinkErrTimer--; // Delaying...
						}
						else {
							bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
							bBlinkErrTimer = round(engineGetSampleRate() / 4);
						}
						if (bBlinkErrAlt)
							strcpy(dmdMessage2, "? Input ?");
							else strcpy(dmdMessage2, "");
					}
				}
				break;
			case 2:
				// Pulse counter.
				strcpy(dmdMessage1, "Peak Counter");
				if (bSourceActive) {
					// Displaying current peak counter.
					if (currentPeakCount > 99999999) {
						currentPeakCount = 100000000;
						xOffsetValue = 3;
						if (!bBlinkErr) {
							bBlinkErr = true;
							bBlinkErrAlt = true;
							bBlinkErrTimer = round(engineGetSampleRate() / 4);
						}
						else {
							if (bBlinkErrTimer > 0) {
								bBlinkErrTimer--; // Delaying...
							}
							else {
								bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
								bBlinkErrTimer = round(engineGetSampleRate() / 4);
							}
							if (bBlinkErrAlt)
								strcpy(dmdMessage2, "!Overflow!");
								else strcpy(dmdMessage2, "");
						}
					}
					else {
						xOffsetValue = 2;
						snprintf(dmdMessage2, sizeof(dmdMessage2), "%8lu", currentPeakCount);
					}
				}
				else {
					xOffsetValue = -1;
					if (!bBlinkErr) {
						bBlinkErr = true;
						bBlinkErrAlt = true;
						bBlinkErrTimer = round(engineGetSampleRate() / 4);
					}
					else {
						if (bBlinkErrTimer > 0) {
							bBlinkErrTimer--; // Delaying...
						}
						else {
							bBlinkErrAlt = !bBlinkErrAlt; // Alternate display.
							bBlinkErrTimer = round(engineGetSampleRate() / 4);
						}
						if (bBlinkErrAlt)
							strcpy(dmdMessage2, "? Input ?");
							else strcpy(dmdMessage2, "");
					}
				}
		}
	}

}

// Dot-matrix display (DMD) handler. Mainly hardcoded for best performances.
struct MetriksDMD : TransparentWidget {
	Metriks *module;
	std::shared_ptr<Font> font;
	MetriksDMD() {
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

	void draw(NVGcontext *vg) override {
		updateDMD1(vg, Vec(14, box.size.y - 174), module->DMDtextColor, module->dmdMessage1);
		updateDMD2(vg, Vec(12, box.size.y - 152), module->DMDtextColor, module->xOffsetValue, module->dmdMessage2);
	}

};

struct MetriksWidget : ModuleWidget {
	// Themed plates.
	SVGPanel *panelClassic;
	SVGPanel *panelStageRepro;
	SVGPanel *panelAbsoluteNight;
	SVGPanel *panelDarkSignature;
	SVGPanel *panelDeepBlueSignature;
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
	// Silver MODE button.
	SVGSwitch *buttonModeSilver;
	// Gold MODE button.
	SVGSwitch *buttonModeGold;
	// Silver PLAY/PAUSE (toggle) button.
	SVGSwitch *buttonPlayPauseSilver;
	// Gold PLAY/PAUSE (toggle) button.
	SVGSwitch *buttonPlayPauseGold;
	// Silver RESET button.
	SVGSwitch *buttonResetSilver;
	// Gold RESET button.
	SVGSwitch *buttonResetGold;
	//
	MetriksWidget(Metriks *module);
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

MetriksWidget::MetriksWidget(Metriks *module) : ModuleWidget(module) {
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	// Model: Classic (beige plate, always default GUI when added from modules menu).
	panelClassic = new SVGPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(SVG::load(assetPlugin(plugin, "res/Metriks_Classic.svg")));
	addChild(panelClassic);
	// Model: Stage Repro.
	panelStageRepro = new SVGPanel();
	panelStageRepro->box.size = box.size;
	panelStageRepro->setBackground(SVG::load(assetPlugin(plugin, "res/Metriks_Stage_Repro.svg")));
	addChild(panelStageRepro);
	// Model: Absolute Night.
	panelAbsoluteNight = new SVGPanel();
	panelAbsoluteNight->box.size = box.size;
	panelAbsoluteNight->setBackground(SVG::load(assetPlugin(plugin, "res/Metriks_Absolute_Night.svg")));
	addChild(panelAbsoluteNight);
	// Model: Dark "Signature".
	panelDarkSignature = new SVGPanel();
	panelDarkSignature->box.size = box.size;
	panelDarkSignature->setBackground(SVG::load(assetPlugin(plugin, "res/Metriks_Dark_Signature.svg")));
	addChild(panelDarkSignature);
	// Model: Deepblue "Signature".
	panelDeepBlueSignature = new SVGPanel();
	panelDeepBlueSignature->box.size = box.size;
	panelDeepBlueSignature->setBackground(SVG::load(assetPlugin(plugin, "res/Metriks_Deepblue_Signature.svg")));
	addChild(panelDeepBlueSignature);
	// Model: Carbon "Signature".
	panelCarbonSignature = new SVGPanel();
	panelCarbonSignature->box.size = box.size;
	panelCarbonSignature->setBackground(SVG::load(assetPlugin(plugin, "res/Metriks_Carbon_Signature.svg")));
	addChild(panelCarbonSignature);
	// Always four screws for 12 HP module, may are silver or gold, depending model.
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
	// DMD Display.
	{
		MetriksDMD *display = new MetriksDMD();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 234);
		addChild(display);
	}
	// Input ports (golden jacks).
	addInput(Port::create<PJ301M_In>(Vec(24, 304), Port::INPUT, module, Metriks::INPUT_SOURCE));
	addInput(Port::create<PJ301M_In>(Vec(24, 262), Port::INPUT, module, Metriks::INPUT_PLAYPAUSE));
	addInput(Port::create<PJ301M_In>(Vec(72, 262), Port::INPUT, module, Metriks::INPUT_RESET));
	// Output port (golden jacks).
	addOutput(Port::create<PJ301M_Out>(Vec(72, 304), Port::OUTPUT, module, Metriks::OUTPUT_THRU));
	// Encoder (using same encoder than KlokSpid module).
	//addParam(ParamWidget::create<KS_Encoder>(Vec(20, 106), module, Metriks::PARAM_ENCODER, -INFINITY, +INFINITY, 0.0));
	module->mtksEncoder = dynamic_cast<Knob*>(Knob::create<KS_Encoder>(Vec(20, 106), module, Metriks::PARAM_ENCODER, -INFINITY, +INFINITY, 0.0));
	addParam(module->mtksEncoder);
	// Push button (silver) used to select MODE (same used by KlokSpid modules).
	buttonModeSilver = ParamWidget::create<KS_ButtonSilver>(Vec(94, 178), module, Metriks::BUTTON_MODE, 0.0, 1.0, 0.0);
	addParam(buttonModeSilver);
	// Push button (gold) used to select MODE (same used by KlokSpid modules).
	buttonModeGold = ParamWidget::create<KS_ButtonGold>(Vec(94, 178), module, Metriks::BUTTON_MODE, 0.0, 1.0, 0.0);
	addParam(buttonModeGold);
	// Push button (silver) above toggle input port, used to toggle PLAY/PAUSE (same used by KlokSpid modules).
	buttonPlayPauseSilver = ParamWidget::create<KS_ButtonSilver>(Vec(27.4, 240), module, Metriks::BUTTON_PLAYPAUSE, 0.0, 1.0, 0.0);
	addParam(buttonPlayPauseSilver);
	// Push button (gold) above toggle input port, used to toggle PLAY/PAUSE (same used by KlokSpid modules).
	buttonPlayPauseGold = ParamWidget::create<KS_ButtonGold>(Vec(27.4, 240), module, Metriks::BUTTON_PLAYPAUSE, 0.0, 1.0, 0.0);
	addParam(buttonPlayPauseGold);
	// Push button (silver) above RST input port, used to RESET peak counter (same used by KlokSpid modules).
	buttonResetSilver = ParamWidget::create<KS_ButtonSilver>(Vec(75.4, 240), module, Metriks::BUTTON_RESET, 0.0, 1.0, 0.0);
	addParam(buttonResetSilver);
	// Push button (gold) above RST input port, used to RESET peak counter (same used by KlokSpid modules).
	buttonResetGold = ParamWidget::create<KS_ButtonGold>(Vec(75.4, 240), module, Metriks::BUTTON_RESET, 0.0, 1.0, 0.0);
	addParam(buttonResetGold);
	// PLAY/STOP (bicolored green/red) LED.
	addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(18, 252), module, Metriks::LED_PLAY_GREEN));
}

//// Make one visible theme at once!

void MetriksWidget::step() {
	Metriks *metriks = dynamic_cast<Metriks*>(module);
	assert(metriks);
	// "Signature"-line modules are using gold parts (instead of silver).
	bool isSignatureLine = (metriks->Theme > 2);
	// Themed module plate, selected via context-menu, is visible (all others are obviously, hidden).
	panelClassic->visible = (metriks->Theme == 0);
	panelStageRepro->visible = (metriks->Theme == 1);
	panelAbsoluteNight->visible = (metriks->Theme == 2);
	panelDarkSignature->visible = (metriks->Theme == 3);
	panelDeepBlueSignature->visible = (metriks->Theme == 4);
	panelCarbonSignature->visible = (metriks->Theme == 5);
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
	// Silver or gold MODE button is visible at once (opposite is, obvisouly, hidden).
	buttonModeSilver->visible = !isSignatureLine;
	buttonModeGold->visible = isSignatureLine;
	// Silver or gold PLAY/PAUSE button is visible at once (opposite is, obvisouly, hidden).
	buttonPlayPauseSilver->visible = !isSignatureLine;
	buttonPlayPauseGold->visible = isSignatureLine;
	// Silver or gold RESET button is visible at once (opposite is, obvisouly, hidden).
	buttonResetSilver->visible = !isSignatureLine;
	buttonResetGold->visible = isSignatureLine;
	// Resume original step() method.
	ModuleWidget::step();
}


//// CONTEXT-MENU (RIGHT-CLICK ON MODULE).

// Classic (default beige) module.
struct metriksClassicMenu : MenuItem {
	Metriks *metriks;
	void onAction(EventAction &e) override {
		metriks->Theme = 0;
	}
	void step() override {
		rightText = (metriks->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

// Stage Repro module.
struct metriksStageReproMenu : MenuItem {
	Metriks *metriks;
	void onAction(EventAction &e) override {
		metriks->Theme = 1;
	}
	void step() override {
		rightText = (metriks->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

// Absolute Night module.
struct metriksAbsoluteNightMenu : MenuItem {
	Metriks *metriks;
	void onAction(EventAction &e) override {
		metriks->Theme = 2;
	}
	void step() override {
		rightText = (metriks->Theme == 2) ? "✔" : "";
		MenuItem::step();
	}
};

// Dark "Signature" module.
struct metriksDarkSignatureMenu : MenuItem {
	Metriks *metriks;
	void onAction(EventAction &e) override {
		metriks->Theme = 3;
	}
	void step() override {
		rightText = (metriks->Theme == 3) ? "✔" : "";
		MenuItem::step();
	}
};

// Deepblue "Signature" module.
struct metriksDeepBlueSignatureMenu : MenuItem {
	Metriks *metriks;
	void onAction(EventAction &e) override {
		metriks->Theme = 4;
	}
	void step() override {
		rightText = (metriks->Theme == 4) ? "✔" : "";
		MenuItem::step();
	}
};

// Carbon "Signature" module.
struct metriksCarbonSignatureMenu : MenuItem {
	Metriks *metriks;
	void onAction(EventAction &e) override {
		metriks->Theme = 5;
	}
	void step() override {
		rightText = (metriks->Theme == 5) ? "✔" : "";
		MenuItem::step();
	}
};

// CONTEXT-MENU CONSTRUCTION.

Menu* MetriksWidget::createContextMenu() {
	Menu* menu = ModuleWidget::createContextMenu();
	Metriks *metriks = dynamic_cast<Metriks*>(module);
	assert(metriks);
	menu->addChild(construct<MenuEntry>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Model:"));
	menu->addChild(construct<metriksClassicMenu>(&metriksClassicMenu::text, "Classic (default)", &metriksClassicMenu::metriks, metriks));
	menu->addChild(construct<metriksStageReproMenu>(&metriksStageReproMenu::text, "Stage Repro", &metriksStageReproMenu::metriks, metriks));
	menu->addChild(construct<metriksAbsoluteNightMenu>(&metriksAbsoluteNightMenu::text, "Absolute Night", &metriksAbsoluteNightMenu::metriks, metriks));
	menu->addChild(construct<metriksDarkSignatureMenu>(&metriksDarkSignatureMenu::text, "Dark \"Signature\"", &metriksDarkSignatureMenu::metriks, metriks));
	menu->addChild(construct<metriksDeepBlueSignatureMenu>(&metriksDeepBlueSignatureMenu::text, "Deepblue \"Signature\"", &metriksDeepBlueSignatureMenu::metriks, metriks));
	menu->addChild(construct<metriksCarbonSignatureMenu>(&metriksCarbonSignatureMenu::text, "Carbon \"Signature\"", &metriksCarbonSignatureMenu::metriks, metriks));
	return menu;
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, Metriks) {
   Model *modelMetriks = Model::create<Metriks, MetriksWidget>("Ohmer Modules", "Metriks", "Metriks", VISUAL_TAG);
   return modelMetriks;
}
