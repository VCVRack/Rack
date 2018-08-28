//***********************************************************************************************
//Chain-able clock module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and Audible Instruments plugins by Andrew Belt and graphics  
//  from the Component Library by Wes Milholen. 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//Module concept and design by Marc Boulé, Nigel Sixsmith and Xavier Belmont
//
//***********************************************************************************************


#include "ImpromptuModular.hpp"

namespace rack_plugin_ImpromptuModular {

class Clock {
	// The -1.0 step is used as a reset state every double-period so that 
	//   lengths can be re-computed; it will stay at -1.0 when a clock is inactive.
	// a clock frame is defined as "length * iterations + syncWait", and
	//   for master, syncWait does not apply and iterations = 1

	
	double step;// -1.0 when stopped, [0 to 2*period[ for clock steps (*2 is because of swing, so we do groups of 2 periods)
	double length;// double period
	double sampleTime;
	int iterations;// run this many double periods before going into sync if sub-clock
	Clock* syncSrc = nullptr; // only subclocks will have this set to master clock
	static constexpr double guard = 0.0005;// in seconds, region for sync to occur right before end of length of last iteration; sub clocks must be low during this period
	
	public:
	
	int isHigh(float swing, float pulseWidth) {
		// last 0.5ms (guard time) must be low so that sync mechanism will work properly (i.e. no missed pulses)
		//   this will automatically be the case, since code below disallows any pulses or inter-pulse times less than 1ms
		int high = 0;
		if (step >= 0.0) {
			float swParam = swing;// swing is [-1 : 1]
			
			// all following values are in seconds
			float onems = 0.001f;
			float period = (float)length / 2.0f;
			float swing = (period - 2.0f * onems) * swParam;
			float p2min = onems;
			float p2max = period - onems - fabs(swing);
			if (p2max < p2min) {
				p2max = p2min;
			}
			
			//double p1 = 0.0;// implicit, no need 
			double p2 = (double)((p2max - p2min) * pulseWidth + p2min);// pulseWidth is [0 : 1]
			double p3 = (double)(period + swing);
			double p4 = ((double)(period + swing)) + p2;
			
			if (step < p2)
				high = 1;
			else if ((step >= p3) && (step < p4))
				high = 2;
		}
		return high;
	}
	
	void setSync(Clock* clkGiven) {
		syncSrc = clkGiven;
	}
	
	inline void reset() {
		step = -1.0;
	}
	inline bool isReset() {
		return step == -1.0;
	}
	inline double getStep() {
		return step;
	}
	
	inline void setup(double lengthGiven, int iterationsGiven, double sampleTimeGiven) {
		length = lengthGiven;
		iterations = iterationsGiven;
		sampleTime = sampleTimeGiven;
	}
	
	inline void start() {
		step = 0.0;
	}
	
	void stepClock() {// here the clock was output on step "step", this function is called at end of module::step()
		if (step >= 0.0) {// if active clock
			step += sampleTime;
			if ( (syncSrc != nullptr) && (iterations == 1) && (step > (length - guard)) ) {// if in sync region
				if (syncSrc->isReset()) {
					reset();
				}// else nothing needs to be done, just wait and step stays the same
			}
			else {
				if (step >= length) {// reached end iteration
					iterations--;
					step -= length;
					if (iterations <= 0) 
						reset();// frame done
				}
			}
		}
	}
	
	void applyNewLength(double lengthStretchFactor) {
		if (step != -1.0)
			step *= lengthStretchFactor;
		length *= lengthStretchFactor;
	}
};


//*****************************************************************************


class ClockDelay {
	long stepCounter;
	int lastWriteValue;
	bool readState;
	long stepRise1;
	long stepFall1;
	long stepRise2;
	long stepFall2;
	
	public:
	
	void reset() {
		stepCounter = 0l;
		lastWriteValue = 0;
		readState = false;
		stepRise1 = 0l;
		stepFall1 = 0l;
		stepRise2 = 0l;
		stepFall2 = 0l;
	}
	
	void write(int value) {
		if (value == 1) {// first pulse is high
			if (lastWriteValue == 0) // if got rise 1
				stepRise1 = stepCounter;
		}
		else if (value == 2) {// second pulse is high
			if (lastWriteValue == 0) // if got rise 2
				stepRise2 = stepCounter;
		}
		else {// value = 0 (pulse is low)
			if (lastWriteValue == 1) // if got fall 1
				stepFall1 = stepCounter;
			else if (lastWriteValue == 2) // if got fall 2
				stepFall2 = stepCounter;
		}
		
		lastWriteValue = value;
	}
	
