#include "FrozenWasteland.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/digital.hpp"
#include "ringbuffer.hpp"
#include <iostream>

#define HISTORY_SIZE (1<<22)
#define NUM_TAPS 64
#define CHANNELS 2
#define DIVISIONS 21
#define NUM_PATTERNS 16
#define NUM_FEEDBACK_TYPES 4

namespace rack_plugin_FrozenWasteland {

struct HairPick : Module {
	typedef float T;

	enum ParamIds {
		CLOCK_DIV_PARAM,
		SIZE_PARAM,
		PATTERN_TYPE_PARAM,
		NUMBER_TAPS_PARAM,
		EDGE_LEVEL_PARAM,
		TENT_LEVEL_PARAM,
		TENT_TAP_PARAM,
		FEEDBACK_TYPE_PARAM,
		FEEDBACK_AMOUNT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		CLOCK_DIVISION_CV_INPUT,
		VOLT_OCTAVE_INPUT,
		SIZE_CV_INPUT,
		PATTERN_TYPE_CV_INPUT,
		NUMBER_TAPS_CV_INPUT,
		EDGE_LEVEL_CV_INPUT,
		TENT_LEVEL_CV_INPUT,
		TENT_TAP_CV_INPUT,
		FEEDBACK_TYPE_CV_INPUT,
		FEEDBACK_CV_INPUT,
		IN_L_INPUT,
		IN_R_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		DELAY_LENGTH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	enum FeedbackTypes {
		FEEDBACK_GUITAR,
		FEEDBACK_SITAR,
		FEEDBACK_CLARINET,
		FEEDBACK_RAW,
	};

