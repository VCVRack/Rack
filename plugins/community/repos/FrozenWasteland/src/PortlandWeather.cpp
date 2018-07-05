#include "FrozenWasteland.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"
#include "ringbuffer.hpp"
#include "StateVariableFilter.h"
#include "clouds/dsp/frame.h"
#include "clouds/dsp/fx/pitch_shifter.h"
#include <iostream>

#define HISTORY_SIZE (1<<22)
#define MAX_GRAIN_SIZE (1<<16)
#define NUM_TAPS 16
#define MAX_GRAINS 4
#define CHANNELS 2
#define DIVISIONS 21
#define NUM_GROOVES 16

namespace rack_plugin_FrozenWasteland {

struct PortlandWeather : Module {
	typedef float T;

	enum ParamIds {
		CLOCK_DIV_PARAM,
		TIME_PARAM,
		GRID_PARAM,
		GROOVE_TYPE_PARAM,
		GROOVE_AMOUNT_PARAM,
		GRAIN_QUANTITY_PARAM,
		GRAIN_SIZE_PARAM,
		FEEDBACK_PARAM,
		FEEDBACK_TAP_L_PARAM,
		FEEDBACK_TAP_R_PARAM,
		FEEDBACK_L_SLIP_PARAM,
		FEEDBACK_R_SLIP_PARAM,
		FEEDBACK_TONE_PARAM,
		FEEDBACK_L_PITCH_SHIFT_PARAM,
		FEEDBACK_R_PITCH_SHIFT_PARAM,
		FEEDBACK_L_DETUNE_PARAM,
		FEEDBACK_R_DETUNE_PARAM,		
		PING_PONG_PARAM,
		REVERSE_PARAM,
		MIX_PARAM,
		TAP_MUTE_PARAM,
		TAP_STACKED_PARAM = TAP_MUTE_PARAM+NUM_TAPS,
		TAP_MIX_PARAM = TAP_STACKED_PARAM+NUM_TAPS,
		TAP_PAN_PARAM = TAP_MIX_PARAM+NUM_TAPS,
		TAP_FILTER_TYPE_PARAM = TAP_PAN_PARAM+NUM_TAPS,
		TAP_FC_PARAM = TAP_FILTER_TYPE_PARAM+NUM_TAPS,
		TAP_Q_PARAM = TAP_FC_PARAM+NUM_TAPS,
		TAP_PITCH_SHIFT_PARAM = TAP_Q_PARAM+NUM_TAPS,
		TAP_DETUNE_PARAM = TAP_PITCH_SHIFT_PARAM+NUM_TAPS,
		CLEAR_BUFFER_PARAM = TAP_DETUNE_PARAM+NUM_TAPS,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		CLOCK_DIVISION_CV_INPUT,
		TIME_CV_INPUT,
		EXTERNAL_DELAY_TIME_INPUT,
		GRID_CV_INPUT,
		GROOVE_TYPE_CV_INPUT,
		GROOVE_AMOUNT_CV_INPUT,
		FEEDBACK_INPUT,
		FEEDBACK_TAP_L_INPUT,
		FEEDBACK_TAP_R_INPUT,
		FEEDBACK_TONE_INPUT,
		FEEDBACK_L_SLIP_CV_INPUT,
		FEEDBACK_R_SLIP_CV_INPUT,
		FEEDBACK_L_PITCH_SHIFT_CV_INPUT,
		FEEDBACK_R_PITCH_SHIFT_CV_INPUT,
		FEEDBACK_L_DETUNE_CV_INPUT,
		FEEDBACK_R_DETUNE_CV_INPUT,
		FEEDBACK_L_RETURN,
		FEEDBACK_R_RETURN,
		PING_PONG_INPUT,
		REVERSE_INPUT,
		MIX_INPUT,
		TAP_MUTE_CV_INPUT,
		TAP_STACK_CV_INPUT = TAP_MUTE_CV_INPUT + NUM_TAPS,
		TAP_MIX_CV_INPUT = TAP_STACK_CV_INPUT + NUM_TAPS,
		TAP_PAN_CV_INPUT = TAP_MIX_CV_INPUT + NUM_TAPS,
		TAP_FC_CV_INPUT = TAP_PAN_CV_INPUT + NUM_TAPS,
		TAP_Q_CV_INPUT = TAP_FC_CV_INPUT + NUM_TAPS,
		TAP_PITCH_SHIFT_CV_INPUT = TAP_Q_CV_INPUT + NUM_TAPS,
		TAP_DETUNE_CV_INPUT = TAP_PITCH_SHIFT_CV_INPUT + NUM_TAPS,
		IN_L_INPUT = TAP_DETUNE_CV_INPUT+NUM_TAPS,
		IN_R_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		FEEDBACK_L_OUTPUT,
		FEEDBACK_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PING_PONG_LIGHT,
		REVERSE_LIGHT,
		TAP_MUTED_LIGHT,
		TAP_STACKED_LIGHT = TAP_MUTED_LIGHT+NUM_TAPS,
		FREQ_LIGHT = TAP_STACKED_LIGHT+NUM_TAPS,
		NUM_LIGHTS
	};
	enum FilterModes {
		FILTER_NONE,
		FILTER_LOWPASS,
		FILTER_HIGHPASS,
		FILTER_BANDPASS,
		FILTER_NOTCH
	};

	struct LowFrequencyOscillator {
	float phase = 0.0;
	float freq = 1.0;
	bool invert = false;

	//void setFrequency(float frequency) {
	//	freq = frequency;
	//}

	void hardReset()
	{
		phase = 0.0;
	}