	bool read(long delaySamples) {
		long delayedStepCounter = stepCounter - delaySamples;
		if (delayedStepCounter == stepRise1 || delayedStepCounter == stepRise2)
			readState = true;
		else if (delayedStepCounter == stepFall1 || delayedStepCounter == stepFall2)
			readState = false;
		stepCounter++;
		if (stepCounter > 1e8) {// keep within long's bounds (could go higher or could allow negative)
			stepCounter -= 1e8;// 192000 samp/s * 2s * 64 * (3/4) = 18.4 Msamp
			stepRise1 -= 1e8;
			stepFall1 -= 1e8;
			stepRise2 -= 1e8;
			stepFall2 -= 1e8;
		}
		return readState;
	}
};


//*****************************************************************************


struct Clocked : Module {
	enum ParamIds {
		ENUMS(RATIO_PARAMS, 4),// master is index 0
		ENUMS(SWING_PARAMS, 4),// master is index 0
		ENUMS(PW_PARAMS, 4),// master is index 0
		RESET_PARAM,
		RUN_PARAM,
		ENUMS(DELAY_PARAMS, 4),// index 0 is unused
		// -- 0.6.9 ^^
		BPMMODE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(PW_INPUTS, 4),// master is index 0
		RESET_INPUT,
		RUN_INPUT,
		BPM_INPUT,
		ENUMS(SWING_INPUTS, 4),// master is index 0
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CLK_OUTPUTS, 4),// master is index 0
		RESET_OUTPUT,
		RUN_OUTPUT,
		BPM_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RESET_LIGHT,
		RUN_LIGHT,
		ENUMS(CLK_LIGHTS, 4),// master is index 0 (not used)
		ENUMS(BPMSYNC_LIGHT, 2),// room for GreenRed
		NUM_LIGHTS
	};
	
	
	// Constants
	const float delayValues[8] = {0.0f,  0.0625f, 0.125f, 0.25f, 1.0f/3.0f, 0.5f , 2.0f/3.0f, 0.75f};
	const float ratioValues[34] = {1, 1.5, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 23, 24, 29, 31, 32, 37, 41, 43, 47, 48, 53, 59, 61, 64};
	static const int bpmMax = 300;
	static const int bpmMin = 30;
	static constexpr float masterLengthMax = 120.0f / bpmMin;// a length is a double period
	static constexpr float masterLengthMin = 120.0f / bpmMax;// a length is a double period
	static constexpr float delayInfoTime = 3.0f;// seconds
	static constexpr float swingInfoTime = 2.0f;// seconds
	
	// Need to save, with reset
	bool running;
	
	// Need to save, no reset
	int panelTheme;
	int expansion;
	bool displayDelayNoteMode;
	bool bpmDetectionMode;
	bool emitResetOnStopRun;
	int ppqn;
	
	// No need to save, with reset
	// none
	
	// No need to save, no reset
	bool scheduledReset;
	float swingVal[4];
	long swingInfo[4];// downward step counter when swing to be displayed, 0 when normal display
	int delayKnobIndexes[4];
	long delayInfo[4];// downward step counter when delay to be displayed, 0 when normal display
	int ratiosDoubled[4];
	int newRatiosDoubled[4];
	Clock clk[4];
	ClockDelay delay[4];
	float masterLength;// a length is a double period
	float resetLight;
	SchmittTrigger resetTrigger;
	SchmittTrigger runTrigger;
	PulseGenerator resetPulse;
	PulseGenerator runPulse;
	SchmittTrigger bpmDetectTrigger;
	int extPulseNumber;// 0 to ppqn - 1
	double extIntervalTime;
	double timeoutTime;
	long cantRunWarning;// 0 when no warning, positive downward step counter timer when warning
	long editingBpmMode;// 0 when no edit bpmMode, downward step counter timer when edit, negative upward when show can't edit ("--") 
	int lightRefreshCounter;
	
	SchmittTrigger bpmModeTrigger;

	
	// called from the main thread (step() can not be called until all modules created)
	Clocked() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		// Need to save, no reset
		panelTheme = 0;
		expansion = 0;
		displayDelayNoteMode = true;
		bpmDetectionMode = false;
		emitResetOnStopRun = false;
		ppqn = 4;
		// No need to save, no reset
		scheduledReset = false;
		for (int i = 0; i < 4; i++) {
			clk[i].setSync(i == 0 ? nullptr : &clk[0]);
			swingVal[i] = 0.0f;
			swingInfo[i] = 0l;
			delayKnobIndexes[i] = 0;
			delayInfo[i] = 0l;
			ratiosDoubled[i] = 0;
			newRatiosDoubled[i] = 0;
			clk[i].reset();
			delay[i].reset();
		}		
		masterLength = 1.0f;// 120 BPM
		resetLight = 0.0f;
		resetTrigger.reset();
		runTrigger.reset();
		resetPulse.reset();
		runPulse.reset();
		bpmDetectTrigger.reset();
		extPulseNumber = -1;
		extIntervalTime = 0.0;
		timeoutTime = 2.0 / ppqn + 0.1;
		cantRunWarning = 0ul;
		bpmModeTrigger.reset();
		lightRefreshCounter = 0;
		