	const char* combPatternNames[NUM_PATTERNS] = {"Uniform","Flat Middle","Early Comb","Fibonacci","Flat Comb","Late Comb","Rev. Fibonacci","Ess Comb","Rand Uniform","Rand Middle","Rand Early","Rand Fibonacci","Rand Flat","Rand Late","Rand Rev Fib","Rand Ess"};
	const float combPatterns[NUM_PATTERNS][NUM_TAPS] = {
		{1.0f,2.0f,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,25.0,26.0,27.0,28.0,29.0,30.0,31.0,32.0,33.0,34.0,35.0,36.0,37.0,38.0,39.0,40.0,41.0,42.0,43.0,44.0,45.0,46.0,47.0,48.0,49.0,50.0,51.0,52.0,53.0,54.0,55.0,56.0,57.0,58.0,59.0,60.0,61.0,62.0,63.0,64.0}, // Uniform
		{1.0f,2.0f,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0,24.5,25.0,25.5,26.0,26.5,27.0,27.5,28.0,28.5,29.0,29.5,30.0,30.5,31.5,32.0,32.5,33.0,33.5,34.0,34.5,35.0,35.5,36.0,36.5,37.0,37.5,38.0,38.5,39.0,39.5,40.0,48.0,49.0,50.0,51.0,52.0,53.0,54.0,55.0,56.0,57.0,58.0,59.0,60.0,61.0,62.0,63.0,64.0}, // Flat Middle
		{0.25f,0.5f,0.75f,1.0f,1.25f,1.5f,1.75f,2.0f,2.25f,2.5f,2.75f,3.0f,3.25f,3.5f,3.75f,4.0f,4.5f,5.0f,5.5f,6.0f,6.5f,7.0f,7.5f,8.0f,8.5f,9.0f,9.5f,10.0f,10.5f,11.0f,11.5f,12.0f,13.0f,14.0f,15.0f,16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,24.0f,25.0f,26.0f,27.0f,28.0f,30.0f,32.0f,34.0f,36.0f,38.0f,40.0f,42.0f,44.0f,46.0f,48.0f,50.0f,52.0f,55.0f,58.0f,61.0f,64.0f}, // Early Comb
		{0.031f,0.037f,0.043f,0.05f,0.056f,0.062f,0.068f,0.074f,0.087f,0.099f,0.111f,0.124f,0.142f,0.161f,0.18f,0.198f,0.229f,0.26f,0.291f,0.322f,0.372f,0.421f,0.471f,0.52f,0.601f,0.681f,0.762f,0.842f,0.972f,1.102f,1.232f,1.362f,1.573f,1.783f,1.994f,2.204f,2.545f,2.885f,3.226f,3.567f,4.118f,4.669f,5.22f,5.771f,6.663f,7.554f,8.446f,9.337f,10.78f,12.223f,13.666f,15.108f,17.443f,19.777f,22.111f,24.446f,28.223f,32.0f,35.777f,39.554f,45.666f,51.777f,57.889f,64.0f}, // Fibonacci
		{2.0f,4.0f,6.0,8.0,10.0,12.0,14.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,24.5,25.0,25.5,26.0,26.5,27.0,27.5,28.0,28.5,29.0,29.5,30.0,30.5,31.0,31.5,32.0,32.5,33.0,33.5,34.0,34.5,35.0,35.5,40.0,41.0,42.0,43.0,44.0,45.0,46.0,47.0,40.0,41.0,42.0,43.0,44.0,45.0,46.0,47.0,48.0,50.0,52.0,54.0,56.0,58.0,60.0,62.0,64.0}, // Flat Comb
		{3.0f,6.0f,9.0f,12.0f,14.0f,16.0f,18.0f,20.0f,22.0f,24.0f,26.0f,28.0f,30.0f,32.0f,34.0f,36.0f,37.0f,38.0f,39.0f,40.0f,41.0f,42.0f,43.0f,44.0f,45.0f,46.0f,47.0f,48.0f,49.0f,50.0f,51.0f,52.0f,52.5f,53.0f,53.5f,54.0f,54.5f,55.0f,55.5f,56.0f,56.5f,57.0f,57.5f,58.0f,58.5f,59.0f,59.5f,60.0f,60.25f,60.5f,60.75f,61.0f,61.25f,61.5f,61.75f,62.0f,62.25f,62.5f,62.75f,63.0f,63.25f,63.5f,63.75f,64.0f}, // Late Comb
		{0.031f,6.142f,12.254f,18.365f,24.477f,28.254f,32.031f,35.808f,39.585f,41.92f,44.254f,46.588f,48.923f,50.365f,51.808f,53.251f,54.693f,55.585f,56.477f,57.368f,58.26f,58.811f,59.362f,59.913f,60.464f,60.805f,61.146f,61.486f,61.827f,62.037f,62.248f,62.458f,62.669f,62.799f,62.929f,63.059f,63.189f,63.269f,63.35f,63.43f,63.511f,63.56f,63.61f,63.659f,63.709f,63.74f,63.771f,63.802f,63.833f,63.851f,63.87f,63.889f,63.907f,63.92f,63.932f,63.944f,63.957f,63.963f,63.969f,63.975f,63.981f,63.988f,63.994f,64.0f}, // Rev Fibonacci
		{0.5f, 1.0f, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 9.0, 10.0, 11.0, 12.0,13.0, 14.0, 15.0, 16.0, 18.0, 20.0, 22.0, 24.0, 26.0, 30.0, 32.0, 34.0, 36.0, 38.0, 40.0, 42.0, 44.0,46.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 53.5, 54.0, 55.0, 55.5, 67.0, 56.5, 57.0, 57.5, 58.0, 58.5,59.0, 59.5, 60.0, 60.5, 61.0, 61.5, 62.0, 62.5, 63.5,64.0}, // Ess Comb
		{1.258f,2.867f,3.545f,4.461f,5.618f,6.916f,7.936f,8.923f,9.572f,10.993f,11.38f,12.026f,13.936f,14.892f,15.736f,16.014f,17.361f,18.787f,19.587f,20.646f,21.276f,22.949f,23.776f,24.212f,25.823f,26.169f,27.944f,28.69f,29.081f,30.472f,31.842f,32.739f,33.38f,34.703f,35.724f,36.334f,37.394f,38.256f,39.527f,40.965f,41.654f,42.668f,43.56f,44.402f,45.806f,46.53f,47.313f,48.504f,49.486f,50.401f,51.495f,52.71f,53.992f,54.407f,55.861f,56.309f,57.091f,58.333f,59.638f,60.449f,61.972f,62.246f,63.842f,64.0f}, // Random Uniform
		{1.255f,2.488f,3.252f,4.298f,5.917f,6.708f,7.255f,8.082f,9.828f,10.863f,11.181f,12.218f,13.025f,14.911f,15.891f,16.885f,25.203f,25.61f,25.888f,26.543f,27.024f,27.042f,28.272f,28.45f,29.018f,29.213f,30.248f,30.414f,30.619f,32.049f,32.299f,33.134f,33.184f,33.506f,34.738f,35.368f,35.86f,36.116f,36.729f,37.229f,37.243f,37.79f,38.802f,39.038f,39.413f,40.111f,40.574f,48.727f,49.52f,50.863f,51.137f,52.111f,53.015f,54.411f,55.078f,56.231f,57.537f,58.981f,59.585f,60.899f,61.794f,62.007f,63.964f,64.0f}, // Random Flat Middle
		{0.519f,0.907f,1.046f,1.428f,1.513f,1.761f,1.778f,2.46f,2.532f,2.632f,3.049f,3.806f,4.057f,4.229f,4.381f,4.643f,4.812f,6.0f,6.408f,6.961f,7.386f,7.437f,8.008f,8.767f,9.292f,9.534f,9.947f,10.775f,11.216f,11.256f,12.19f,12.835f,13.474f,14.255f,15.6f,16.858f,17.551f,18.903f,19.838f,20.843f,21.639f,22.448f,23.15f,24.456f,25.991f,26.898f,27.615f,28.734f,30.359f,32.205f,34.68f,36.531f,38.956f,40.711f,42.76f,44.832f,46.899f,48.996f,50.255f,52.644f,55.831f,58.236f,61.704f,64.0f}, // Random Early Comb
		{0.353f,0.4f,0.408f,0.435f,0.458f,0.516f,0.529f,0.545f,0.557f,0.624f,0.626f,0.631f,0.695f,0.701f,0.705f,0.812f,0.823f,0.843f,0.87f,0.88f,1.028f,1.034f,1.066f,1.115f,1.168f,1.177f,1.32f,1.355f,1.588f,1.629f,1.64f,1.667f,2.343f,2.579f,2.599f,3.182f,3.268f,3.395f,3.697f,4.066f,5.02f,5.276f,5.329f,6.487f,6.749f,7.659f,9.04f,9.645f,10.997f,12.85f,13.996f,15.983f,18.059f,20.243f,22.64f,24.964f,28.876f,32.715f,35.883f,40.513f,46.186f,51.974f,57.948f,64.0f}, // Random Fibonacci
		{2.748f,4.499f,6.144f,8.714f,10.868f,12.113f,14.522f,16.561f,17.24f,18.734f,19.323f,20.399f,21.06f,22.009f,23.648f,24.649f,25.162f,25.282f,25.916f,26.128f,26.515f,27.296f,27.521f,28.801f,29.097f,29.135f,29.803f,30.726f,31.407f,31.455f,32.31f,32.72f,33.135f,33.189f,33.761f,34.614f,35.094f,35.548f,36.104f,40.283f,40.338f,41.026f,41.325f,42.119f,42.227f,43.504f,43.758f,44.016f,44.263f,45.679f,45.827f,46.661f,46.991f,47.772f,47.962f,48.453f,50.478f,52.343f,54.117f,56.733f,58.81f,60.796f,62.606f,64.0f}, // Random Flat Comb
		{3.295f,6.695f,9.844f,12.738f,14.884f,16.065f,18.612f,20.819f,22.418f,24.964f,26.659f,28.592f,30.977f,32.615f,34.054f,36.412f,37.115f,38.85f,39.301f,40.246f,41.363f,42.518f,43.872f,44.889f,45.561f,46.958f,47.21f,48.644f,49.094f,50.385f,50.443f,51.001f,52.025f,52.201f,53.049f,53.684f,53.732f,54.015f,55.074f,55.077f,55.563f,56.796f,57.155f,57.254f,57.687f,58.124f,59.307f,59.479f,60.035f,60.294f,60.348f,60.517f,60.707f,61.066f,61.553f,61.778f,61.926f,62.073f,62.456f,62.926f,63.105f,63.468f,63.475f,64.0f}, // Random Late Comb
		{0.072f,7.105f,12.843f,18.42f,24.766f,28.496f,32.434f,36.489f,40.163f,42.361f,45.122f,46.641f,49.864f,51.319f,52.808f,53.11f,54.105f,55.209f,56.226f,56.628f,57.493f,58.371f,59.075f,59.561f,59.764f,60.242f,61.078f,61.163f,61.183f,61.63f,61.843f,62.011f,62.132f,62.201f,62.386f,62.553f,62.775f,62.797f,62.869f,62.894f,62.933f,63.032f,63.047f,63.17f,63.206f,63.233f,63.249f,63.291f,63.319f,63.425f,63.443f,63.484f,63.581f,63.59f,63.608f,63.686f,63.714f,63.725f,63.737f,63.828f,63.889f,63.903f,63.905f,64.0f}, // Random Rev Fibonacci
		{1.227f,1.69f,2.356f,2.925f,3.035f,3.53f,4.379f,4.581f,4.872f,5.614f,6.15f,6.629f,6.876f,7.199f,8.111f,8.855f,9.744f,10.338f,11.417f,12.745f,13.09f,14.512f,15.197f,16.068f,18.112f,20.342f,22.943f,24.92f,26.793f,30.707f,32.794f,34.89f,36.117f,38.542f,40.327f,42.95f,44.243f,46.082f,48.205f,49.592f,50.992f,51.408f,52.914f,53.784f,53.947f,54.299f,55.104f,55.65f,56.713f,57.184f,58.025f,58.193f,59.061f,59.281f,59.715f,60.494f,61.289f,61.693f,61.813f,62.587f,63.459f,63.58f,63.868f,64.0f} //Random Ess Comb
	};