	void reset()
	{
		phase -= 1.0;
	}


	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5);
		phase += deltaPhase;
		//if (phase >= 1.0)
		//	phase -= 1.0;
	}
	float sin() {
		return sinf(2*M_PI * phase) * (invert ? -1.0 : 1.0);
	}
	float progress() {
		return phase;
	}
};

	const char* grooveNames[NUM_GROOVES] = {"Straight","Swing","Hard Swing","Reverse Swing","Alternate Swing","Accelerando","Ritardando","Waltz Time","Half Swing","Roller Coaster","Quintuple","Random 1","Random 2","Random 3","Early Reflection","Late Reflection"};
	const float tapGroovePatterns[NUM_GROOVES][NUM_TAPS] = {
		{1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,16.0f}, // Straight time
		{1.25f,2.0f,3.25f,4.0f,5.25f,6.0f,7.25f,8.0f,9.25f,10.0f,11.25f,12.0f,13.25f,14.0f,15.25f,16.0f}, // Swing
		{1.75f,2.0f,3.75,4.0f,5.75f,6.0f,7.75f,8.0f,9.75f,10.0f,11.75f,12.0f,13.75f,14.0f,15.75f,16.0f}, // Hard Swing
		{0.75f,2.0f,2.75f,4.0f,4.75f,6.0f,6.75f,8.0f,8.75f,10.0f,10.75f,12.0f,12.75f,14.0f,14.75f,16.0f}, // Reverse Swing
		{1.25f,2.0f,3.0f,4.0f,5.25f,6.0f,7.0f,8.0f,9.25f,10.0f,11.0f,12.0f,13.25f,14.0f,15.0f,16.0f}, // Alternate Swing
		{3.0f,5.0f,7.0f,9.0f,10.0f,11.0f,12.0f,13.0f,13.5f,14.0f,14.5f,15.0f,15.25f,15.5f,15.75f,16.0f}, // Accelerando
		{0.25f,0.5f,0.75f,1.0f,1.5f,2.0f,2.5f,3.0f,4.0f,5.0f,6.0f,7.0f,9.0f,11.0f,13.0f,16.0f}, // Ritardando
		{1.25f,2.75f,3.25f,4.0f,5.25f,6.75f,7.25f,8.0f,9.25f,10.75f,11.25f,12.0f,13.25f,14.75f,15.25f,16.0f}, // Waltz Time
		{1.5f,2.0f,3.5f,4.0f,5.0f,6.0f,7.0f,8.0f,9.5f,10.0f,11.5f,12.0f,13.0f,14.0f,15.0f,16.0f}, // Half Swing
		{1.0f,2.0f,4.0f,5.0f,6.0f,8.0f,10.0f,12.0f,12.5f,13.0f,13.5f,14.0f,14.5f,15.0f,15.5f,16.0f}, // Roller Coaster
		{1.75f,2.5f,3.25f,4.0f,4.75f,6.5f,7.25f,8.0f,9.75f,10.5f,11.25f,12.0f,12.75f,14.5f,15.25f,16.0f}, // Quintuple
		{0.25f,0.75f,1.0f,1.25f,4.0f,5.5f,7.25f,7.5f,8.0f,8.25f,10.0f,11.0f,13.5f,15.0f,15.75f,16.0f}, // Uniform Random 1
		{0.25f,4.75f,5.25f,5.5f,7.0f,8.0f,8.5f,8.75f,9.0f,9.25f,11.75f,12.75f,13.0f,13.25f,14.75f,15.5f}, // Uniform Random 2
		{0.75f,2.0f,2.25f,5.75f,7.25f,7.5f,7.75f,8.5f,8.75f,12.5f,12.75f,13.0f,13.75f,14.0f,14.5f,16.0f}, // Uniform Random 3
		{0.25f,0.5f,1.0f,1.25f,1.75f,2.0f,2.5f,3.5f,4.25f,4.5f,4.75f,5.0f,6.25f,8.25f,11.0f,16.0f}, // Early Reflection
		{7.0f,7.25f,9.0f,9.25f,10.25f,12.5f,13.0f,13.75f,14.0f,15.0f,15.25f,15.5f,15.75f,16.0f,16.0f,16.0f} // Late Reflection
	};

	const float minCutoff = 15.0;
	const float maxCutoff = 8400.0;

	int tapGroovePattern = 0;
	float grooveAmount = 1.0f;

	bool pingPong = false;
	bool reverse = false;
	int grainNumbers;
	bool tapMuted[NUM_TAPS+1];
	bool tapStacked[NUM_TAPS+1];
	int lastFilterType[NUM_TAPS+1];
	float lastTapFc[NUM_TAPS+1];
	float lastTapQ[NUM_TAPS+1];
	float tapPitchShift[NUM_TAPS+1];
	float tapDetune[NUM_TAPS+1];
	int tapFilterType[NUM_TAPS+1];
	int feedbackTap[CHANNELS] = {NUM_TAPS-1,NUM_TAPS-1};
	float feedbackSlip[CHANNELS] = {0.0f,0.0f};
	float feedbackPitch[CHANNELS] = {0.0f,0.0f};
	float feedbackDetune[CHANNELS] = {0.0f,0.0f};
	float delayTime[NUM_TAPS+1][CHANNELS];
	float actualDelayTime[NUM_TAPS+1][CHANNELS][2];
	float initialWindowedOutput[NUM_TAPS+1][CHANNELS][2];

	StateVariableFilterState<T> filterStates[NUM_TAPS][CHANNELS];
    StateVariableFilterParams<T> filterParams[NUM_TAPS];
	RCFilter lowpassFilter[CHANNELS];
	RCFilter highpassFilter[CHANNELS];

	const char* filterNames[5] = {"O","L","H","B","N"};
	
	clouds::PitchShifter pitch_shifter_[NUM_TAPS+1][CHANNELS][MAX_GRAINS];
	SchmittTrigger clockTrigger,pingPongTrigger,reverseTrigger,clearBufferTrigger,mutingTrigger[NUM_TAPS],stackingTrigger[NUM_TAPS];
	float divisions[DIVISIONS] = {1/256.0f,1/192.0f,1/128.0f,1/96.0f,1/64.0f,1/48.0f,1/32.0f,1/24.0f,1/16.0f,1/13.0f,1/12.0f,1/11.0f,1/8.0f,1/7.0f,1/6.0f,1/5.0f,1/4.0f,1/3.0f,1/2.0f,1/1.5f,1};
	const char* divisionNames[DIVISIONS] = {"/256","/192","/128","/96","/64","/48","/32","/24","/16","/13","/12","/11","/8","/7","/6","/5","/4","/3","/2","/1.5","x 1"};
	int division;
	float time = 0.0;
	float duration = 0;
	float baseDelay;
	bool secondClockReceived = false;

	LowFrequencyOscillator sinOsc[2];
	MultiTapDoubleRingBuffer<float, HISTORY_SIZE,NUM_TAPS+1> historyBuffer[CHANNELS][2];
	ReverseRingBuffer<float, HISTORY_SIZE> reverseHistoryBuffer[CHANNELS];
	float pitchShiftBuffer[NUM_TAPS+1][CHANNELS][MAX_GRAINS][MAX_GRAIN_SIZE];
	clouds::FloatFrame pitchShiftOut_;
	DoubleRingBuffer<float, 16> outBuffer[NUM_TAPS+1][CHANNELS][2]; 
	SampleRateConverter<1> src;
	float lastFeedback[CHANNELS] = {0.0f,0.0f};

	float lerp(float v0, float v1, float t) {
	  return (1 - t) * v0 + t * v1;
	}

	float SemitonesToRatio(float semiTone) {
		return powf(2,semiTone/12.0f);
	}

	PortlandWeather() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		sinOsc[1].phase = 0.25; //90 degrees out

		for (int i = 0; i <= NUM_TAPS; ++i) {
			tapMuted[i] = false;
			tapStacked[i] = false;
			tapPitchShift[i] = 0.0f;
			tapDetune[i] = 0.0f;
			filterParams[i].setMode(StateVariableFilterParams<T>::Mode::LowPass);
			filterParams[i].setQ(5); 	
	        filterParams[i].setFreq(T(800.0f / engineGetSampleRate()));

	        for(int j=0;j < CHANNELS;j++) {
	        	actualDelayTime[i][j][0] = 0.0f;
	        	actualDelayTime[i][j][1] = 0.0f;
	        	for(int k=0;k<MAX_GRAINS;k++) {
	    	 	   pitch_shifter_[i][j][k].Init(pitchShiftBuffer[i][j][k],k*0.25f);
	    		}
	    	}
	    }

	    //Initialize the feedback pitch shifters
        for(int j=0;j < CHANNELS;j++) {
        	for(int k=0;k<MAX_GRAINS;k++) {
    	 	   pitch_shifter_[NUM_TAPS][j][k].Init(pitchShiftBuffer[NUM_TAPS][j][k],k*0.25f);
    		}
    	}

	}

	const char* tapNames[NUM_TAPS+2] {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","ALL","EXT"};
	const char* grainNames[MAX_GRAINS] {"1","2","4","Raw"};


	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "pingPong", json_integer((int) pingPong));

		//json_object_set_new(rootJ, "reverse", json_integer((int) reverse));
		
		for(int i=0;i<NUM_TAPS;i++) {
			//This is so stupid!!! why did he not use strings?
			char buf[100];
			strcpy(buf, "muted");
			strcat(buf, tapNames[i]);
			json_object_set_new(rootJ, buf, json_integer((int) tapMuted[i]));

			strcpy(buf, "stacked");
			strcat(buf, tapNames[i]);
			json_object_set_new(rootJ, buf, json_integer((int) tapStacked[i]));
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {

		json_t *sumJ = json_object_get(rootJ, "pingPong");
		if (sumJ) {
			pingPong = json_integer_value(sumJ);			
		}

		//json_t *sumR = json_object_get(rootJ, "reverse");
		//if (sumR) {
		//	reverse = json_integer_value(sumR);			
		//}
		
		char buf[100];			
		for(int i=0;i<NUM_TAPS;i++) {
			strcpy(buf, "muted");
			strcat(buf, tapNames[i]);

			json_t *sumJ = json_object_get(rootJ, buf);
			if (sumJ) {
				tapMuted[i] = json_integer_value(sumJ);			
			}
		}

		for(int i=0;i<NUM_TAPS;i++) {
			strcpy(buf, "stacked");
			strcat(buf, tapNames[i]);

			json_t *sumJ = json_object_get(rootJ, buf);
			if (sumJ) {
				tapStacked[i] = json_integer_value(sumJ);
			}
		}
		
	}


	void step() override;
};