		onReset();
	}
	

	// widgets are not yet created when module is created 
	// even if widgets not created yet, can use params[] and should handle 0.0f value since step may call 
	//   this before widget creation anyways
	// called from the main thread if by constructor, called by engine thread if right-click initialization
	//   when called by constructor, module is created before the first step() is called
	void onReset() override {
		// Need to save, with reset
		running = false;
		// No need to save, with reset
		// none
		
		scheduledReset = true;
	}
	
	
	// widgets randomized before onRandomize() is called
	// called by engine thread if right-click randomize
	void onRandomize() override {
		// Need to save, with reset
		running = false;
		// No need to save, with reset
		// none
		
		scheduledReset = true;
	}

	
	// called by main thread
	json_t *toJson() override {
		json_t *rootJ = json_object();
		// Need to save (reset or not)
		
		// running
		json_object_set_new(rootJ, "running", json_boolean(running));
		
		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// expansion
		json_object_set_new(rootJ, "expansion", json_integer(expansion));

		// displayDelayNoteMode
		json_object_set_new(rootJ, "displayDelayNoteMode", json_boolean(displayDelayNoteMode));
		
		// bpmDetectionMode
		json_object_set_new(rootJ, "bpmDetectionMode", json_boolean(bpmDetectionMode));
		
		// emitResetOnStopRun
		json_object_set_new(rootJ, "emitResetOnStopRun", json_boolean(emitResetOnStopRun));
		
		// ppqn
		json_object_set_new(rootJ, "ppqn", json_integer(ppqn));
		
		return rootJ;
	}


	// widgets have their fromJson() called before this fromJson() is called
	// called by main thread
	void fromJson(json_t *rootJ) override {
		// Need to save (reset or not)
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// expansion
		json_t *expansionJ = json_object_get(rootJ, "expansion");
		if (expansionJ)
			expansion = json_integer_value(expansionJ);

		// displayDelayNoteMode
		json_t *displayDelayNoteModeJ = json_object_get(rootJ, "displayDelayNoteMode");
		if (displayDelayNoteModeJ)
			displayDelayNoteMode = json_is_true(displayDelayNoteModeJ);

		// bpmDetectionMode
		json_t *bpmDetectionModeJ = json_object_get(rootJ, "bpmDetectionMode");
		if (bpmDetectionModeJ)
			bpmDetectionMode = json_is_true(bpmDetectionModeJ);

		// emitResetOnStopRun
		json_t *emitResetOnStopRunJ = json_object_get(rootJ, "emitResetOnStopRun");
		if (emitResetOnStopRunJ)
			emitResetOnStopRun = json_is_true(emitResetOnStopRunJ);

		// ppqn
		json_t *ppqnJ = json_object_get(rootJ, "ppqn");
		if (ppqnJ)
			ppqn = clamp(json_integer_value(ppqnJ), 4, 24);

		// No need to save, with reset
		// none
		
		scheduledReset = true;
	}

	
	int getRatioDoubled(int ratioKnobIndex) {
		// ratioKnobIndex is 0 for master BPM's ratio (1 is implicitly returned), and 1 to 3 for other ratio knobs
		// returns a positive ratio for mult, negative ratio for div (0 never returned)
		int ret = 1;
		if (ratioKnobIndex > 0) {
			bool isDivision = false;
			int i = (int) round( params[RATIO_PARAMS + ratioKnobIndex].value );// [ -(numRatios-1) ; (numRatios-1) ]
			if (i < 0) {
				i *= -1;
				isDivision = true;
			}
			if (i >= 34) {
				i = 34 - 1;
			}
			ret = (int) (ratioValues[i] * 2.0f + 0.5f);
			if (isDivision) 
				ret = -1l * ret;
		}
		return ret;
	}
	
	
	void resetClocked() {
		for (int i = 0; i < 4; i++) {
			clk[i].reset();
			delay[i].reset();
		}
		extPulseNumber = -1;
		extIntervalTime = 0.0;
		timeoutTime = 2.0 / ppqn + 0.1;
	}		
	
	// called by engine thread
	void onSampleRateChange() override {
		resetClocked();
	}		
	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	// called by engine thread
	void step() override {		
		double sampleRate = (double)engineGetSampleRate();
		double sampleTime = 1.0 / sampleRate;// do this here since engineGetSampleRate() returns float

		// Scheduled reset (just the parts that do not have a place below in rest of function)
		if (scheduledReset) {
			resetClocked();		
			resetLight = 0.0f;
			resetTrigger.reset();
			runTrigger.reset();
			resetPulse.reset();
			runPulse.reset();
			bpmDetectTrigger.reset();
			cantRunWarning = 0l;
			editingBpmMode = 0l;
		}
		
		// Run button
		if (runTrigger.process(params[RUN_PARAM].value + inputs[RUN_INPUT].value)) {
			if (!(bpmDetectionMode && inputs[BPM_INPUT].active) || running) {// toggle when not BPM detect, turn off only when BPM detect (allows turn off faster than timeout if don't want any trailing beats after stoppage). If allow manually start in bpmDetectionMode   the clock will not know which pulse is the 1st of a ppqn set, so only allow stop
				running = !running;
				runPulse.trigger(0.001f);
				resetClocked();// reset on any change of run state (will not re-launch if not running, thus clock railed low)
				if (!running && emitResetOnStopRun) {
					//resetLight = 1.0f;
					resetPulse.trigger(0.001f);
				}
			}
			else
				cantRunWarning = (long) (0.7 * sampleRate / displayRefreshStepSkips);
		}

		// Reset (has to be near top because it sets steps to 0, and 0 not a real step (clock section will move to 1 before reaching outputs)
		if (resetTrigger.process(inputs[RESET_INPUT].value + params[RESET_PARAM].value)) {
			resetLight = 1.0f;
			resetPulse.trigger(0.001f);
			resetClocked();	
		}	

		// BPM mode
		if (bpmModeTrigger.process(params[BPMMODE_PARAM].value)) {
			if (inputs[BPM_INPUT].active) {
				if (editingBpmMode != 0ul) {// force active before allow change
					if (bpmDetectionMode == false) {
						bpmDetectionMode = true;
						ppqn = 4;
					}
					else {
						if (ppqn == 4)
							ppqn = 8;
						else if (ppqn == 8)
							ppqn = 12;
						else if (ppqn == 12)
							ppqn = 24;
						else 
							bpmDetectionMode = false;
					}
				}
				editingBpmMode = (long) (3.0 * sampleRate / displayRefreshStepSkips);
			}
			else
				editingBpmMode = (long) (-1.5 * sampleRate / displayRefreshStepSkips);
		}
		
		// BPM input and knob
		float newMasterLength = masterLength;
		if (inputs[BPM_INPUT].active) { 
			float bpmInValue = inputs[BPM_INPUT].value;
			bool trigBpmInValue = bpmDetectTrigger.process(bpmInValue);
			
			// BPM Detection method
			if (bpmDetectionMode) {
				if (scheduledReset)
					newMasterLength = 1.0f;// 120 BPM
				// rising edge detect
				if (trigBpmInValue) {
					if (!running) {
						// this must be the only way to start runnning when in bpmDetectionMode or else
						//   when manually starting, the clock will not know which pulse is the 1st of a ppqn set
						//runPulse.trigger(0.001f); don't need this since slaves will detect the same thing
						running = true;
						runPulse.trigger(0.001f);
						resetClocked();
					}
					if (running) {
						extPulseNumber++;
						if (extPulseNumber >= ppqn * 2)// *2 because working with double_periods
							extPulseNumber = 0;
						if (extPulseNumber == 0)// if first pulse, start interval timer
							extIntervalTime = 0.0;
						else {
							// all other ppqn pulses except the first one. now we have an interval upon which to plan a strecth 
							double timeLeft = extIntervalTime * (double)(ppqn * 2 - extPulseNumber) / ((double)extPulseNumber);
							newMasterLength = clk[0].getStep() + timeLeft;
							timeoutTime = extIntervalTime * ((double)(1 + extPulseNumber) / ((double)extPulseNumber)) + 0.1;
						}
					}
				}
				if (running) {
					extIntervalTime += sampleTime;
					if (extIntervalTime > timeoutTime) {
						running = false;
						runPulse.trigger(0.001f);
						resetClocked();
						if (emitResetOnStopRun) {
							//resetLight = 1.0f;
							resetPulse.trigger(0.001f);
						}
					}
				}
			}
			// BPM CV method
			else {
				newMasterLength = 1.0f / powf(2.0f, bpmInValue);// bpm = 120*2^V, 2T = 120/bpm = 120/(120*2^V) = 1/2^V
				// no need to round since this clocked's master's BPM knob is a snap knob thus already rounded, and with passthru approach, no cumul error
			}
		}
		else {
			newMasterLength = 120.0f / params[RATIO_PARAMS + 0].value;// already integer BPM since using snap knob
		}
		newMasterLength = clamp(newMasterLength, masterLengthMin, masterLengthMax);
		if (scheduledReset)
			masterLength = newMasterLength;
		if (newMasterLength != masterLength) {
			double lengthStretchFactor = ((double)newMasterLength) / ((double)masterLength);
			for (int i = 0; i < 4; i++) {
				clk[i].applyNewLength(lengthStretchFactor);
			}
			masterLength = newMasterLength;
		}

		// Ratio knobs changed (setup a sync)
		bool syncRatios[4] = {false, false, false, false};// 0 index unused
		for (int i = 1; i < 4; i++) {
			newRatiosDoubled[i] = getRatioDoubled(i);
			if (scheduledReset)
				ratiosDoubled[i] = newRatiosDoubled[i];
			if (newRatiosDoubled[i] != ratiosDoubled[i]) {
				syncRatios[i] = true;// 0 index not used, but loop must start at i = 0
			}
		}
		
		// Swing and delay changed (for swing and delay info), ignore CV inputs for the info process
		for (int i = 0; i < 4; i++) {
			float newSwingVal = params[SWING_PARAMS + i].value;
			if (scheduledReset) {
				swingInfo[i] = 0l;
				swingVal[i] = newSwingVal; 
			}
			if (newSwingVal != swingVal[i]) {
				swingVal[i] = newSwingVal;
				swingInfo[i] = (long) (swingInfoTime * (float)sampleRate / displayRefreshStepSkips);// trigger swing info on channel i
				delayInfo[i] = 0l;// cancel delayed being displayed (if so)
			}
			if (i > 0) {
				int newDelayKnobIndex = clamp((int) round( params[DELAY_PARAMS + i].value ), 0, 8 - 1);
				if (scheduledReset) {
					delayInfo[i] = 0l;
					delayKnobIndexes[i] = newDelayKnobIndex;
				}
				if (newDelayKnobIndex != delayKnobIndexes[i]) {
					delayKnobIndexes[i] = newDelayKnobIndex;
					delayInfo[i] = (long) (delayInfoTime * (float)sampleRate / displayRefreshStepSkips);// trigger delay info on channel i
					swingInfo[i] = 0l;// cancel swing being displayed (if so)
				}
			}
		}			

		
		
		//********** Clocks and Delays **********
		
		// Clocks
		if (running) {
			// See if clocks finished their prescribed number of iteratios of double periods (and syncWait for sub) or 
			//    were forced reset and if so, recalc and restart them
			
			// Master clock
			if (clk[0].isReset()) {
				// See if ratio knobs changed (or unitinialized)
				for (int i = 1; i < 4; i++) {
					if (syncRatios[i]) {// always false for master
						clk[i].reset();// force reset (thus refresh) of that sub-clock
						ratiosDoubled[i] = newRatiosDoubled[i];
						syncRatios[i] = false;
					}
				}
				clk[0].setup(masterLength, 1, sampleTime);// must call setup before start. length = double_period
				clk[0].start();
			}
			
			// Sub clocks
			for (int i = 1; i < 4; i++) {
				if (clk[i].isReset()) {
					double length;
					int iterations;
					int ratioDoubled = ratiosDoubled[i];
					if (ratioDoubled < 0) { // if div 
						ratioDoubled *= -1;
						length = masterLength * ((double)ratioDoubled) / 2.0;
						iterations = 1l + (ratioDoubled % 2);		
						clk[i].setup(length, iterations, sampleTime);
					}
					else {// mult 
						length = (2.0f * masterLength) / ((double)ratioDoubled);
						iterations = ratioDoubled / (2l - (ratioDoubled % 2l));							
						clk[i].setup(length, iterations, sampleTime);
					}
					clk[i].start();
				}
			}

			
			// Write clk outputs into delay buffer
			for (int i = 0; i < 4; i++) {
				float pulseWidth = params[PW_PARAMS + i].value;
				if (i < 3 && inputs[PW_INPUTS + i].active) {
					pulseWidth += (inputs[PW_INPUTS + i].value / 10.0f) - 0.5f;
					pulseWidth = clamp(pulseWidth, 0.0f, 1.0f);
				}
				float swingAmount = swingVal[i];
				if (i < 3 && inputs[SWING_INPUTS + i].active) {
					swingAmount += (inputs[SWING_INPUTS + i].value / 5.0f) - 1.0f;
					swingAmount = clamp(swingAmount, -1.0f, 1.0f);
				}
				delay[i].write(clk[i].isHigh(swingAmount, pulseWidth));
			}
		
		
		
		//********** Outputs and lights **********
		
			// Clock outputs
			for (int i = 0; i < 4; i++) {
				long delaySamples = 0l;
				if (i > 0) {
					float delayFraction = delayValues[delayKnobIndexes[i]];
					float ratioValue = ((float)ratiosDoubled[i]) / 2.0f;
					if (ratioValue < 0)
						ratioValue = 1.0f / (-1.0f * ratioValue);
					delaySamples = (long)(masterLength * delayFraction * sampleRate / (ratioValue * 2.0));
				}
				outputs[CLK_OUTPUTS + i].value = delay[i].read(delaySamples) ? 10.0f : 0.0f;
			}
		}
		else {
			for (int i = 0; i < 4; i++) 
				outputs[CLK_OUTPUTS + i].value = 0.0f;
		}
		for (int i = 0; i < 4; i++)
			clk[i].stepClock();
			
		// Chaining outputs
		outputs[RESET_OUTPUT].value = (resetPulse.process((float)sampleTime) ? 10.0f : 0.0f);
		outputs[RUN_OUTPUT].value = (runPulse.process((float)sampleTime) ? 10.0f : 0.0f);
		outputs[BPM_OUTPUT].value =  inputs[BPM_INPUT].active ? inputs[BPM_INPUT].value : log2f(1.0f / masterLength);
			
		
		lightRefreshCounter++;
		if (lightRefreshCounter > displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			// Reset light
			lights[RESET_LIGHT].value =	resetLight;	
			resetLight -= (resetLight / lightLambda) * (float)sampleTime * displayRefreshStepSkips;
			
			// Run light
			lights[RUN_LIGHT].value = running ? 1.0f : 0.0f;
			
			// BPM light
			bool warningFlashState = true;
			if (cantRunWarning > 0l) 
				warningFlashState = calcWarningFlash(cantRunWarning, (long) (0.7 * sampleRate / displayRefreshStepSkips));
			lights[BPMSYNC_LIGHT + 0].value = (bpmDetectionMode && warningFlashState && inputs[BPM_INPUT].active) ? 1.0f : 0.0f;
			if (editingBpmMode < 0l)
				lights[BPMSYNC_LIGHT + 1].value = 1.0f;
			else
				lights[BPMSYNC_LIGHT + 1].value = (bpmDetectionMode && warningFlashState && inputs[BPM_INPUT].active) ? (float)((ppqn - 4)*(ppqn - 4))/400.0f : 0.0f;			
			
			// ratios synched lights
			for (int i = 1; i < 4; i++) {
				lights[CLK_LIGHTS + i].value = (syncRatios[i] && running) ? 1.0f: 0.0f;
			}

			// Incr/decr all counters related to step()
			for (int i = 0; i < 4; i++) {
				if (swingInfo[i] > 0)
					swingInfo[i]--;
				if (delayInfo[i] > 0)
					delayInfo[i]--;
			}
			if (cantRunWarning > 0l)
				cantRunWarning--;
			if (editingBpmMode != 0l) {
				if (editingBpmMode > 0l)
					editingBpmMode--;
				else
					editingBpmMode++;
			}
		}// lightRefreshCounter
		
		scheduledReset = false;
	}// step()
};