	const char* feedbackTypeNames[NUM_FEEDBACK_TYPES] = {"Guitar","Sitar","Clarinet","Raw"};


	int combPattern = 0;
	int feedbackType = 0;	
	float edgeLevel = 0.0f;
	float tentLevel = 1.0f;
	int tentTap = 32;
	
	SchmittTrigger clockTrigger;
	float divisions[DIVISIONS] = {1/256.0f,1/192.0f,1/128.0f,1/96.0f,1/64.0f,1/48.0f,1/32.0f,1/24.0f,1/16.0f,1/13.0f,1/12.0f,1/11.0f,1/8.0f,1/7.0f,1/6.0f,1/5.0f,1/4.0f,1/3.0f,1/2.0f,1/1.5f,1};
	const char* divisionNames[DIVISIONS] = {"/256","/192","/128","/96","/64","/48","/32","/24","/16","/13","/12","/11","/8","/7","/6","/5","/4","/3","/2","/1.5","x 1"};
	int division;
	float time = 0.0;
	float duration = 0;
	float baseDelay;
	bool secondClockReceived = false;


	bool combActive[NUM_TAPS];
	float combLevel[NUM_TAPS];


	MultiTapDoubleRingBuffer<float, HISTORY_SIZE,NUM_TAPS> historyBuffer[CHANNELS];
	DoubleRingBuffer<float, 16> outBuffer[NUM_TAPS][CHANNELS]; 
	SampleRateConverter<1> src;
	float lastFeedback[CHANNELS] = {0.0f,0.0f};

