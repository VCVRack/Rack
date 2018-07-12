///////////////////////////////////////////////////////////////////////////////////////////////////////
////// RKD with "Break" is a 2x 4 HP module, combining RKD module (4 HP) and "BRK" panel (4 HP). //////
////// The "Break" panel provides six switches to access "jumpers" (setting) more easily         //////
////// (without need to access jumpers located on module's PCB!).                                //////
///////////////////////////////////////////////////////////////////////////////////////////////////////
////// Inspired from existing Eurorack hardware RCDBO combo module, by 4ms Company.              //////
////// Done with restricted 4ms Company permission (thank you 4ms).                              //////
////// This module uses its own algorithm (no original part of firmware code was used).          //////
////// 4ms Company name, logo, RCD, RCDBO, Rotating Clock Divider & RCD Breakout as TRADEMARKED! //////
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"
#include <dsp/digital.hpp>

namespace rack_plugin_Ohmer {

struct RKDBRK : Module {
	enum ParamIds {
		JUMPER_COUNTINGDOWN,
		JUMPER_GATE,
		JUMPER_MAXDIVRANGE16,
		JUMPER_MAXDIVRANGE32,
		JUMPER_SPREAD,
		JUMPER_AUTORESET,
		NUM_PARAMS
	};
	enum InputIds {
		ROTATE_INPUT,
		RESET_INPUT,
		CLK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		OUTPUT_7,
		OUTPUT_8,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_OUT_1,
		LED_OUT_2,
		LED_OUT_3,
		LED_OUT_4,
		LED_OUT_5,
		LED_OUT_6,
		LED_OUT_7,
		LED_OUT_8,
		LED_CLK,
		LED_RESET_RED,
		LED_RESET_ORANGE,
		LED_RESET_BLUE,
		NUM_LIGHTS
	};
	RKDBRK() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	// NOT USED FOR RKD+BRK MODULE! (this flag indicates if jumpers (PCB) is visible, or not).
	//bool bViewPCB = false; NOT USED FOR RKD+BRK MODULE!
	// This flag is set when module is running (CLK jack is wired).
	bool bCLKisActive = false;
	// Schmitt trigger, for RESET input port.
	SchmittTrigger RESET_Port;
	// Schmitt trigger, for CLK input port.
	SchmittTrigger CLK_Port;
	// This flag, when true, indicates the CLK rising edge at the current step.
	bool bIsRisingEdge = false;
	// Next incoming rising edge will be the first rising edge. Required to handle gate modes together with counting up or down.
	bool bIsEarlyRisingEdge = true;
	// This flag, when true, indicates the CLK falling edge at the current step.
	bool bIsFallingEdge = false;
	// This flag, when true, indicates the CLK is high (voltage equal or higher +2V).
	bool bCLKisHigh = false;
	// Assumed timeout at start.
	bool bCLKTimeOut = true;
	// Default jumpers/switches setting (false = Off, true = On).
	bool jmprCountingDown = false; // Factory is Off: Counting Up.
	bool jmprCountingDownPrevious = false;
	bool jmprGate = false; // Factory is Off: Trig.
	bool jmprGatePrevious = false;
	bool jmprMaxDivRange16 = true; // Factory is On (combined with Max-Div-Range 32, also On by default): Max Div amount = 8.
	bool jmprMaxDivRange16Previous = true;
	bool jmprMaxDivRange32 = true; // Factory is On (combined with Max-Div-Range 16, also On by default): Max Div amount = 8.
	bool jmprMaxDivRange32Previous = true;
	bool jmprSpread = false; // Factory is Off: Spread Off.
	bool jmprSpreadPrevious = false;
	bool jmprAutoReset = false; // Factory is Off = Auto-Reset Off.
	// Table set (0: Manufacturer, 1: Prime numbers, 2: Perfect squares, 3: Fibonacci sequence, 4: Triplet & 16ths).
	int tableSet = 0; // This variable is persistent (json).
	int tableSetPrev = 0; // Used to change detection across consecutive steps.
	// RKD default dividers table.
	int tblDividersR0[NUM_OUTPUTS] = {1, 2, 3, 4, 5, 6, 7, 8}; // default dividers (R+0) when using factory jumpers/switches setting.
	// Prime numbers base table.
	int tblPrimes[18] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61};
	// Perfect squares base table.
	int tblSquares[8] = {1, 4, 9, 16, 25, 36, 49, 64};
	// Fibonacci sequence base table.
	int tblFibonacci[9] = {1, 2, 3, 5, 8, 13, 21, 34, 55};
	// Triplet & 16ths table.
	int tblTripletSixteenths[8] = {1, 2, 3, 4, 8, 16, 32, 64};
	// Current (active) dividers table.
	int tblActiveDividers[NUM_OUTPUTS] = {1, 2, 3, 4, 5, 6, 7, 8};
	// Rotation dividers table (as prepared table).
	// Future dividers table, when rotation is required. Last value of this array will be used as "temp backup/restore", during rotation.
	int tblDividersRt[NUM_OUTPUTS + 1] = {1, 2, 3, 4, 5, 6, 7, 8, 0};
	// When set (armed), indicates table have been changed (eg after jumper/switch change/context-menu table).
	bool bTableChange = true;
	// When set (armed), prepare the rotation (set new dividers table).
	bool bDoRotation = false;
	// When set (armed), doing rotation on next rising-edge (coming on CLK input port).
	bool bDoRotationOnRisingEdge = false;
	// Displayed dividers into segment-LED displays (assuming defaults are "--" because the CLK isn't patched).
	char dispDiv1[3] = "--";
	char dispDiv2[3] = "--";
	char dispDiv3[3] = "--";
	char dispDiv4[3] = "--";
	char dispDiv5[3] = "--";
	char dispDiv6[3] = "--";
	char dispDiv7[3] = "--";
	char dispDiv8[3] = "--";
	// Maximum divide amount, default is 8 (for manufacter table).
	int maxDivAmount = 8;
	// ROTATE (CV) voltage.
	float cvRotate = 0.0f;
	int cvRotateTblIndex = 0;
	int cvRotateTblIndexPrevious = 0;
	// RESET voltage (trigger input port).
	float cvReset = 0.0f;
	bool bResetOnJack = false;
	bool bRegisteredResetOnJack = false;
	// Step-based (sample) counters.
	long long int currentStep = 0;
	long long int previousStep = 0;
	long long int expectedStep = 0;
	// Source (CLK) frequency flag (set when source frequency is known).
	bool bCLKFreqKnown = false;
	// Dividers counters (one per output jack).
	int divCounters[NUM_OUTPUTS] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Global Auto-Reset sequence counter.
	int divCountersAutoReset = 0;
	// This flag is set on "Auto-Reset" event.
	bool bIsAutoReset = false;
	// This flag allow/inhibit Auto-Reset - temporary (Auto-Reset will not fired after a timeout/reset, or a reset done via RESET jack).
	bool bAllowAutoReset = false;
	// This flag is used only for blue RESET (Auto-Reset) LED (too avoid too long flashing LED).
	bool bAutoResetLEDfired = false;
	// True if output jack is fired (pulsing).
	bool bJackIsFired[NUM_OUTPUTS] = {false, false, false, false, false, false, false, false};
	// RESET LED afterglow (0: end of afterglow/unlit LED, other positive values indicate how many steps the LED is lit.
	int ledResetAfterglow = 0;
	//
	// Methods (void functions).
	// DSP method.
	void step() override;
	// While using "Initialize" from context-menu, or by using Ctrl+I/Command+I shortcut over module.
	void reset() override {
		// While using "Initialize" from context-menu (or by using Ctrl+I/Command+I over module).
		this->jmprCountingDown = false;
		jmprCountingDownPrevious = false;
		this->jmprGate = false;
		jmprGatePrevious = false;
		this->jmprMaxDivRange16 = true;
		jmprMaxDivRange16Previous = true;
		this->jmprMaxDivRange32 = true;
		jmprMaxDivRange32Previous = true;
		this->jmprSpread = false;
		jmprSpreadPrevious = false;
		this->jmprAutoReset = false;
		this->tableSet = 0;
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
			tblDividersR0[i] = i + 1; // Default dividers for all output ports (manufacturer table).
		// Default factory maximum divide amount is 8.
		maxDivAmount = 8;
		// Set module in timeout (sleeping) mode, to reset some variables/flags/counters...
		ModuleTimeOut();
	}