void PortlandWeather::step() {
	sinOsc[0].step(1.0 / engineGetSampleRate());
	sinOsc[1].step(1.0 / engineGetSampleRate());

	if (clearBufferTrigger.process(params[CLEAR_BUFFER_PARAM].value)) {
		for(int i=0;i<CHANNELS;i++) {
			historyBuffer[i][0].clear();
			historyBuffer[i][1].clear();
		}
	}


	tapGroovePattern = (int)clamp(params[GROOVE_TYPE_PARAM].value + (inputs[GROOVE_TYPE_CV_INPUT].value / 10.0f),0.0f,15.0);
	grooveAmount = clamp(params[GROOVE_AMOUNT_PARAM].value + (inputs[GROOVE_AMOUNT_CV_INPUT].value / 10.0f),0.0f,1.0f);

	float divisionf = params[CLOCK_DIV_PARAM].value;
	if(inputs[CLOCK_DIVISION_CV_INPUT].active) {
		divisionf +=(inputs[CLOCK_DIVISION_CV_INPUT].value * (DIVISIONS / 10.0));
	}
	divisionf = clamp(divisionf,0.0f,20.0f);
	division = (DIVISIONS-1) - int(divisionf); //TODO: Reverse Division Order

	time += 1.0 / engineGetSampleRate();
	if(inputs[CLOCK_INPUT].active) {
		if(clockTrigger.process(inputs[CLOCK_INPUT].value)) {
			if(secondClockReceived) {
				duration = time;
			}
			time = 0;
			//secondClockReceived = true;			
			secondClockReceived = !secondClockReceived;			
		}
		//lights[CLOCK_LIGHT].value = time > (duration/2.0);
	}
	
	if(inputs[CLOCK_INPUT].active) {
		baseDelay = duration / divisions[division];
	} else {
		baseDelay = clamp(params[TIME_PARAM].value + inputs[TIME_CV_INPUT].value, 0.001f, 10.0f);
		//baseDelay = clamp(params[TIME_PARAM].value, 0.001f, 10.0f);			
	}

	if (pingPongTrigger.process(params[PING_PONG_PARAM].value + inputs[PING_PONG_INPUT].value)) {
		pingPong = !pingPong;
	}
	lights[PING_PONG_LIGHT].value = pingPong;

	if (reverseTrigger.process(params[REVERSE_PARAM].value + inputs[REVERSE_INPUT].value)) {
		reverse = !reverse;
		if(reverse) {
			for(int channel =0;channel <CHANNELS;channel++) {
				reverseHistoryBuffer[channel].clear();
			}
		}
	}
	lights[REVERSE_LIGHT].value = reverse;

	grainNumbers = (int)params[GRAIN_QUANTITY_PARAM].value;

	for(int channel = 0;channel < CHANNELS;channel++) {
		// Get input to delay block
		float in = 0.0f;
		if(channel == 0) {
			in = inputs[IN_L_INPUT].value;
		} else {
			in = inputs[IN_R_INPUT].active ? inputs[IN_R_INPUT].value : inputs[IN_L_INPUT].value;			
		}
		feedbackTap[channel] = (int)clamp(params[FEEDBACK_TAP_L_PARAM+channel].value + (inputs[FEEDBACK_TAP_L_INPUT+channel].value / 10.0f),0.0f,17.0);
		feedbackSlip[channel] = clamp(params[FEEDBACK_L_SLIP_PARAM+channel].value + (inputs[FEEDBACK_L_SLIP_CV_INPUT+channel].value / 10.0f),-0.5f,0.5);
		float feedbackAmount = clamp(params[FEEDBACK_PARAM].value + (inputs[FEEDBACK_INPUT].value / 10.0f), 0.0f, 1.0f);
		float feedbackInput = lastFeedback[channel];

		float dry = in + feedbackInput * feedbackAmount;

		float dryToUse = dry; //Normally the same as dry unless in reverse mode

		// Push dry sample into reverse history buffer
		reverseHistoryBuffer[channel].push(dry);
		if(reverse) {
			float reverseDry = reverseHistoryBuffer[channel].shift();
			dryToUse = reverseDry;
		}

		// Push dry sample into history buffer
		for(int dualIndex=0;dualIndex<2;dualIndex++) {
			if (!historyBuffer[channel][dualIndex].full(NUM_TAPS-1)) {
				historyBuffer[channel][dualIndex].push(dryToUse);
			}
		}

		float wet = 0.0f; // This is the mix of delays and input that is outputed
		float feedbackValue = 0.0f; // This is the output of a tap that gets sent back to input
		float activeTapCount = 0.0f; // This will be used to normalize output
		for(int tap = 0; tap <= NUM_TAPS;tap++) { 

			// Stacking
			if (tap < NUM_TAPS -1 && stackingTrigger[tap].process(params[TAP_STACKED_PARAM+tap].value + inputs[TAP_STACK_CV_INPUT+tap].value)) {
				tapStacked[tap] = !tapStacked[tap];
			}

			//float pitch_grain_size = 1.0f; //Can be between 0 and 1
			float pitch_grain_size = params[GRAIN_SIZE_PARAM].value; //Can be between 0 and 1
			float pitch,detune;
			if (tap < NUM_TAPS) {
				pitch = floor(params[TAP_PITCH_SHIFT_PARAM+tap].value + (inputs[TAP_PITCH_SHIFT_CV_INPUT+tap].value*2.4f));
				detune = floor(params[TAP_DETUNE_PARAM+tap].value + (inputs[TAP_DETUNE_CV_INPUT+tap].value*10.0f));
				tapPitchShift[tap] = pitch;
				tapDetune[tap] = detune;
			} else {
				pitch = floor(params[FEEDBACK_L_PITCH_SHIFT_PARAM+channel].value + (inputs[FEEDBACK_L_PITCH_SHIFT_CV_INPUT+channel].value*2.4f));
				detune = floor(params[FEEDBACK_L_DETUNE_PARAM+channel].value + (inputs[FEEDBACK_L_DETUNE_CV_INPUT+channel].value*10.0f));		
				feedbackPitch[channel] = pitch;
				feedbackDetune[channel] = detune;		
			}
			pitch += detune/100.0f; 

			float delayMod = 0.0f;

			//Normally the delay tap is the same as the tap itself, unless it is stacked, then it is its neighbor;
			int delayTap = tap;
			while(delayTap < NUM_TAPS && tapStacked[delayTap]) {
				delayTap++;			
			}
			//Pull feedback off of normal tap time
			if(tap == NUM_TAPS && feedbackTap[channel] < NUM_TAPS) {
				delayTap = feedbackTap[channel];
				delayMod = feedbackSlip[channel] * baseDelay;
			}

			// Compute delay time in seconds
			float delay = baseDelay * lerp(tapGroovePatterns[0][delayTap],tapGroovePatterns[tapGroovePattern][delayTap],grooveAmount); //Balance between straight time and groove

			//External feedback time
			if(tap == NUM_TAPS && feedbackTap[channel] == NUM_TAPS+1) {
				delay = clamp(inputs[EXTERNAL_DELAY_TIME_INPUT].value, 0.001f, 10.0f);
			}


			if(inputs[TIME_CV_INPUT].active) { //The CV can change either clocked or set delay by 10MS
				delayMod += (0.001f * inputs[TIME_CV_INPUT].value); 
			}

			delayTime[tap][channel] = delay + delayMod;


			//Set reverse size
			if(tap == NUM_TAPS) {
				reverseHistoryBuffer[channel].setDelaySize((delay+delayMod) * engineGetSampleRate());
			}


			for(int dualIndex=0;dualIndex<2;dualIndex++) {
				//if((actualDelayTime[tap][channel][dualIndex] != delayTime[tap][channel] && sinOsc[dualIndex].progress() >= 1) || actualDelayTime[tap][channel][dualIndex] == 0.0f) {
				if((actualDelayTime[tap][channel][dualIndex] != delayTime[tap][channel]) || actualDelayTime[tap][channel][dualIndex] == 0.0f) {
					actualDelayTime[tap][channel][dualIndex] = delayTime[tap][channel];
					sinOsc[dualIndex].reset();									
				}

				float index = actualDelayTime[tap][channel][dualIndex] * engineGetSampleRate();

				// How many samples do we need consume to catch up?
				float consume = index - historyBuffer[channel][dualIndex].size(tap);

				if (outBuffer[tap][channel][dualIndex].empty()) {
					int inFrames = min(historyBuffer[channel][dualIndex].size(tap), 16); 
					
					double ratio = 1.0;
					if (consume <= -16) 
						ratio = 0.5;
					else if (consume >= 16) 
						ratio = 2.0;

					float inSR = engineGetSampleRate();
			        float outSR = ratio * inSR;

			        int outFrames = outBuffer[tap][channel][dualIndex].capacity();
			        src.setRates(inSR, outSR);
			        src.process((const Frame<1>*)historyBuffer[channel][dualIndex].startData(tap), &inFrames, (Frame<1>*)outBuffer[tap][channel][dualIndex].endData(), &outFrames);
			        outBuffer[tap][channel][dualIndex].endIncr(outFrames);
			        historyBuffer[channel][dualIndex].startIncr(tap, inFrames);
				}

				if (!outBuffer[tap][channel][dualIndex].empty()) {
					initialWindowedOutput[tap][channel][dualIndex] = outBuffer[tap][channel][dualIndex].shift();
				}
			}	

			float wetTap = 0.0f;	
			float initialOutput = (sinOsc[0].sin() * sinOsc[0].sin() * initialWindowedOutput[tap][channel][0]) + (sinOsc[1].sin() * sinOsc[1].sin() * initialWindowedOutput[tap][channel][1]);

			float grainVolumeScaling = 1;
			for(int k=0;k<MAX_GRAINS;k++) {
        		pitchShiftOut_.l = initialOutput;
				//Apply Pitch Shifting
			    pitch_shifter_[tap][channel][k].set_ratio(SemitonesToRatio(pitch));
			    pitch_shifter_[tap][channel][k].set_size(pitch_grain_size);

			    //TODO: Put back into outBuffer
			    bool useTriangleWindow = grainNumbers != 4;
			    pitch_shifter_[tap][channel][k].Process(&pitchShiftOut_,useTriangleWindow); 
			    if(k == 0) {
			    	wetTap +=pitchShiftOut_.l; //First one always use
			    } else if (k == 2 && grainNumbers >= 2) {
			    	wetTap +=pitchShiftOut_.l; //Use middle grain for 2
			    	grainVolumeScaling = 1.414;
			    } else if (k != 2 && grainNumbers == 3) {
			    	wetTap +=pitchShiftOut_.l; //Use them all
			    	grainVolumeScaling = 2;
			    }
			}
    		wetTap = wetTap / grainVolumeScaling;		        	
	        	 

			//Feedback tap doesn't get panned or filtered
        	if(tap < NUM_TAPS) {

				// Muting
				if (mutingTrigger[tap].process(params[TAP_MUTE_PARAM+tap].value + inputs[TAP_MUTE_CV_INPUT+tap].value)) {
					tapMuted[tap] = !tapMuted[tap];
					if(!tapMuted[tap]) {
						activeTapCount +=1.0f;
					}
				}

				float pan = 0.0f;
				if(channel == 0) {
					pan = clamp(1.0-(params[TAP_PAN_PARAM+tap].value + (inputs[TAP_PAN_CV_INPUT+tap].value / 10.0f)),0.0f,0.5f) * 2.0f;
				} else {
					pan = clamp(params[TAP_PAN_PARAM+tap].value + (inputs[TAP_PAN_CV_INPUT+tap].value / 10.0f),0.0f,0.5f) * 2.0f;
				}				
				wetTap = wetTap * clamp(params[TAP_MIX_PARAM+tap].value + (inputs[TAP_MIX_CV_INPUT+tap].value / 10.0f),0.0f,1.0f) * pan;
			

				int tapFilterType = (int)params[TAP_FILTER_TYPE_PARAM+tap].value;
				// Apply Filter to tap wet output			
				if(tapFilterType != FILTER_NONE) {
					if(tapFilterType != lastFilterType[tap]) {
						switch(tapFilterType) {
							case FILTER_LOWPASS:
							filterParams[tap].setMode(StateVariableFilterParams<T>::Mode::LowPass);
							break;
							case FILTER_HIGHPASS:
							filterParams[tap].setMode(StateVariableFilterParams<T>::Mode::HiPass);
							break;
							case FILTER_BANDPASS:
							filterParams[tap].setMode(StateVariableFilterParams<T>::Mode::BandPass);
							break;
							case FILTER_NOTCH:
							filterParams[tap].setMode(StateVariableFilterParams<T>::Mode::Notch);
							break;
						}					
					}

					float cutoffExp = clamp(params[TAP_FC_PARAM+tap].value + inputs[TAP_FC_CV_INPUT+tap].value / 10.0f,0.0f,1.0f); 
					float tapFc = minCutoff * powf(maxCutoff / minCutoff, cutoffExp) / engineGetSampleRate();
					if(lastTapFc[tap] != tapFc) {
						filterParams[tap].setFreq(T(tapFc));
						lastTapFc[tap] = tapFc;
					}
					float tapQ = clamp(params[TAP_Q_PARAM+tap].value + (inputs[TAP_Q_CV_INPUT+tap].value / 10.0f),0.01f,1.0f) * 50; 
					if(lastTapQ[tap] != tapQ) {
						filterParams[tap].setQ(tapQ); 
						lastTapQ[tap] = tapQ;
					}
					wetTap = StateVariableFilter<T>::run(wetTap, filterStates[tap][channel], filterParams[tap]);
				}
				lastFilterType[tap] = tapFilterType;

				if(tapMuted[tap]) {
					wetTap = 0.0f;
				}

				wet += wetTap;

				lights[TAP_STACKED_LIGHT+tap].value = tapStacked[tap];
				lights[TAP_MUTED_LIGHT+tap].value = (tapMuted[tap]);	

			} else {
				feedbackValue = wetTap;
			}
		}
		
		//activeTapCount = 16.0f;
		//wet = wet / activeTapCount * sqrt(activeTapCount);	

		if(feedbackTap[channel] == NUM_TAPS) { //This would be the All  Taps setting
			//float feedbackScaling = 4.0f; // Trying to make full feedback not, well feedback
			//feedbackValue = wet * feedbackScaling / NUM_TAPS; 
			feedbackValue = wet; 
		}

			
		//Apply global filtering
		// TODO Make it sound better
		float color = clamp(params[FEEDBACK_TONE_PARAM].value + inputs[FEEDBACK_TONE_INPUT].value / 10.0f, 0.0f, 1.0f);
		float lowpassFreq = 10000.0f * powf(10.0f, clamp(2.0f*color, 0.0f, 1.0f));
		lowpassFilter[channel].setCutoff(lowpassFreq / engineGetSampleRate());
		lowpassFilter[channel].process(feedbackValue);
		feedbackValue = lowpassFilter[channel].lowpass();
		float highpassFreq = 10.0f * powf(100.0f, clamp(2.0f*color - 1.0f, 0.0f, 1.0f));
		highpassFilter[channel].setCutoff(highpassFreq / engineGetSampleRate());
		highpassFilter[channel].process(feedbackValue);
		feedbackValue = highpassFilter[channel].highpass();


		outputs[FEEDBACK_L_OUTPUT+channel].value = feedbackValue;

		if(inputs[FEEDBACK_L_RETURN+channel].active) {
			feedbackValue = inputs[FEEDBACK_L_RETURN+channel].value;
		}

		//feedbackValue = clamp(feedbackValue,-5.0f,5.0f); // Let's keep things civil


		int feedbackDestinationChannel = channel;
		if (pingPong) {
			feedbackDestinationChannel = 1 - channel;
		}
		lastFeedback[feedbackDestinationChannel] = feedbackValue;

		float mix = clamp(params[MIX_PARAM].value + inputs[MIX_INPUT].value / 10.0f, 0.0f, 1.0f);
		float out = crossfade(in, wet, mix);  // Not sure this should be wet
		
		outputs[OUT_L_OUTPUT + channel].value = out;
	}
}