	float lerp(float v0, float v1, float t) {
	  return (1 - t) * v0 + t * v1;
	}

	int muteTap(int tapIndex)
    {
        int tapNumber = 0;
        for (int levelIndex = 0; levelIndex < 6; levelIndex += 1)
        {
            tapNumber += (tapIndex & (1 << levelIndex)) > 0 ? (1 << (5-levelIndex)) : 0;
        }
        return 64-tapNumber;     
    }

	float envelope(int tapNumber, float edgeLevel, float tentLevel, int tentNumber) {
        float t;
        if (tapNumber <= tentNumber)
        {
            t = (float)tapNumber / tentNumber;
            return (1 - t) * edgeLevel + t * tentLevel;
        }
        else
        {
            t = (float)(tapNumber-tentNumber) / (63-tentNumber);
            return (1 - t) * tentLevel+ t * edgeLevel;
        }
    }

	HairPick() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	const char* tapNames[NUM_TAPS+2] {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","ALL","EXT"};



	void step() override;
};


void HairPick::step() {

	combPattern = (int)clamp(params[PATTERN_TYPE_PARAM].value + (inputs[PATTERN_TYPE_CV_INPUT].value * 1.5f),0.0f,15.0);
	feedbackType = (int)clamp(params[FEEDBACK_TYPE_PARAM].value + (inputs[FEEDBACK_TYPE_CV_INPUT].value / 10.0f),0.0f,3.0);

	int tapCount = (int)clamp(params[NUMBER_TAPS_PARAM].value + (inputs[NUMBER_TAPS_CV_INPUT].value * 6.4f),1.0f,64.0);


	edgeLevel = clamp(params[EDGE_LEVEL_PARAM].value + (inputs[EDGE_LEVEL_CV_INPUT].value / 10.0f),0.0f,1.0);
	tentLevel = clamp(params[TENT_LEVEL_PARAM].value + (inputs[TENT_LEVEL_CV_INPUT].value / 10.0f),0.0f,1.0);

	tentTap = (int)clamp(params[TENT_TAP_PARAM].value + (inputs[TENT_TAP_CV_INPUT].value * 6.3f),1.0f,63.0f);


	//Initialize muting - set all active first
	for(int tapNumber = 0;tapNumber<NUM_TAPS;tapNumber++) {
		combActive[tapNumber] = true;	
	}
	//Turn off as needed
	for(int tapIndex = NUM_TAPS-1;tapIndex >= tapCount;tapIndex--) {
		int tapNumber = muteTap(tapIndex);
		combActive[tapNumber] = false;
	}

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
		baseDelay = clamp(params[SIZE_PARAM].value, 0.001f, 10.0f);			
	}
	float pitchShift = powf(2.0f,inputs[VOLT_OCTAVE_INPUT].value);
	baseDelay = baseDelay / pitchShift;
	outputs[DELAY_LENGTH_OUTPUT].value = baseDelay;
	