	void ModuleTimeOut() {
		// Reset Schmitt trigger used by RESET input jack.
		RESET_Port.reset();
		// Defining trigger thresholds for RESET input jack (rescale).
		//RESET_Port.setThresholds(0.2f, 3.5f);
		bResetOnJack = false;
		bRegisteredResetOnJack = false;
		// Reset Schmitt trigger used by CLK input jack.
		CLK_Port.reset();
		// Defining thresholds for CLK input jack (rescale).
		//CLK_Port.setThresholds(0.2f, 3.5f);
		// CLK is low (not wired = no signal = false).
		bCLKisHigh = false;
		// Reset ROTATE indexes.
		cvRotateTblIndex = 0;
		cvRotateTblIndexPrevious = 0;
		// Table rotation is on Initialize. For now we're using standard "R+0" base table.
		bDoRotation  = true;
		bDoRotationOnRisingEdge = false;
		//
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
			divCounters[i] = 0; // Reset all dividers counters to 0 (for all output jacks).
			pulseOutputJack(i, false); // Be sure this jack isn't pulsing.
		}
		// Reset "Auto-Reset" counter and related flags.
		divCountersAutoReset = 0;
		bIsAutoReset = false;
		bAllowAutoReset = false;
		bAutoResetLEDfired = false;
		// Unlit CLK LED.
		lights[LED_CLK].value = 0.0f;
		// Source (CLK) frequency is reset (because CLK signal is lost/absent).
		bCLKFreqKnown = false;
		// Reset step-based counters.
		currentStep = 0;
		previousStep = 0;
		expectedStep = 0;
		// Early rising edge flag. When set, this meaning the next rising edge will be considered as early (first) rising edge. Required for gate modes!
		bIsEarlyRisingEdge = true;
		// Set time out flag (this will lit RESET red LED).
		bCLKTimeOut = true;
	}

	// Pulse manager.
	void pulseOutputJack(int givenOutputJack, bool bJackPulseState) {
		outputs[givenOutputJack].value = bJackPulseState ? 5.0f : 0.0f;
		lights[givenOutputJack].value =  bJackPulseState ? 1.0f : 0.0f;
		bJackIsFired[givenOutputJack] = bJackPulseState;
	}

	// Persistence for extra datas via json functions (in particular setting defined via jumpers/switches, and table set).
	// These extra datas are saved into .vcv files (including "autosave.vcv").
	// Also these extra datas are "transfered" as soon as you duplicate (clone) module on the rack.
	json_t *toJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "jmprCountingDown", json_boolean(jmprCountingDown)); // "Counting" jumper/switch.
		json_object_set_new(rootJ, "jmprGate", json_boolean(jmprGate)); // "Trig./Gate" jumper/switch.
		json_object_set_new(rootJ, "jmprMaxDivRange16", json_boolean(jmprMaxDivRange16)); // "Max-Div-Range 16" jumper/switch.
		json_object_set_new(rootJ, "jmprMaxDivRange32", json_boolean(jmprMaxDivRange32)); // "Max-Div-Range 32" jumper/switch.
		json_object_set_new(rootJ, "jmprSpread", json_boolean(jmprSpread)); // "Spread" jumper/switch.
		json_object_set_new(rootJ, "jmprAutoReset", json_boolean(jmprAutoReset)); // "Auto-Reset" jumper/switch.
		json_object_set_new(rootJ, "tableSet", json_integer(tableSet)); // Table set (0: Manufacturer, 1: Prime numbers, 2: Perfect squares, 3: Fibonacci sequence).
		return rootJ;
	}

	// Retrieving "json" persistent settings.
	void fromJson(json_t *rootJ) override	{
		json_t *jmprCountingDownJ = json_object_get(rootJ, "jmprCountingDown");
		if (jmprCountingDownJ)
			jmprCountingDown = json_is_true(jmprCountingDownJ);
		json_t *jmprGateJ = json_object_get(rootJ, "jmprGate");
		if (jmprGateJ)
			jmprGate = json_is_true(jmprGateJ);
		json_t *jmprMaxDivRange16J = json_object_get(rootJ, "jmprMaxDivRange16");
		if (jmprMaxDivRange16J)
			jmprMaxDivRange16 = json_is_true(jmprMaxDivRange16J);
		json_t *jmprMaxDivRange32J = json_object_get(rootJ, "jmprMaxDivRange32");
		if (jmprMaxDivRange32J)
			jmprMaxDivRange32 = json_is_true(jmprMaxDivRange32J);
		json_t *jmprSpreadJ = json_object_get(rootJ, "jmprSpread");
		if (jmprSpreadJ)
			jmprSpread = json_is_true(jmprSpreadJ);
		json_t *jmprAutoResetJ = json_object_get(rootJ, "jmprAutoReset");
		if (jmprAutoResetJ)
			jmprAutoReset = json_is_true(jmprAutoResetJ);
		json_t *tableSetJ = json_object_get(rootJ, "tableSet");
		if (tableSetJ)
			tableSet = json_integer_value(tableSetJ);
	}
}; // End of module (object) definition.