struct PWStatusDisplay : TransparentWidget {
	PortlandWeather *module;
	int frame = 0;
	std::shared_ptr<Font> fontNumbers,fontText;

	

	PWStatusDisplay() {
		fontNumbers = Font::load(assetPlugin(plugin, "res/fonts/01 Digit.ttf"));
		fontText = Font::load(assetPlugin(plugin, "res/fonts/DejaVuSansMono.ttf"));
	}

	void drawProgress(NVGcontext *vg, float phase) 
	{
		const float rotate90 = (M_PI) / 2.0;
		float startArc = 0 - rotate90;
		float endArc = (phase * M_PI * 2) - rotate90;

		// Draw indicator
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0x20, 0xff));
		{
			nvgBeginPath(vg);
			nvgArc(vg,75.8,170,35,startArc,endArc,NVG_CW);
			nvgLineTo(vg,75.8,170);
			nvgClosePath(vg);
		}
		nvgFill(vg);
	}

	void drawDivision(NVGcontext *vg, Vec pos, int division) {
		nvgFontSize(vg, 28);
		nvgFontFaceId(vg, fontNumbers->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", module->divisionNames[division]);
		nvgText(vg, pos.x, pos.y, text, NULL);
	}

	void drawDelayTime(NVGcontext *vg, Vec pos, float delayTime) {
		nvgFontSize(vg, 28);
		nvgFontFaceId(vg, fontNumbers->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%6.0f", delayTime*1000);	
		nvgText(vg, pos.x, pos.y, text, NULL);
	}

	void drawGrooveType(NVGcontext *vg, Vec pos, int grooveType) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, fontText->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", module->grooveNames[grooveType]);
		nvgText(vg, pos.x, pos.y, text, NULL);
	}

	void drawFeedbackTaps(NVGcontext *vg, Vec pos, int *feedbackTaps) {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, fontNumbers->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		for(int i=0;i<CHANNELS;i++) {
			char text[128];
			snprintf(text, sizeof(text), "%s", module->tapNames[feedbackTaps[i]]);
			nvgText(vg, pos.x + i*142, pos.y, text, NULL);
		}
	}

	void drawFeedbackPitch(NVGcontext *vg, Vec pos, float *feedbackPitch) {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, fontNumbers->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		for(int i=0;i<CHANNELS;i++) {
			char text[128];
			snprintf(text, sizeof(text), "%-2.0f", feedbackPitch[i]);
			nvgText(vg, pos.x + i*142, pos.y, text, NULL);
		}
	}

	void drawFeedbackDetune(NVGcontext *vg, Vec pos, float *feedbackDetune) {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, fontNumbers->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		for(int i=0;i<CHANNELS;i++) {
			char text[128];
			snprintf(text, sizeof(text), "%-3.0f", feedbackDetune[i]);
			nvgText(vg, pos.x + i*142, pos.y, text, NULL);
		}
	}


	void drawFilterTypes(NVGcontext *vg, Vec pos, int *filterType) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, fontText->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		for(int i=0;i<NUM_TAPS;i++) {
			char text[128];
			snprintf(text, sizeof(text), "%s", module->filterNames[filterType[i]]);
			nvgText(vg, pos.x + i*50, pos.y, text, NULL);
		}
	}

	void drawTapPitchShift(NVGcontext *vg, Vec pos, float *pitchShift) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, fontText->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		for(int i=0;i<NUM_TAPS;i++) {
			char text[128];
			snprintf(text, sizeof(text), "%-2.0f", pitchShift[i]);
			nvgText(vg, pos.x + i*50, pos.y, text, NULL);
		}
	}

	void drawTapDetune(NVGcontext *vg, Vec pos, float *detune) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, fontText->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		for(int i=0;i<NUM_TAPS;i++) {
			char text[128];
			snprintf(text, sizeof(text), "%-3.0f", detune[i]);
			nvgText(vg, pos.x + i*50, pos.y, text, NULL);
		}
	}

	void drawGrainNumbers(NVGcontext *vg, Vec pos, int grainNumbers) {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, fontNumbers->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", module->grainNames[grainNumbers-1]);
		nvgText(vg, pos.x, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		
		//drawProgress(vg,module->oscillator.progress());
		drawDivision(vg, Vec(150,65), module->division);
		drawDelayTime(vg, Vec(350,65), module->baseDelay);
		drawGrooveType(vg, Vec(147,125), module->tapGroovePattern);
		drawFeedbackTaps(vg, Vec(570,50), module->feedbackTap);
		drawFeedbackPitch(vg, Vec(570,150), module->feedbackPitch);
		drawFeedbackDetune(vg, Vec(570,200), module->feedbackDetune);
		drawFilterTypes(vg, Vec(80,420), module->lastFilterType);
		drawTapPitchShift(vg, Vec(78,585), module->tapPitchShift);
		drawTapDetune(vg, Vec(78,645), module->tapDetune);
		drawGrainNumbers(vg, Vec(800,60), module->grainNumbers);
	}
};