	for(int channel = 0;channel < CHANNELS;channel++) {
		// Get input to delay block
		float in = 0.0f;
		if(channel == 0) {
			in = inputs[IN_L_INPUT].value;
		} else {
			in = inputs[IN_R_INPUT].active ? inputs[IN_R_INPUT].value : inputs[IN_L_INPUT].value;			
		}
		float feedbackAmount = clamp(params[FEEDBACK_AMOUNT_PARAM].value + (inputs[FEEDBACK_CV_INPUT].value / 10.0f), 0.0f, 1.0f);
		float feedbackInput = lastFeedback[channel];

		float dry = in + feedbackInput * feedbackAmount;

		// Push dry sample into history buffer
		if (!historyBuffer[channel].full(NUM_TAPS-1)) {
			historyBuffer[channel].push(dry);
		}

		float delayNonlinearity = 1.0f;
		float percentChange = 10.0f;
		//Apply non-linearity
		if(feedbackType == FEEDBACK_SITAR) {
			if(in > 0) {
				delayNonlinearity = 1 + (in/(10.0f * 100.0f)*percentChange); //Test sitar will change length by 1%
			}
		}


		float wet = 0.0f; // This is the mix of delays and input that is outputed
		float feedbackValue = 0.0f; // This is the output of a tap that gets sent back to input
		for(int tap = 0; tap < NUM_TAPS;tap++) { 

			int inFrames = min(historyBuffer[channel].size(tap), 16); 
		
			// Compute delay time in seconds
			float delay = baseDelay * delayNonlinearity * combPatterns[combPattern][tap] / NUM_TAPS; 

			//float delayMod = 0.0f;
			// Number of delay samples
			float index = delay * engineGetSampleRate();


			// How many samples do we need consume to catch up?
			float consume = index - historyBuffer[channel].size(tap);

			if (outBuffer[tap][channel].empty()) {
				
				double ratio = 1.0;
				if (consume <= -16) 
					ratio = 0.5;
				else if (consume >= 16) 
					ratio = 2.0;

				float inSR = engineGetSampleRate();
		        float outSR = ratio * inSR;

		        int outFrames = outBuffer[tap][channel].capacity();
		        src.setRates(inSR, outSR);
		        src.process((const Frame<1>*)historyBuffer[channel].startData(tap), &inFrames, (Frame<1>*)outBuffer[tap][channel].endData(), &outFrames);
		        outBuffer[tap][channel].endIncr(outFrames);
		        historyBuffer[channel].startIncr(tap, inFrames);
			}

			float wetTap = 0.0f;
			if (!outBuffer[tap][channel].empty()) {
				wetTap = outBuffer[tap][channel].shift();
				if(tap == NUM_TAPS-1) {
					feedbackValue = wetTap;
				}
				if(!combActive[tap]) {
					wetTap = 0.0f;
				} else {
					wetTap = wetTap * envelope(tap,edgeLevel,tentLevel,tentTap);
				}
			}

			wet += wetTap;
		}

		wet = wet / ((float)tapCount) * sqrt((float)tapCount);

		float feedbackWeight = 0.5;
		switch(feedbackType) {
			case FEEDBACK_GUITAR :
				feedbackValue = (feedbackWeight * feedbackValue) + ((1-feedbackWeight) * lastFeedback[channel]);
				break;
			case FEEDBACK_SITAR :
				feedbackValue = (feedbackWeight * feedbackValue) + ((1-feedbackWeight) * lastFeedback[channel]);
				break;
			case FEEDBACK_CLARINET :
				feedbackValue = (feedbackWeight * feedbackValue) + ((1-feedbackWeight) * lastFeedback[channel]);
				break;
			case FEEDBACK_RAW :
				//feedbackValue = wet;
				break;
		}
		
		//feedbackValue = clamp(feedbackValue,-10.0f,10.0f);


		lastFeedback[channel] = feedbackValue;

		float out = wet;
		
		outputs[OUT_L_OUTPUT + channel].value = out;
	}
}

struct HPStatusDisplay : TransparentWidget {
	HairPick *module;
	int frame = 0;
	std::shared_ptr<Font> fontNumbers,fontText;

	