void RKDBRK::step() {
	// DSP processing.
	// Reading jumpers/switches setting.
	jmprGate = (params[JUMPER_GATE].value == 1.0);
	jmprGatePrevious = jmprGate;
	jmprCountingDown = (params[JUMPER_COUNTINGDOWN].value == 1.0);
	// Gate mode only: if "Counting" is changed on the fly, invert firing status for each output jack.
	if ((jmprGate) && (jmprCountingDownPrevious != jmprCountingDown))
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
			bJackIsFired[i] = !bJackIsFired[i];
	jmprCountingDownPrevious = jmprCountingDown;
	jmprMaxDivRange16 = (params[JUMPER_MAXDIVRANGE16].value == 1.0);
	bTableChange = bTableChange || (jmprMaxDivRange16 != jmprMaxDivRange16Previous);
	jmprMaxDivRange16Previous = jmprMaxDivRange16;
	jmprMaxDivRange32 = (params[JUMPER_MAXDIVRANGE32].value == 1.0);
	bTableChange = bTableChange || (jmprMaxDivRange32 != jmprMaxDivRange32Previous);
	jmprMaxDivRange32Previous = jmprMaxDivRange32;
	jmprSpread = (params[JUMPER_SPREAD].value == 1.0);
	if (tableSet == 0)
		bTableChange = bTableChange || (jmprSpread != jmprSpreadPrevious); // Spread concerns manufacturer table only. Have no effect on other tables.
	jmprSpreadPrevious = jmprSpread;
	jmprAutoReset = (params[JUMPER_AUTORESET].value == 1.0);
	// Checking if table set was changed via context-menu.
	if (!bTableChange)
		bTableChange = (tableSetPrev != tableSet);
	// Is table change?
	if (bTableChange) {
		// Yep! assuming table have been changed (either by jumpers/switches setting, or table set via module's context-menu).
		if (tableSet == 0) {
			// Define new "Mav Div" amount, regardling current "Max-Div-Range 16", "Max-Div-Range 32" and "Spread" jumpers/switches setting.
			if (jmprMaxDivRange16 && jmprMaxDivRange32 && jmprSpread)
				maxDivAmount = 16; // Max-Div-Range 16 = On, Max-Div-Range 32 = On: Max Div = 8, but Spread On --> Max Div 16.
				else if (jmprMaxDivRange16 && jmprMaxDivRange32 && !jmprSpread)
					maxDivAmount = 8; // Max-Div-Range 16 = On, Max-Div-Range 32 = On, Spread Off: Max Div = 8 (it's the default factory).
					else if (jmprMaxDivRange16 && !jmprMaxDivRange32)
						maxDivAmount = 16; // Max-Div-Range 16 = On, Max-Div-Range 32 = Off: Max Div = 16.
						else if (!jmprMaxDivRange16 && jmprMaxDivRange32)
							maxDivAmount = 32; // Max-Div-Range 16 = Off, Max-Div-Range 32 = On: Max Div = 32.
							else maxDivAmount = 64; // Last possible remaining case is... Max-Div-Range 16 = Off, Max-Div-Range 32 = Off: Max Div = 64.
		}
		else maxDivAmount = 64; // Max Div = 64 for all "extra" tables.

		switch (tableSet) {
			case 0:
				// Now we're defining "future" table, but based on "Manufacturer" table.
				if (jmprMaxDivRange16 && jmprMaxDivRange32 && jmprSpread) {
					// Max-Div-Range 16 = On, Max-Div-Range 32 = On, Spread = On.
					// Special case of "musical" divisions (triplets, 16ths).
					tblDividersR0[OUTPUT_1] = 1;
					tblDividersR0[OUTPUT_2] = 2;
					tblDividersR0[OUTPUT_3] = 3;
					tblDividersR0[OUTPUT_4] = 4;
					tblDividersR0[OUTPUT_5] = 6;
					tblDividersR0[OUTPUT_6] = 8;
					tblDividersR0[OUTPUT_7] = 12;
					tblDividersR0[OUTPUT_8] = 16;
				}
				else if (jmprMaxDivRange16 && jmprMaxDivRange32 && !jmprSpread) {
					// Max-Div-Range 16 = On, Max-Div-Range 32 = On, Spread = Off (factory setting).
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersR0[i] = i + 1;
				}
				else if (jmprMaxDivRange16 && !jmprMaxDivRange32 && jmprSpread) {
					// Max-Div-Range 16 is On, Max-Div-Range 32 is Off, Spread is On.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersR0[i] = 2 * i + 2;
				}
				else if (jmprMaxDivRange16 && !jmprMaxDivRange32 && !jmprSpread) {
					// Max-Div-Range 16 is On, Max-Div-Range 32 is Off, Spread is Off.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersR0[i] = i + 9;
				}
				else if (!jmprMaxDivRange16 && jmprMaxDivRange32 && jmprSpread) {
					// Max-Div-Range 16 is Off, Max-Div-Range 32 is On, Spread is On.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersR0[i] = 4 * i + 4;
				}
				else if (!jmprMaxDivRange16 && jmprMaxDivRange32 && !jmprSpread) {
					// Max-Div-Range 16 is Off, Max-Div-Range 32 is On, Spread is Off.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersR0[i] = i + 17;
				}
				else if (!jmprMaxDivRange16 && !jmprMaxDivRange32 && jmprSpread) {
					// Max-Div-Range 16 is Off, Max-Div-Range 32 is Off, Spread is On.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersR0[i] = 8 * i + 8;
				}
				else if (!jmprMaxDivRange16 && !jmprMaxDivRange32 && !jmprSpread) {
					// Last possibility: Max-Div-Range 16 is Off, Max-Div-Range 32 is Off, Spread is Off.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersR0[i] = i + 33;
				}
				// Now we're defining "future" table.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					tblDividersRt[i] = tblDividersR0[i];
				break;
			case 1:
				// Now we're defining "future" table, but based on prime numbers.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					tblDividersRt[i] = tblPrimes[i];
				break;
			case 2:
				// Now we're defining "future" table, but based on perfect squares.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					tblDividersRt[i] = tblSquares[i];
				break;
			case 3:
				// Now we're defining "future" table, but based on Fibonacci sequence.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					tblDividersRt[i] = tblFibonacci[i];
				break;
			case 4:
				// Now we're defining "future" table, but based on triplet & 16ths.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					tblDividersRt[i] = tblTripletSixteenths[i];
		}
		// Clearing flag about table change "preparation".
		bTableChange = false;
		// Arming table rotation (required after table change).
		bDoRotation  = true;
		bDoRotationOnRisingEdge  = false;
	}

	tableSetPrev = tableSet;

	// CV ROTATE analysis: is module receive ROTATE voltage?
	if (inputs[ROTATE_INPUT].active) {
		// CV ROTATE voltage must be between 0V to +5V (inclusive) - otherwise, voltage is clipped.
		cvRotate = clamp(inputs[ROTATE_INPUT].value, 0.0f, 5.0f);
	}
	else cvRotate = 0.0f; // Assuming 0V while ROTATE input port isn't wired.

	// "cvRotateTblIndex" is a kind of index to dividers table.
	switch (tableSet) {
		case 0:
			// Manufacturer table is also based on "Max-Div" amount (jumpers J3-J4, or Max Div switches setting on BRK panel).
			cvRotateTblIndex = int(cvRotate / 5.0f * (float)(maxDivAmount));
			if (cvRotateTblIndex >= maxDivAmount)
				cvRotateTblIndex = maxDivAmount - 1; // Possible number of table rotations is based on Max Div amount!
			break;
		case 1:
			// Prime is based on 18 possible values, meaning 11 possible "sliding windows" to get access to 8 (consecutive) prime numbers (one per output jack).
			cvRotateTblIndex = int(cvRotate / 5.0f * 11.0f);
			if (cvRotateTblIndex >= 11)
				cvRotateTblIndex = 10;
			break;
		case 2:
		case 4:
			// "Perfect squares" and "Triplet & 16ths" are based on 8 possible values (one per output jack).
			cvRotateTblIndex = int(cvRotate / 5.0f * 8.0f);
			if (cvRotateTblIndex >= 8)
				cvRotateTblIndex = 7;
			break;
		case 3:
			// Fibonacci sequence is based on 8 possible values, 1 possible rotation (first), then 10 possible translations.
			cvRotateTblIndex = int(cvRotate / 5.0f * 11.0f);
			if (cvRotateTblIndex >= 11)
				cvRotateTblIndex = 10;
			break;
	}

	// If table index have changed (or rotation was previously set), rotation is required.
	bDoRotation = bDoRotation || (cvRotateTblIndexPrevious != cvRotateTblIndex);

	// Is table rotation required?
	if (bDoRotation) {
		// Clear "preparation" flag.
		bDoRotation  = false;
		// Table rotation is required. Set (arm) another/next flag, by this way, real rotation will occur on next CLK rising-edge.
		bDoRotationOnRisingEdge  = true;
		// Depending table set (Manufacturer, Prime numbers, Perfect squares, or Fibonacci sequence).
		switch (tableSet) {
			case 0:
				// Manufacturer table.
				if (jmprMaxDivRange16 && jmprMaxDivRange32 && jmprSpread) {
					// Particular case when Max-Divide-Amount is 8 by jumpers/switches --AND-- Spread is On...
					// In this special case, all ports will output standard "musical" divisions of 16ths & triplets.
					// We're using "cell moves" (like a shorting routine will do).
					// Firstly, duplicating base "R+0" table...
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersRt[i] = tblDividersR0[i];
					// ...then doing "j" moves (from bottom to top).
					for (int j = 0; j < cvRotateTblIndex; j++) {
						// Saving OUTPUT 1.
						tblDividersRt[8] = tblDividersRt[OUTPUT_1];
						tblDividersRt[OUTPUT_1] = tblDividersRt[OUTPUT_2];
						tblDividersRt[OUTPUT_2] = tblDividersRt[OUTPUT_3];
						tblDividersRt[OUTPUT_3] = tblDividersRt[OUTPUT_4];
						tblDividersRt[OUTPUT_4] = tblDividersRt[OUTPUT_5];
						tblDividersRt[OUTPUT_5] = tblDividersRt[OUTPUT_6];
						tblDividersRt[OUTPUT_6] = tblDividersRt[OUTPUT_7];
						tblDividersRt[OUTPUT_7] = tblDividersRt[OUTPUT_8];
						// Previously saved OUTPUT 1 is restored to... OUTPUT 8!
						tblDividersRt[OUTPUT_8] = tblDividersRt[8];

					}
				}
				else {
					// All other cases are standard shifting.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
						// Duplicating "R+0" reference table, then adding number of rotation(s).
						tblDividersRt[i] = tblDividersR0[i] + cvRotateTblIndex;
						if (tblDividersRt[i] > maxDivAmount) {
							// Applying "modulo" if necessary!
							tblDividersRt[i] = (tblDividersRt[i] % maxDivAmount);
						}
					}
				}
				break;
			case 1:
				// Prime numbers-based table.
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					tblDividersRt[i] = tblPrimes[i + cvRotateTblIndex];
				break;
			case 2:
			case 4:
				// "Perfect squares" or "Triplet & 16ths" based table.
				// Firstly, duplicating perfect squares table...
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					if (tableSet == 2)
					tblDividersRt[i] = tblSquares[i];
					else tblDividersRt[i] = tblTripletSixteenths[i];
				// ...then doing "j" moves (from bottom to top).
				for (int j = 0; j < cvRotateTblIndex; j++) {
					// Saving OUTPUT 1.
					tblDividersRt[8] = tblDividersRt[OUTPUT_1];
					tblDividersRt[OUTPUT_1] = tblDividersRt[OUTPUT_2];
					tblDividersRt[OUTPUT_2] = tblDividersRt[OUTPUT_3];
					tblDividersRt[OUTPUT_3] = tblDividersRt[OUTPUT_4];
					tblDividersRt[OUTPUT_4] = tblDividersRt[OUTPUT_5];
					tblDividersRt[OUTPUT_5] = tblDividersRt[OUTPUT_6];
					tblDividersRt[OUTPUT_6] = tblDividersRt[OUTPUT_7];
					tblDividersRt[OUTPUT_7] = tblDividersRt[OUTPUT_8];
					// Previously saved OUTPUT 1 is restored to... OUTPUT 8!
					tblDividersRt[OUTPUT_8] = tblDividersRt[8];
				}
				break;
			case 3:
				// Fibonacci-based table.
				// Firstly, duplicating Fibonacci table.
				if (cvRotateTblIndex < 2) {
					// No rotation use 1, 2, 3, 5, 8, 13, 21, 34.
					// First rotation uses 2, 3, 5, 8, 13, 21, 34, 55.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersRt[i] = tblFibonacci[i + cvRotateTblIndex];
				}
				else {
					// Next rotations are based on 2, 3, 5, 8, 13, 21, 34, 55, then applying +R shift.
					// On second and later rotation, then applying "+R" on all ports.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						tblDividersRt[i] = tblFibonacci[i + 1] + cvRotateTblIndex - 1;
				}
		}
	}

	// By default assuming this step isn't a CLK rising edge.
	bIsRisingEdge = false;
	// By default assuming this step isn't a CLK falling edge.
	bIsFallingEdge = false;

	// Module is running (enabled) as long as its CLK input jack is wired.
	bCLKisActive = inputs[CLK_INPUT].active;
	if (!bCLKisActive) {
		// Module in timeout (idle) mode.
		ModuleTimeOut();
		// CLK isn't connected: display "--" in all segment-LED displays (using -1).
		strcpy(dispDiv1, "--");
		strcpy(dispDiv2, "--");
		strcpy(dispDiv3, "--");
		strcpy(dispDiv4, "--");
		strcpy(dispDiv5, "--");
		strcpy(dispDiv6, "--");
		strcpy(dispDiv7, "--");
		strcpy(dispDiv8, "--");
	}
	else {
		// CLK input port is wired.
		// Update segment-LEDs to display the eight dividers alongside their jacks.
		// Based on "future" table!
		snprintf(dispDiv1, sizeof(dispDiv1), "%2i", tblDividersRt[OUTPUT_1]);
		snprintf(dispDiv2, sizeof(dispDiv2), "%2i", tblDividersRt[OUTPUT_2]);
		snprintf(dispDiv3, sizeof(dispDiv3), "%2i", tblDividersRt[OUTPUT_3]);
		snprintf(dispDiv4, sizeof(dispDiv4), "%2i", tblDividersRt[OUTPUT_4]);
		snprintf(dispDiv5, sizeof(dispDiv5), "%2i", tblDividersRt[OUTPUT_5]);
		snprintf(dispDiv6, sizeof(dispDiv6), "%2i", tblDividersRt[OUTPUT_6]);
		snprintf(dispDiv7, sizeof(dispDiv7), "%2i", tblDividersRt[OUTPUT_7]);
		snprintf(dispDiv8, sizeof(dispDiv8), "%2i", tblDividersRt[OUTPUT_8]);
		// Increment step number.
		currentStep++;
		// Using Schmitt trigger (SchmittTrigger is provided by dsp/digital.hpp) to detect triggers on CLK input jack.
		if (CLK_Port.process(rescale(inputs[CLK_INPUT].value, 0.2f, 3.5f, 0.0f, 1.0f))) {
			// It's a rising edge.
			bIsRisingEdge = true;
			// Disarm timeout flag.
			bCLKTimeOut = false;
			// CLK input is receiving a compliant trigger voltage (trigger on rising edge).
			// If rotation was requested, it becomes effective on received rising edge. Set the new current dividers table.
			if (bDoRotationOnRisingEdge) {
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					tblActiveDividers[i] = tblDividersRt[i];
				// Define adaptative "Max Div" amount, but only for "Primes numbers" table!
				// "Max Div" can be 32 or 64, depending highest divider.
				if (tableSet == 1) {
					if (tblActiveDividers[7] > 32)
						maxDivAmount = 64;
						else maxDivAmount = 32;
				}
				// Rotation was done.
				bDoRotationOnRisingEdge  = false;
			}
			// Frequency of source CLK.
			if (previousStep == 0) {
				// Source CLK frequency is unknown.
				bCLKFreqKnown = false;
				expectedStep = 0;
			}
			else {
				// But perhaps this rising edge comes too early (source frequency is increased).
				if (bCLKFreqKnown) {
					if (currentStep != expectedStep) {						
						bCLKFreqKnown = false;
						expectedStep = 0;
						currentStep = 1;
					}
					else {
						// Source CLK frequency is stable.
						bCLKFreqKnown = true;
						expectedStep = currentStep - previousStep + currentStep;
					}
				}
				else {
					// Source CLK frequency was unknow at previous rising edge, but for now it can be established on this (next) rising edge.
					bCLKFreqKnown = true;
					expectedStep = currentStep - previousStep + currentStep;
				}
			}
			// Of course, on rising edgen the CLK signal is high!
			bCLKisHigh = true;
			// ...and this current step (on rising edge) becomes... previous step, for next rising edge detection!
			previousStep = currentStep;
		}
		else {
			// At this point it's not a rising edge (maybe incoming signal is already at high state, or low, or a falling edge).
			// Is it a falling edge?
			if (bCLKisHigh && (clamp(inputs[CLK_INPUT].value, 0.0f, 15.0f) < 0.2f)) {
				// At previous step it was high, but now is low, meaning this step is a falling edge.
				bCLKisHigh = false; // Below 2V, disarm the flag to stop counting.
				// It's a falling edge.
				bIsFallingEdge = true;
			}
			// Also, be sure the CLK source frequency wasn't lower (slower signal).
			if (expectedStep != 0) {
				if (currentStep >= expectedStep) {
					// CLK frequency is lower (slower), or... no more signal (kind of "timeout").
					if (bCLKFreqKnown) {
						// If the frequency was previously known, we give an extra delay prior timeout.
						expectedStep = currentStep - previousStep + currentStep;  // Give an extra delay prior timeout.
						bCLKFreqKnown = false;
					}
					else ModuleTimeOut(); // Timeout: module becomes "idle".
				}
			}
		}
	}

	// "CLK" white LED state.
	lights[LED_CLK].value = bCLKisHigh ? 1.0f : 0.0f;

	// Using Schmitt trigger (SchmittTrigger is provided by dsp/digital.hpp) to register incoming trigger signal on RESET jack (anytime).
	if (!bRegisteredResetOnJack)
		bRegisteredResetOnJack = RESET_Port.process(rescale(inputs[RESET_INPUT].value, 0.2f, 3.5f, 0.0f, 1.0f));

	// Registered RESET on jack becomes effective on next incoming rising edge.
	if (bIsRisingEdge) {
		// Clearing "Auto-Reset" flag.
		bIsAutoReset = false;
		// Is RESET jack was triggered?
		bResetOnJack = bRegisteredResetOnJack;
		bRegisteredResetOnJack = false;
		// Global dividers counters reset for all output jacks, due to received pulse on "RESET" jack.
		if (bResetOnJack) {
			// Reset Schmitt trigger used by RESET input jack.
			RESET_Port.reset();
			// This will "force" disabling RESET LED afterglow, in order to lit orange LED.
			ledResetAfterglow = 0;
			// Reset dividers counters.
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
				divCounters[i] = 0;
			// Temporary inhibit Auto-Reset.
			bAllowAutoReset = false;
			// Restart Auto-Reset sequence (counter).
			divCountersAutoReset = 0;
		}		
	}
	else bResetOnJack = false;

	// Determine initial pulsing state, only on early rising edge!
	if (bIsEarlyRisingEdge)
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
			bJackIsFired[i] = !jmprCountingDown;

	// Auto-reset and pulsing management (for each output jack).
	for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
		if (bIsRisingEdge) {
			// On rising edge.
			// Is the "Auto-Reset", for this current jack, must be applied first, or not?
			// "Auto-Reset" may occurs only if "Auto-Reset" jumper/switch is On, not temporary disabled, and Auto-Reset counter is 0, and for certain dividers.
			if ((jmprAutoReset) && (bAllowAutoReset) && (divCountersAutoReset == 0) && (((2 * maxDivAmount) % tblActiveDividers[i]) != 0)) {
				divCounters[i] = 0;
				bIsAutoReset = true;
				bAutoResetLEDfired = true;
			}
			// Pulse generators.
			if (jmprGate) {
				// Gate modes.
				if ((tblActiveDividers[i] % 2) == 0) {
					// On all even dividers...
					if (divCounters[i] % (tblActiveDividers[i] / 2) == 0)
						pulseOutputJack(i, !bJackIsFired[i]); // Invert state of pulse.
				}
				else {
					// On all odd dividers...
					// /1 in (gate modes) must be considered differently, in fact like... trigger mode! (TBC).
					if (tblActiveDividers[i] == 1)
						pulseOutputJack(i, true); // Degraded" pulse while source frequency isn't stable.
					else if ((divCounters[i] % tblActiveDividers[i]) == 0)
						pulseOutputJack(i, !bJackIsFired[i]); // Invert state of pulse.
				}
			}
			else {
				// Trigger modes (default).
				if (jmprCountingDown)
					pulseOutputJack(i, (divCounters[i] % tblActiveDividers[i]) == 0); // Counting down mode.
					else pulseOutputJack(i, ((divCounters[i] + 1) % tblActiveDividers[i]) == 0); // Counting up mode (default).
			}
			// Advances next divider counter for this jack.
			// Increment divider counter...
			divCounters[i]++;
			// ...and restart to 0 when division value (for current jack) is reached (or over).
			if (divCounters[i] >= tblActiveDividers[i])
				divCounters[i] = 0;
		}
		else if (bIsFallingEdge) {
			// On falling edge (only).
			if (jmprGate) {
				// Gate mode.
				if ((tblActiveDividers[i] % 2) == 1) {
					if (((divCounters[i] + ((tblActiveDividers[i] - 1) / 2)) % tblActiveDividers[i]) == 0)
						pulseOutputJack(i, !bJackIsFired[i]); // Invert state of pulse.
				}
			}
			else pulseOutputJack(i, false); // Trigger mode: always stop pulsing on falling edge (for any divider).
		}
	}

	// Advance "Auto-Reset" counter (global, on rising edges only).
	if (bIsRisingEdge) {
		// Increment "Auto-Reset" sequence counter...
		divCountersAutoReset++;
		// ...and restart to 0 when "2 x Max-Div" is reached.
		if ((divCountersAutoReset % (2 * maxDivAmount)) == 0)
			divCountersAutoReset = 0;
		// Now next rising edge aren't first rising edge.
		bIsEarlyRisingEdge = false;
		// Allow next Auto-Reset events.
		bAllowAutoReset = true;
	}
	else bResetOnJack = false;

	// "RESET" small red LED management (having afterglow).
	if (ledResetAfterglow > 0) {
		ledResetAfterglow--;
	}
	else {
		if ((bIsAutoReset && bAutoResetLEDfired) || bResetOnJack || bCLKTimeOut) {
			if (bCLKTimeOut) {
				// Highest priority LED.
				// Setup counter for red LED afterglow.
				ledResetAfterglow = round(engineGetSampleRate() / 4);
				// Lit "RESET" LED (red) on CLK timeout.
				lights[LED_RESET_RED].value = 1.0f;
				lights[LED_RESET_ORANGE].value = 0.0f;
				lights[LED_RESET_BLUE].value = 0.0f;
			}
			else if (bResetOnJack) {
				// Setup counter for orange LED afterglow.
				ledResetAfterglow = round(engineGetSampleRate() / 6);
				// Lit "RESET" LED (orange) on RESET via jack.
				lights[LED_RESET_RED].value = 0.0f;
				lights[LED_RESET_ORANGE].value = 1.0f;
				lights[LED_RESET_BLUE].value = 0.0f;
			}
			else {
				// Setup counter for blue LED afterglow.
				ledResetAfterglow = round(engineGetSampleRate() / 8);
				// Lit "RESET" LED (blue) on "Auto-Reset" event.
				lights[LED_RESET_RED].value = 0.0f;
				lights[LED_RESET_ORANGE].value = 0.0f;
				lights[LED_RESET_BLUE].value = 1.0f;
				bAutoResetLEDfired = false;
			}
		}
		else {
			// Unlit "RESET" LEDs.
			lights[LED_RESET_RED].value = 0.0f;
			lights[LED_RESET_ORANGE].value = 0.0f;
			lights[LED_RESET_BLUE].value = 0.0f;
		}
	}

	// Update current rotation index to become "previous". This will be useful to detect possible "table rotation" on next step.
	cvRotateTblIndexPrevious = cvRotateTblIndex;
} // End of DSP processing...