struct ClockedWidget : ModuleWidget {
	Clocked *module;
	DynamicSVGPanel *panel;
	int oldExpansion;
	int expWidth = 60;
	IMPort* expPorts[6];


	struct RatioDisplayWidget : TransparentWidget {
		Clocked *module;
		int knobIndex;
		std::shared_ptr<Font> font;
		char displayStr[4];
		const std::string delayLabelsClock[8] = {"  0", "/16",   "1/8",  "1/4", "1/3",     "1/2", "2/3",     "3/4"};
		const std::string delayLabelsNote[8]  = {"  0", "/64",   "/32",  "/16", "/8t",     "1/8", "/4t",     "/8d"};

		
		RatioDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}
		
		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box);
			nvgFontFaceId(vg, font->handle);
			//nvgTextLetterSpacing(vg, 2.5);

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, 16));
			nvgText(vg, textPos.x, textPos.y, "~~~", NULL);
			nvgFillColor(vg, textColor);
			if (module->swingInfo[knobIndex] > 0)
			{
				float swValue = module->swingVal[knobIndex];
				int swInt = (int)round(swValue * 99.0f);
				snprintf(displayStr, 4, " %2u", (unsigned) abs(swInt));
				if (swInt < 0)
					displayStr[0] = '-';
				if (swInt > 0)
					displayStr[0] = '+';
			}
			else if (module->delayInfo[knobIndex] > 0)
			{
				int delayKnobIndex = module->delayKnobIndexes[knobIndex];
				if (module->displayDelayNoteMode)
					snprintf(displayStr, 4, "%s", (delayLabelsNote[delayKnobIndex]).c_str());
				else
					snprintf(displayStr, 4, "%s", (delayLabelsClock[delayKnobIndex]).c_str());				
			}
			else {
				if (knobIndex > 0) {// ratio to display
					bool isDivision = false;
					int ratioDoubled = module->newRatiosDoubled[knobIndex];
					if (ratioDoubled < 0) {
						ratioDoubled = -1 * ratioDoubled;
						isDivision = true;
					}
					if ( (ratioDoubled % 2) == 1 )
						snprintf(displayStr, 4, "%c,5", 0x30 + (char)(ratioDoubled / 2));
					else {
						snprintf(displayStr, 4, "X%2u", (unsigned)(ratioDoubled / 2));
						if (isDivision)
							displayStr[0] = '/';
					}
				}
				else {// BPM to display
					if (module->editingBpmMode != 0l) {
						if (module->editingBpmMode > 0l) {
							if (!module->bpmDetectionMode)
								snprintf(displayStr, 4, " CV");
							else
								snprintf(displayStr, 4, "P%2u", (unsigned) module->ppqn);
						}
						else
							snprintf(displayStr, 4, " --");
					}
					else
						snprintf(displayStr, 4, "%3u", (unsigned)((120.0f / module->masterLength) + 0.5f));
				}
			}
			displayStr[3] = 0;// more safety
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};		
	
	struct PanelThemeItem : MenuItem {
		Clocked *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	struct ExpansionItem : MenuItem {
		Clocked *module;
		void onAction(EventAction &e) override {
			module->expansion = module->expansion == 1 ? 0 : 1;
		}
	};
	struct DelayDisplayNoteItem : MenuItem {
		Clocked *module;
		void onAction(EventAction &e) override {
			module->displayDelayNoteMode = !module->displayDelayNoteMode;
		}
	};
	struct EmitResetItem : MenuItem {
		Clocked *module;
		void onAction(EventAction &e) override {
			module->emitResetOnStopRun = !module->emitResetOnStopRun;
		}
	};	
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		Clocked *module = dynamic_cast<Clocked*>(this->module);
		assert(module);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Panel Theme";
		menu->addChild(themeLabel);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = lightPanelID;// ImpromptuModular.hpp
		lightItem->module = module;
		lightItem->theme = 0;
		menu->addChild(lightItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// ImpromptuModular.hpp
		darkItem->module = module;
		darkItem->theme = 1;
		menu->addChild(darkItem);

		menu->addChild(new MenuLabel());// empty line
		
		MenuLabel *settingsLabel = new MenuLabel();
		settingsLabel->text = "Settings";
		menu->addChild(settingsLabel);
		
		DelayDisplayNoteItem *ddnItem = MenuItem::create<DelayDisplayNoteItem>("Display Delay Values in Notes", CHECKMARK(module->displayDelayNoteMode));
		ddnItem->module = module;
		menu->addChild(ddnItem);

		EmitResetItem *erItem = MenuItem::create<EmitResetItem>("Emit Reset when Run is Turned Off", CHECKMARK(module->emitResetOnStopRun));
		erItem->module = module;
		menu->addChild(erItem);

		menu->addChild(new MenuLabel());// empty line
		
		MenuLabel *expansionLabel = new MenuLabel();
		expansionLabel->text = "Expansion module";
		menu->addChild(expansionLabel);

		ExpansionItem *expItem = MenuItem::create<ExpansionItem>(expansionMenuLabel, CHECKMARK(module->expansion != 0));
		expItem->module = module;
		menu->addChild(expItem);

		return menu;
	}	
	
	void step() override {
		if(module->expansion != oldExpansion) {
			if (oldExpansion!= -1 && module->expansion == 0) {// if just removed expansion panel, disconnect wires to those jacks
				for (int i = 0; i < 6; i++)
               rack::global_ui->app.gRackWidget->wireContainer->removeAllWires(expPorts[i]);
			}
			oldExpansion = module->expansion;		
		}
		box.size.x = panel->box.size.x - (1 - module->expansion) * expWidth;
		Widget::step();
	}
	
	ClockedWidget(Clocked *module) : ModuleWidget(module) {
 		this->module = module;
		oldExpansion = -1;
		
		// Main panel from Inkscape
        panel = new DynamicSVGPanel();
        panel->mode = &module->panelTheme;
        panel->expWidth = &expWidth;
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/Clocked.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/Clocked_dark.svg")));
        box.size = panel->box.size;
        box.size.x = box.size.x - (1 - module->expansion) * expWidth;
        addChild(panel);		
		
		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30-expWidth, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30-expWidth, 365), &module->panelTheme));


		static const int rowRuler0 = 50;//reset,run inputs, master knob and bpm display
		static const int rowRuler1 = rowRuler0 + 55;// reset,run switches
		//
		static const int rowRuler2 = rowRuler1 + 55;// clock 1
		static const int rowSpacingClks = 50;
		static const int rowRuler5 = rowRuler2 + rowSpacingClks * 2 + 55;// reset,run outputs, pw inputs
		
		
		static const int colRulerL = 18;// reset input and button, ratio knobs
		// First two rows and last row
		static const int colRulerSpacingT = 47;
		static const int colRulerT1 = colRulerL + colRulerSpacingT;// run input and button
		static const int colRulerT2 = colRulerT1 + colRulerSpacingT;// in and pwMaster inputs
		static const int colRulerT3 = colRulerT2 + colRulerSpacingT + 5;// swingMaster knob
		static const int colRulerT4 = colRulerT3 + colRulerSpacingT;// pwMaster knob
		static const int colRulerT5 = colRulerT4 + colRulerSpacingT;// clkMaster output
		// Three clock rows
		static const int colRulerM0 = colRulerL + 5;// ratio knobs
		static const int colRulerM1 = colRulerL + 60;// ratio displays
		static const int colRulerM2 = colRulerT3;// swingX knobs
		static const int colRulerM3 = colRulerT4;// pwX knobs
		static const int colRulerM4 = colRulerT5;// clkX outputs
		
		RatioDisplayWidget *displayRatios[4];
		
		// Row 0
		// Reset input
		addInput(createDynamicPort<IMPort>(Vec(colRulerL, rowRuler0), Port::INPUT, module, Clocked::RESET_INPUT, &module->panelTheme));
		// Run input
		addInput(createDynamicPort<IMPort>(Vec(colRulerT1, rowRuler0), Port::INPUT, module, Clocked::RUN_INPUT, &module->panelTheme));
		// In input
		addInput(createDynamicPort<IMPort>(Vec(colRulerT2, rowRuler0), Port::INPUT, module, Clocked::BPM_INPUT, &module->panelTheme));
		// Master BPM knob
		addParam(createDynamicParam<IMBigSnapKnob>(Vec(colRulerT3 + 1 + offsetIMBigKnob, rowRuler0 + offsetIMBigKnob), module, Clocked::RATIO_PARAMS + 0, (float)(module->bpmMin), (float)(module->bpmMax), 120.0f, &module->panelTheme));// must be a snap knob, code in step() assumes that a rounded value is read from the knob	(chaining considerations vs BPM detect)
		// BPM display
		displayRatios[0] = new RatioDisplayWidget();
		displayRatios[0]->box.pos = Vec(colRulerT4 + 11, rowRuler0 + vOffsetDisplay);
		displayRatios[0]->box.size = Vec(55, 30);// 3 characters
		displayRatios[0]->module = module;
		displayRatios[0]->knobIndex = 0;
		addChild(displayRatios[0]);
		
		// Row 1
		// Reset LED bezel and light
		addParam(ParamWidget::create<LEDBezel>(Vec(colRulerL + offsetLEDbezel, rowRuler1 + offsetLEDbezel), module, Clocked::RESET_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(colRulerL + offsetLEDbezel + offsetLEDbezelLight, rowRuler1 + offsetLEDbezel + offsetLEDbezelLight), module, Clocked::RESET_LIGHT));
		// Run LED bezel and light
		addParam(ParamWidget::create<LEDBezel>(Vec(colRulerT1 + offsetLEDbezel, rowRuler1 + offsetLEDbezel), module, Clocked::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(colRulerT1 + offsetLEDbezel + offsetLEDbezelLight, rowRuler1 + offsetLEDbezel + offsetLEDbezelLight), module, Clocked::RUN_LIGHT));
		// BPM mode and light
		addParam(ParamWidget::create<TL1105>(Vec(colRulerT2 + offsetTL1105, rowRuler1 + offsetTL1105), module, Clocked::BPMMODE_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(colRulerM1 + 62, rowRuler1 + offsetMediumLight), module, Clocked::BPMSYNC_LIGHT));		
		// Swing master knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerT3 + offsetIMSmallKnob, rowRuler1 + offsetIMSmallKnob), module, Clocked::SWING_PARAMS + 0, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		// PW master knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerT4 + offsetIMSmallKnob, rowRuler1 + offsetIMSmallKnob), module, Clocked::PW_PARAMS + 0, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		// Clock master out
		addOutput(createDynamicPort<IMPort>(Vec(colRulerT5, rowRuler1), Port::OUTPUT, module, Clocked::CLK_OUTPUTS + 0, &module->panelTheme));
		
		
		// Row 2-4 (sub clocks)		
		for (int i = 0; i < 3; i++) {
			// Ratio1 knob
			addParam(createDynamicParam<IMBigSnapKnob>(Vec(colRulerM0 + offsetIMBigKnob, rowRuler2 + i * rowSpacingClks + offsetIMBigKnob), module, Clocked::RATIO_PARAMS + 1 + i, (34.0f - 1.0f)*-1.0f, 34.0f - 1.0f, 0.0f, &module->panelTheme));		
			// Ratio display
			displayRatios[i + 1] = new RatioDisplayWidget();
			displayRatios[i + 1]->box.pos = Vec(colRulerM1, rowRuler2 + i * rowSpacingClks + vOffsetDisplay);
			displayRatios[i + 1]->box.size = Vec(55, 30);// 3 characters
			displayRatios[i + 1]->module = module;
			displayRatios[i + 1]->knobIndex = i + 1;
			addChild(displayRatios[i + 1]);
			// Sync light
			addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(colRulerM1 + 62, rowRuler2 + i * rowSpacingClks + 10), module, Clocked::CLK_LIGHTS + i + 1));		
			// Swing knobs
			addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerM2 + offsetIMSmallKnob, rowRuler2 + i * rowSpacingClks + offsetIMSmallKnob), module, Clocked::SWING_PARAMS + 1 + i, -1.0f, 1.0f, 0.0f, &module->panelTheme));
			// PW knobs
			addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerM3 + offsetIMSmallKnob, rowRuler2 + i * rowSpacingClks + offsetIMSmallKnob), module, Clocked::PW_PARAMS + 1 + i, 0.0f, 1.0f, 0.5f, &module->panelTheme));
			// Delay knobs
			addParam(createDynamicParam<IMSmallSnapKnob>(Vec(colRulerM4 + offsetIMSmallKnob, rowRuler2 + i * rowSpacingClks + offsetIMSmallKnob), module, Clocked::DELAY_PARAMS + 1 + i , 0.0f, 8.0f - 1.0f, 0.0f, &module->panelTheme));
		}

		// Last row
		// Reset out
		addOutput(createDynamicPort<IMPort>(Vec(colRulerL, rowRuler5), Port::OUTPUT, module, Clocked::RESET_OUTPUT, &module->panelTheme));
		// Run out
		addOutput(createDynamicPort<IMPort>(Vec(colRulerT1, rowRuler5), Port::OUTPUT, module, Clocked::RUN_OUTPUT, &module->panelTheme));
		// Out out
		addOutput(createDynamicPort<IMPort>(Vec(colRulerT2, rowRuler5), Port::OUTPUT, module, Clocked::BPM_OUTPUT, &module->panelTheme));
		// Sub-clock outputs
		addOutput(createDynamicPort<IMPort>(Vec(colRulerT3, rowRuler5), Port::OUTPUT, module, Clocked::CLK_OUTPUTS + 1, &module->panelTheme));	
		addOutput(createDynamicPort<IMPort>(Vec(colRulerT4, rowRuler5), Port::OUTPUT, module, Clocked::CLK_OUTPUTS + 2, &module->panelTheme));	
		addOutput(createDynamicPort<IMPort>(Vec(colRulerT5, rowRuler5), Port::OUTPUT, module, Clocked::CLK_OUTPUTS + 3, &module->panelTheme));	

		// Expansion module
		static const int rowRulerExpTop = 60;
		static const int rowSpacingExp = 50;
		static const int colRulerExp = 497 - 30 -150;// Clocked is (2+10)HP less than PS32
		addInput(expPorts[0] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 0), Port::INPUT, module, Clocked::PW_INPUTS + 0, &module->panelTheme));
		addInput(expPorts[1] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 1), Port::INPUT, module, Clocked::PW_INPUTS + 1, &module->panelTheme));
		addInput(expPorts[2] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 2), Port::INPUT, module, Clocked::PW_INPUTS + 2, &module->panelTheme));
		addInput(expPorts[3] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 3), Port::INPUT, module, Clocked::SWING_INPUTS + 0, &module->panelTheme));
		addInput(expPorts[4] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 4), Port::INPUT, module, Clocked::SWING_INPUTS + 1, &module->panelTheme));
		addInput(expPorts[5] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 5), Port::INPUT, module, Clocked::SWING_INPUTS + 2, &module->panelTheme));
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, Clocked) {
   Model *modelClocked = Model::create<Clocked, ClockedWidget>("Impromptu Modular", "Clocked", "CLK - Clocked", CLOCK_TAG);
   return modelClocked;
}

/*CHANGE LOG

0.6.10:
add ppqn setting of 12
move master PW to expansion panel and move BPM mode from right-click menu to main pannel button

0.6.9:
new approach to BPM Detection (all slaves must enable Use BPM Detect if master does, and same ppqn)
choice of 4, 8, 24 PPQN when using BPM detection
add sub-clock ratio of 24 (existing patches making use of greater than 23 mult or div will need to adjust)
add right click option for emit reset when run turned off

0.6.8:
replace bit-ring-buffer delay engine with event-based delay engine
add BPM pulse frequency vs CV level option in right click settings
updated BPM CV levels (in, out) to new Rack standard for clock CVs

0.6.7:
created

*/