	HPStatusDisplay() {
		fontNumbers = Font::load(assetPlugin(plugin, "res/fonts/01 Digit.ttf"));
		fontText = Font::load(assetPlugin(plugin, "res/fonts/DejaVuSansMono.ttf"));

	}

	

	void drawDivision(NVGcontext *vg, Vec pos, int division) {
		nvgFontSize(vg, 20);
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

	void drawPatternType(NVGcontext *vg, Vec pos, int patternType) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, fontText->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", module->combPatternNames[patternType]);
		nvgText(vg, pos.x, pos.y, text, NULL);
	}

	void drawEnvelope(NVGcontext *vg, Vec pos,float edgeLevel, float tentLevel, int tentTap) {
		nvgStrokeWidth(vg, 1);
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0x20, 0xff));	
		nvgBeginPath(vg);
		float xOffset = (float)(tentTap-1)/64*100;
		nvgMoveTo(vg, pos.x+0, pos.y+20*(1-edgeLevel));
		nvgLineTo(vg, pos.x+xOffset, pos.y+20*(1-tentLevel));
		nvgLineTo(vg, pos.x+99, pos.y+20*(1-edgeLevel));
			
		nvgStroke(vg);
	}


	void drawFeedbackType(NVGcontext *vg, Vec pos, int feedbackType) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, fontText->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", module->feedbackTypeNames[feedbackType]);
		nvgText(vg, pos.x, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		
		//drawProgress(vg,module->oscillator.progress());
		drawDivision(vg, Vec(91,60), module->division);
		drawDelayTime(vg, Vec(350,65), module->baseDelay);
		drawPatternType(vg, Vec(64,135), module->combPattern);
		drawEnvelope(vg,Vec(55,166),module->edgeLevel,module->tentLevel,module->tentTap);
		drawFeedbackType(vg, Vec(60,303), module->feedbackType);
	}
};