// Segment-LED displays.
struct RKDBRK_Displays : TransparentWidget {
	RKDBRK *module;
	std::shared_ptr<Font> font;
	RKDBRK_Displays() {
		font = Font::load(assetPlugin(plugin, "res/fonts/digital-readout.medium.ttf"));
	}

	void updateD1(NVGcontext *vg, Vec pos, char* dispDiv1) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 29.5, dispDiv1, NULL);
	}

	void updateD2(NVGcontext *vg, Vec pos, char* dispDiv2) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 59.5, dispDiv2, NULL);
	}

	void updateD3(NVGcontext *vg, Vec pos, char* dispDiv3) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 89.5, dispDiv3, NULL);
	}

	void updateD4(NVGcontext *vg, Vec pos, char* dispDiv4) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 119.5, dispDiv4, NULL);
	}

	void updateD5(NVGcontext *vg, Vec pos, char* dispDiv5) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 149.5, dispDiv5, NULL);
	}

	void updateD6(NVGcontext *vg, Vec pos, char* dispDiv6) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 179.5, dispDiv6, NULL);
	}

	void updateD7(NVGcontext *vg, Vec pos, char* dispDiv7) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 209.5, dispDiv7, NULL);
	}

	void updateD8(NVGcontext *vg, Vec pos, char* dispDiv8) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgTransRGBA(nvgRGB(0x6c, 0xff, 0xff), 0xff));
		nvgText(vg, pos.x + 16, pos.y + 239.5, dispDiv8, NULL);
	}

	void draw(NVGcontext *vg) override {
		updateD1(vg, Vec(0, box.size.y - 150), module->dispDiv1);
		updateD2(vg, Vec(0, box.size.y - 150), module->dispDiv2);
		updateD3(vg, Vec(0, box.size.y - 150), module->dispDiv3);
		updateD4(vg, Vec(0, box.size.y - 150), module->dispDiv4);
		updateD5(vg, Vec(0, box.size.y - 150), module->dispDiv5);
		updateD6(vg, Vec(0, box.size.y - 150), module->dispDiv6);
		updateD7(vg, Vec(0, box.size.y - 150), module->dispDiv7);
		updateD8(vg, Vec(0, box.size.y - 150), module->dispDiv8);
	}
};