struct PortlandWeatherWidget : ModuleWidget {
	PortlandWeatherWidget(PortlandWeather *module);
};

PortlandWeatherWidget::PortlandWeatherWidget(PortlandWeather *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*57, RACK_GRID_HEIGHT * 2);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/PortlandWeather.svg")));
		addChild(panel);
	}


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 745)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 745)));

	{
		PWStatusDisplay *display = new PWStatusDisplay();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 500);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(57, 40), module, PortlandWeather::CLOCK_DIV_PARAM, 0, DIVISIONS-1, 0));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(257, 40), module, PortlandWeather::TIME_PARAM, 0.0f, 10.0f, 0.350f));
	//addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(257, 40), module, PortlandWeather::GRID_PARAM, 0.001f, 10.0f, 0.350f));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(57, 110), module, PortlandWeather::GROOVE_TYPE_PARAM, 0.0f, 15.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(257, 110), module, PortlandWeather::GROOVE_AMOUNT_PARAM, 0.0f, 1.0f, 1.0f));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(57, 180), module, PortlandWeather::FEEDBACK_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(157, 180), module, PortlandWeather::FEEDBACK_TONE_PARAM, 0.0f, 1.0f, 0.5f));

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(500, 30), module, PortlandWeather::FEEDBACK_TAP_L_PARAM, 0.0f, 17.0f, 15.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(642, 30), module, PortlandWeather::FEEDBACK_TAP_R_PARAM, 0.0f, 17.0f, 15.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(500, 80), module, PortlandWeather::FEEDBACK_L_SLIP_PARAM, -0.5f, 0.5f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(642, 80), module, PortlandWeather::FEEDBACK_R_SLIP_PARAM, -0.5f, 0.5f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(500, 130), module, PortlandWeather::FEEDBACK_L_PITCH_SHIFT_PARAM, -24.0f, 24.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(642, 130), module, PortlandWeather::FEEDBACK_R_PITCH_SHIFT_PARAM, -24.0f, 24.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(500, 180), module, PortlandWeather::FEEDBACK_L_DETUNE_PARAM, -99.0f, 99.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(642, 180), module, PortlandWeather::FEEDBACK_R_DETUNE_PARAM, -99.0f, 99.0f, 0.0f));

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(766, 40), module, PortlandWeather::GRAIN_QUANTITY_PARAM, 1, 4, 1));
	//addParam(ParamWidget::create<RoundBlackKnob>(Vec(766, 110), module, PortlandWeather::GRAIN_SIZE_PARAM, 8, 11, 11));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(766, 110), module, PortlandWeather::GRAIN_SIZE_PARAM, 0.0f, 1.0f, 1.0f));
	

	addParam(ParamWidget::create<CKD6>(Vec(766, 180), module, PortlandWeather::CLEAR_BUFFER_PARAM, 0.0f, 1.0f, 0.0f));


	addParam( ParamWidget::create<LEDButton>(Vec(372,182), module, PortlandWeather::REVERSE_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(376, 186), module, PortlandWeather::REVERSE_LIGHT));
	addInput(Port::create<PJ301MPort>(Vec(392, 178), Port::INPUT, module, PortlandWeather::REVERSE_INPUT));

	addParam( ParamWidget::create<LEDButton>(Vec(435,182), module, PortlandWeather::PING_PONG_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(439, 186), module, PortlandWeather::PING_PONG_LIGHT));
	addInput(Port::create<PJ301MPort>(Vec(455, 178), Port::INPUT, module, PortlandWeather::PING_PONG_INPUT));



	//last tap isn't stacked
	for (int i = 0; i< NUM_TAPS-1; i++) {
		addParam( ParamWidget::create<LEDButton>(Vec(54 + 50*i,239), module, PortlandWeather::TAP_STACKED_PARAM + i, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(58 + 50*i, 243), module, PortlandWeather::TAP_STACKED_LIGHT+i));
		addInput(Port::create<PJ301MPort>(Vec(74+ 50*i, 233), Port::INPUT, module, PortlandWeather::TAP_STACK_CV_INPUT+i));
	}

	for (int i = 0; i < NUM_TAPS; i++) {
		addParam( ParamWidget::create<LEDButton>(Vec(54 + 50*i,260), module, PortlandWeather::TAP_MUTE_PARAM + i, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(58 + 50*i, 264), module, PortlandWeather::TAP_MUTED_LIGHT+i));
		addInput(Port::create<PJ301MPort>(Vec(74+ 50*i, 260), Port::INPUT, module, PortlandWeather::TAP_MUTE_CV_INPUT+i));

		addParam( ParamWidget::create<RoundBlackKnob>(Vec(48 + 50*i, 280), module, PortlandWeather::TAP_MIX_PARAM + i, 0.0f, 1.0f, 0.5f));
		addInput(Port::create<PJ301MPort>(Vec(51+ 50*i, 311), Port::INPUT, module, PortlandWeather::TAP_MIX_CV_INPUT+i));
		addParam( ParamWidget::create<RoundBlackKnob>(Vec(48 + 50*i, 340), module, PortlandWeather::TAP_PAN_PARAM + i, 0.0f, 1.0f, 0.5f));
		addInput(Port::create<PJ301MPort>(Vec(51 + 50*i, 371), Port::INPUT, module, PortlandWeather::TAP_PAN_CV_INPUT+i));
		addParam( ParamWidget::create<RoundBlackKnob>(Vec(48 + 50*i, 400), module, PortlandWeather::TAP_FILTER_TYPE_PARAM + i, 0, 4, 0));
		addParam( ParamWidget::create<RoundBlackKnob>(Vec(48 + 50*i, 440), module, PortlandWeather::TAP_FC_PARAM + i, 0.0f, 1.0f, 0.5f));
		addInput(Port::create<PJ301MPort>(Vec(51 + 50*i, 471), Port::INPUT, module, PortlandWeather::TAP_FC_CV_INPUT+i));
		addParam( ParamWidget::create<RoundBlackKnob>(Vec(48 + 50*i, 500), module, PortlandWeather::TAP_Q_PARAM + i, 0.01f, 1.0f, 0.5f));
		addInput(Port::create<PJ301MPort>(Vec(51 + 50*i, 531), Port::INPUT, module, PortlandWeather::TAP_Q_CV_INPUT+i));
		addParam( ParamWidget::create<RoundBlackKnob>(Vec(48 + 50*i, 560), module, PortlandWeather::TAP_PITCH_SHIFT_PARAM + i, -24.0f, 24.0f, 0.0f));
		addInput(Port::create<PJ301MPort>(Vec(51 + 50*i, 591), Port::INPUT, module, PortlandWeather::TAP_PITCH_SHIFT_CV_INPUT+i));
		addParam( ParamWidget::create<RoundBlackKnob>(Vec(48 + 50*i, 620), module, PortlandWeather::TAP_DETUNE_PARAM + i, -99.0f, 99.0f, 0.0f));
		addInput(Port::create<PJ301MPort>(Vec(51 + 50*i, 651), Port::INPUT, module, PortlandWeather::TAP_DETUNE_CV_INPUT+i));
	}

	addInput(Port::create<PJ301MPort>(Vec(18, 50), Port::INPUT, module, PortlandWeather::CLOCK_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(100, 45), Port::INPUT, module, PortlandWeather::CLOCK_DIVISION_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(300, 45), Port::INPUT, module, PortlandWeather::TIME_CV_INPUT));
	//addInput(Port::create<PJ301MPort>(Vec(300, 45), Port::INPUT, module, PortlandWeather::GRID_CV_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(100, 115), Port::INPUT, module, PortlandWeather::GROOVE_TYPE_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(300, 115), Port::INPUT, module, PortlandWeather::GROOVE_AMOUNT_CV_INPUT));


	addInput(Port::create<PJ301MPort>(Vec(100, 185), Port::INPUT, module, PortlandWeather::FEEDBACK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(195, 182), Port::INPUT, module, PortlandWeather::FEEDBACK_TONE_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(270, 182), Port::INPUT, module, PortlandWeather::EXTERNAL_DELAY_TIME_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(538, 32), Port::INPUT, module, PortlandWeather::FEEDBACK_TAP_L_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(680, 32), Port::INPUT, module, PortlandWeather::FEEDBACK_TAP_R_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(538, 82), Port::INPUT, module, PortlandWeather::FEEDBACK_L_SLIP_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(680, 82), Port::INPUT, module, PortlandWeather::FEEDBACK_R_SLIP_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(538, 132), Port::INPUT, module, PortlandWeather::FEEDBACK_L_PITCH_SHIFT_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(680, 132), Port::INPUT, module, PortlandWeather::FEEDBACK_R_PITCH_SHIFT_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(538, 182), Port::INPUT, module, PortlandWeather::FEEDBACK_L_DETUNE_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(680, 182), Port::INPUT, module, PortlandWeather::FEEDBACK_R_DETUNE_CV_INPUT));



	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(440, 705), module, PortlandWeather::MIX_PARAM, 0.0f, 1.0f, 0.5f));
	addInput(Port::create<PJ301MPort>(Vec(480, 710), Port::INPUT, module, PortlandWeather::MIX_INPUT));


	addInput(Port::create<PJ301MPort>(Vec(75, 710), Port::INPUT, module, PortlandWeather::IN_L_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(105, 710), Port::INPUT, module, PortlandWeather::IN_R_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(200, 710), Port::OUTPUT, module, PortlandWeather::FEEDBACK_L_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(230, 710), Port::OUTPUT, module, PortlandWeather::FEEDBACK_R_OUTPUT));
	addInput(Port::create<PJ301MPort>(Vec(306, 710), Port::INPUT, module, PortlandWeather::FEEDBACK_L_RETURN));
	addInput(Port::create<PJ301MPort>(Vec(336, 710), Port::INPUT, module, PortlandWeather::FEEDBACK_R_RETURN));
	addOutput(Port::create<PJ301MPort>(Vec(595, 710), Port::OUTPUT, module, PortlandWeather::OUT_L_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(625, 710), Port::OUTPUT, module, PortlandWeather::OUT_R_OUTPUT));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, PortlandWeather) {
   Model *modelPortlandWeather = Model::create<PortlandWeather, PortlandWeatherWidget>("Frozen Wasteland", "PortlandWeather", "Portland Weather", DELAY_TAG);
   return modelPortlandWeather;
}