struct HairPickWidget : ModuleWidget {
	HairPickWidget(HairPick *module);
};

HairPickWidget::HairPickWidget(HairPick *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*14, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/HairPick.svg")));
		addChild(panel);
	}


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 745)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 745)));

	{
		HPStatusDisplay *display = new HPStatusDisplay();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 200);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(45, 33), module, HairPick::CLOCK_DIV_PARAM, 0, DIVISIONS-1, 0));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(157, 33), module, HairPick::SIZE_PARAM, 0.001f, 10.0f, 0.350f));
	//addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(257, 40), module, HairPick::GRID_PARAM, 0.001f, 10.0f, 0.350f));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(17, 115), module, HairPick::PATTERN_TYPE_PARAM, 0.0f, 15.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(157, 115), module, HairPick::NUMBER_TAPS_PARAM, 1.0f, 64.0f, 64.0f));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(17, 200), module, HairPick::EDGE_LEVEL_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(87, 200), module, HairPick::TENT_LEVEL_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(157, 200), module, HairPick::TENT_TAP_PARAM, 0.0f, 63.0f, 32.0f));


	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(17, 283), module, HairPick::FEEDBACK_TYPE_PARAM, 0.0f, 3, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(157, 283), module, HairPick::FEEDBACK_AMOUNT_PARAM, 0.0f, 1.0f, 0.0f));


//	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(474, 189), module, HairPick::PING_PONG_LIGHT));


	

	addInput(Port::create<PJ301MPort>(Vec(12, 32), Port::INPUT, module, HairPick::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(12, 74), Port::INPUT, module, HairPick::VOLT_OCTAVE_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(50, 74), Port::INPUT, module, HairPick::CLOCK_DIVISION_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(162, 74), Port::INPUT, module, HairPick::SIZE_CV_INPUT));
	//addInput(Port::create<PJ301MPort>(Vec(300, 45), Port::INPUT, module, HairPick::GRID_CV_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(22, 156), Port::INPUT, module, HairPick::PATTERN_TYPE_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(162, 156), Port::INPUT, module, HairPick::NUMBER_TAPS_CV_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(22, 241), Port::INPUT, module, HairPick::EDGE_LEVEL_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(92, 241), Port::INPUT, module, HairPick::TENT_LEVEL_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(162, 241), Port::INPUT, module, HairPick::TENT_TAP_CV_INPUT));


	addInput(Port::create<PJ301MPort>(Vec(22, 324), Port::INPUT, module, HairPick::FEEDBACK_TYPE_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(162, 324), Port::INPUT, module, HairPick::FEEDBACK_CV_INPUT));


	addInput(Port::create<PJ301MPort>(Vec(50, 336), Port::INPUT, module, HairPick::IN_L_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(75, 336), Port::INPUT, module, HairPick::IN_R_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(110, 336), Port::OUTPUT, module, HairPick::OUT_L_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(135, 336), Port::OUTPUT, module, HairPick::OUT_R_OUTPUT));

	addOutput(Port::create<PJ301MPort>(Vec(130, 74), Port::OUTPUT, module, HairPick::DELAY_LENGTH_OUTPUT));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, HairPick) {
   Model *modelHairPick = Model::create<HairPick, HairPickWidget>("Frozen Wasteland", "HairPick", "Hair Pick", FILTER_TAG);
   return modelHairPick;
}