struct RKDBRKWidget : ModuleWidget {
	RKDBRKWidget(RKDBRK *module);
	// Action for "Randomize", from context-menu, is (for now) bypassed.
	void randomize() override {
	};
	// RKDBRK module uses a custom context-menu, to access to different table sets.
	Menu* createContextMenu() override;
};

RKDBRKWidget::RKDBRKWidget(RKDBRK *module) : ModuleWidget(module) {
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/RKDBRK.svg")));
		panel->box.size = box.size;
		addChild(panel);
	}
	// Using four screws configuration for 8 HP module.
	// Top-left screw.
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0)));
	// Top-right screw.
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 3 * RACK_GRID_WIDTH, 0)));
	// Bottom-left screw.
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	// Bottom-right screw.
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	// Segment-LED displays.
	{
		RKDBRK_Displays *display = new RKDBRK_Displays();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 234);
		addChild(display);
	}
	// Input jack: ROTATE.
	addInput(Port::create<CL1362_In_RR>(Vec(2.4, 35), Port::INPUT, module, RKDBRK::ROTATE_INPUT));
	// Input jack: RESET.
	addInput(Port::create<CL1362_In_RR>(Vec(32.2, 35), Port::INPUT, module, RKDBRK::RESET_INPUT));
	// Input jack: CLK.
	addInput(Port::create<CL1362_In>(Vec(30.8, 66), Port::INPUT, module, RKDBRK::CLK_INPUT));
	// Output jack: 1+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 96), Port::OUTPUT, module, RKDBRK::OUTPUT_1));
	// Output jack: 2+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 126), Port::OUTPUT, module, RKDBRK::OUTPUT_2));
	// Output jack: 3+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 156), Port::OUTPUT, module, RKDBRK::OUTPUT_3));
	// Output jack: 4+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 186), Port::OUTPUT, module, RKDBRK::OUTPUT_4));
	// Output jack: 5+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 216), Port::OUTPUT, module, RKDBRK::OUTPUT_5));
	// Output jack: 6+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 246), Port::OUTPUT, module, RKDBRK::OUTPUT_6));
	// Output jack: 7+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 276), Port::OUTPUT, module, RKDBRK::OUTPUT_7));
	// Output jack: 8+R.
	addOutput(Port::create<CL1362_Out>(Vec(30.8, 306), Port::OUTPUT, module, RKDBRK::OUTPUT_8));
	// White LED for CLK.
	addChild(ModuleLightWidget::create<MediumLight<RKDWhiteLight>>(Vec(3.7, 73.4), module, RKDBRK::LED_CLK));
	// Red LED for output 1+R.
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(3.7, 103.4), module, RKDBRK::LED_OUT_1));
	// Orange LED for output 2+R.
	addChild(ModuleLightWidget::create<MediumLight<RKDOrangeLight>>(Vec(3.7, 133.4), module, RKDBRK::LED_OUT_2));
	// Yellow LED for output 3+R.
	addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(Vec(3.7, 163.4), module, RKDBRK::LED_OUT_3));
	// Green LED for output 4+R.
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(3.7, 193.4), module, RKDBRK::LED_OUT_4));
	// Green LED for output 5+R.
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(3.7, 223.4), module, RKDBRK::LED_OUT_5));
	// Blue (cyan) LED for output 6+R.
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(3.7, 253.4), module, RKDBRK::LED_OUT_6));
	// Purple LED for output 7+R.
	addChild(ModuleLightWidget::create<MediumLight<RKDPurpleLight>>(Vec(3.7, 283.4), module, RKDBRK::LED_OUT_7));
	// White LED for output 8+R.
	addChild(ModuleLightWidget::create<MediumLight<RKDWhiteLight>>(Vec(3.7, 313.4), module, RKDBRK::LED_OUT_8));
	// Small red LED near RESET input jack (tri-colored: red, orange or blue).
	addChild(ModuleLightWidget::create<SmallLight<RedOrangeBlueLight>>(Vec(50.8, 33.2), module, RKDBRK::LED_RESET_RED));
	// Switch "Counting Up/Down". By default Off (counting up).
	addParam(ParamWidget::create<RKDBRK_Switch>(Vec(70.3, 64.2), module, RKDBRK::JUMPER_COUNTINGDOWN, 0.0, 1.0, 0.0));
	// Switch "Trig/Gate". By default Off (trigger).
	addParam(ParamWidget::create<RKDBRK_Switch>(Vec(70.3, 112.2), module, RKDBRK::JUMPER_GATE, 0.0, 1.0, 0.0));
	// Switch "Max-Div-Range 16". By default On. Working together with "Max-Div-Range 32".
	addParam(ParamWidget::create<RKDBRK_Switch>(Vec(70.3, 160.2), module, RKDBRK::JUMPER_MAXDIVRANGE16, 0.0, 1.0, 1.0));
	// Switch "Max-Div-Range 32". By default On. Working together with "Max-Div-Range 16".
	addParam(ParamWidget::create<RKDBRK_Switch>(Vec(70.3, 206.2), module, RKDBRK::JUMPER_MAXDIVRANGE32, 0.0, 1.0, 1.0));
	// Switch "Spread Off/On". By default Off (spread off).
	addParam(ParamWidget::create<RKDBRK_Switch>(Vec(70.3, 256.2), module, RKDBRK::JUMPER_SPREAD, 0.0, 1.0, 0.0));
	// Switch "Auto-Reset Off/On". By default Off (auto-reset is disabled).
	addParam(ParamWidget::create<RKDBRK_Switch>(Vec(70.3, 304.2), module, RKDBRK::JUMPER_AUTORESET, 0.0, 1.0, 0.0));
}

// Context-menu entry routine: select "RKD (factory)" table.
struct RKDBRKManufacturerItem : MenuItem {
	RKDBRK *rkdbrk;
	void onAction(EventAction &e) override {
		rkdbrk->tableSet = 0;
	}
	void step() override {
		rightText = (rkdbrk->tableSet == 0) ? "✔" : "";
		MenuItem::step();
	}
};

// Context-menu entry routine: select "Prime numbers" table.
struct RKDBRKPrimesItem : MenuItem {
	RKDBRK *rkdbrk;
	void onAction(EventAction &e) override {
		rkdbrk->tableSet = 1;
	}
	void step() override {
		rightText = (rkdbrk->tableSet == 1) ? "✔" : "";
		MenuItem::step();
	}
};

// Context-menu entry routine: select "Perfect squares" table.
struct RKDBRKSquaresItem : MenuItem {
	RKDBRK *rkdbrk;
	void onAction(EventAction &e) override {
		rkdbrk->tableSet = 2;
	}
	void step() override {
		rightText = (rkdbrk->tableSet == 2) ? "✔" : "";
		MenuItem::step();
	}
};

// Context-menu entry routine: select "Fibonacci sequence" table.
struct RKDBRKFibonacciItem : MenuItem {
	RKDBRK *rkdbrk;
	void onAction(EventAction &e) override {
		rkdbrk->tableSet = 3;
	}
	void step() override {
		rightText = (rkdbrk->tableSet == 3) ? "✔" : "";
		MenuItem::step();
	}
};

// Context-menu entry routine: select "Triplet & 16ths" table.
struct RKDBRKTripletSixteenthsItem : MenuItem {
	RKDBRK *rkdbrk;
	void onAction(EventAction &e) override {
		rkdbrk->tableSet = 4;
	}
	void step() override {
		rightText = (rkdbrk->tableSet == 4) ? "✔" : "";
		MenuItem::step();
	}
};

// CONTEXT-MENU CONSTRUCTION.

Menu* RKDBRKWidget::createContextMenu() {
	Menu* menu = ModuleWidget::createContextMenu();
	RKDBRK *rkdbrk = dynamic_cast<RKDBRK*>(module);
	assert(rkdbrk);
	menu->addChild(construct<MenuEntry>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Dividers table:"));
	menu->addChild(construct<RKDBRKManufacturerItem>(&RKDBRKManufacturerItem::text, "Manufacturer", &RKDBRKManufacturerItem::rkdbrk, rkdbrk));
	menu->addChild(construct<RKDBRKPrimesItem>(&RKDBRKPrimesItem::text, "Prime numbers", &RKDBRKPrimesItem::rkdbrk, rkdbrk));
	menu->addChild(construct<RKDBRKSquaresItem>(&RKDBRKSquaresItem::text, "Perfect squares", &RKDBRKSquaresItem::rkdbrk, rkdbrk));
	menu->addChild(construct<RKDBRKFibonacciItem>(&RKDBRKFibonacciItem::text, "Fibonacci sequence", &RKDBRKFibonacciItem::rkdbrk, rkdbrk));
	menu->addChild(construct<RKDBRKTripletSixteenthsItem>(&RKDBRKTripletSixteenthsItem::text, "Triplet & 16ths", &RKDBRKTripletSixteenthsItem::rkdbrk, rkdbrk));
	return menu;
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, RKDBRK) {
   Model *modelRKDBRK = Model::create<RKDBRK, RKDBRKWidget>("Ohmer Modules", "RKDBRK", "RKD with \"Break\"", CLOCK_MODULATOR_TAG);
   return modelRKDBRK;
}
