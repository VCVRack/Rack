#ifndef MODULE_MULTIOSCILLATOR_HPP
#define MODULE_MULTIOSCILLATOR_HPP
//----------------------------------------------------------------------
// Multi-oscillator (digital) for drawing.
//----------------------------------------------------------------------

#include "rack.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"

#define DEBUG 1


#define TROWA_MOSC_DEFAULT_NUM_OSCILLATORS		  3 // Default # of oscillators.
#define TROWA_MOSC_DEFAULT_NUM_OSC_OUTPUTS		  2 // For a given frequency, how many output signals
#define TROWA_MOSC_KNOB_MIN_V			   -10.0f // [V]
#define TROWA_MOSC_KNOB_MAX_V			    10.0f // [V]
#define TROWA_MOSC_INPUT_MIN_V			   -10.0f // [V]
#define TROWA_MOSC_INPUT_MAX_V			    10.0f // [V]
#define TROWA_MOSC_KNOB_AUX_MIN_V			 0.0f // [V] Min knob voltage for AUX
#define TROWA_MOSC_KNOB_AUX_MAX_V			10.0f // [V] Max knob voltage for AUX
#define TROWA_MOSC_AUX_MIN_V				-5.0f // [V] Min input voltage for AUX
#define TROWA_MOSC_AUX_MAX_V				 5.0f // [V] Max input voltage for AUX
#define TROWA_MOSC_TYPE_INPUT_MIN_V			-5.0f // [V] 
#define TROWA_MOSC_TYPE_INPUT_MAX_V			 5.0f // [V]
#define TROWA_MOSC_MIX_MIN_V				 0.0f // [V] Min input knob voltage for MIX
#define TROWA_MOSC_MIX_MAX_V				 1.0f // [V] Max input knob voltage for MIX

#define TROWA_MOSC_MIX_DEF_V				 0.5f // [V] Default knob voltage for MIX

#define MOSC_FREQ_DEFAULT_HZ			  432.0f // Default Frequency [Hz]
#define MOSC_PHASE_SHIFT_DEFAULT_DEG		0.0f // [degrees]
#define MOSC_OFFSET_DEFAULT_V				0.0f // [V]
#define MOSC_AMPLITUDE_DEFAULT_V			5.0f // Default Amplittude [V]

#define MOSC_FREQ_MIN_HZ					0.0f // Min Frequency [Hz]
#define MOSC_PHASE_SHIFT_MIN_DEG		 -360.0f // [degrees]. Since we will listen to up to -10V, if -5V is input limit this will still give -180
#define MOSC_OFFSET_MIN_V				  -10.0f // [V]
#define MOSC_AMPLITUDE_MIN_V			   -10.0f // Min Amplittude [V]

#define MOSC_FREQ_MAX_HZ				 20000.0f // Max Frequency [Hz]
#define MOSC_PHASE_SHIFT_MAX_DEG		  360.0f // [degrees].
#define MOSC_OFFSET_MAX_V		           10.0f // [V]
#define MOSC_AMPLITUDE_MAX_V		        10.0f // Max Amplitude [V]


//#define TROWA_MOSC_F_KNOB_MIN_V			   -20.0f // Frequency 
//#define TROWA_MOSC_F_KNOB_MAX_V			    20.0f // Frequency
#define TROWA_MOSC_F_KNOB_MIN_V			   MOSC_FREQ_MIN_HZ // Frequency 
#define TROWA_MOSC_F_KNOB_MAX_V			   MOSC_FREQ_MAX_HZ // Frequency
#define TROWA_MOSC_FREQ_KNOB_NEEDS_CONVERSION		0 // If Knob value is same as the frequency values, then we don't need to convert.


// Wave form type (SINE, SQUARE, TRIANGLE, SAW).
enum WaveFormType {
	// Sine
	WAVEFORM_SIN,
	// Triangle
	WAVEFORM_TRI,
	// Saw
	WAVEFORM_SAW,
	// Square
	WAVEFORM_SQR,
	// The number of wave forms.
	NUM_WAVEFORMS
};

// Oscillator output.
struct TS_OscillatorOutput {
	// Base param ids for the oscilator
	enum BaseParamIds {
		// What type of oscillator (SIN, SQU, TRI, SAW). [Voltage Range: +/- 10V]
		OUT_OSC_TYPE_PARAM,
		// Secondary parameter (i.e. Pulse width for Rectangle, Ramp slope sign for Saw).
		OUT_AUX_PARAM,
		// Phi_degrees: Phase Shift (degrees) [-360 to 360].
		OUT_PHASE_SHIFT_PARAM,
		// Mix of the raw with the AM signal. 1.0 is all AM; 0.0 is all raw.
		OUT_AM_MIX_PARAM,
		// Toggle (Digital or Ring Modulation)
		OUT_AM_TYPE_PARAM,
		// Number of params for an oscillator.
		OUT_NUM_PARAMS
	};
	// Base input ids for the oscilator
	enum BaseInputIds {
		// Oscillator type (SIN, SQU, TRI, SAW). [Voltage Range: +/- 5V].
		OUT_OSC_TYPE_INPUT,
		// Secondary input (i.e. Pulse width for Rectangle, Ramp slope sign for Saw).
		OUT_AUX_INPUT,
		// Phi_degrees: Phase Shift (degrees) [-360 to 360].  [Voltage Range: +/- 10V]
		OUT_PHASE_SHIFT_INPUT,
		// Amplitude modulation
		OUT_AM_INPUT,
		// Number of inputs for an oscillator.
		OUT_NUM_INPUTS
	};
	// Base output ids for the oscillator.
	enum BaseOutputIds {
		// Raw output.
		OUT_RAW_SIGNAL,
		// After amplitude modulation.
		OUT_MULTIPLIED_SIGNAL,
		// Number of outputs for an oscillator.
		OUT_NUM_OUTPUTS
	};
	// Base light ids for the oscillator. 
	enum BaseLightIds {
		OUT_OSC_TYPE_LED,
		// On = Ring modulation, OFf = Digital.
		OUT_AM_MODE_LED,
		OUT_NUM_LIGHTS
	};
	int outputChannelNumber = 0;
	// Phase shift (degrees) from user inputs (not CV).
	float ui_phaseShift_deg = 0.0f;
	// Phase shift (degrees) from user inputs and CV.
	float phaseShift_deg = 0.0f;
	// Phase shift (-1 to 1).
	float phaseShift_norm = 0.0f;
	// Which wave form to output.
	WaveFormType waveFormType = WaveFormType::WAVEFORM_SIN;

	// Waveform from knob/ui control (not CV).
	WaveFormType ui_waveFormType = WaveFormType::WAVEFORM_SIN;
	// [Rectangle] Pulse width (normalized 0-1).
	// [Ramp] Or >= 0.5f for positive Ramp, < 0.5f for negative Ramp.
	float auxParam_norm = 0.5;
	// For cycling through waveform types (btn).
	SchmittTrigger waveFormTrigger;
	// Digital (false) or Ring Mod (true)
	bool amRingModulation = false;
	// For AM mode (btn). Digital (false) or Ring Mod (true).
	SchmittTrigger amRingModulationTrigger;

#if DEBUG
	// For debugging:
	float lastPhi_deg = 0.0f;
#endif


	TS_OscillatorOutput()
	{
		initialize();
		return;
	}

	//--------------------------------------------------------
	// getRampSlope()
	// @returns: True for positive slope, false for negative slope.
	//--------------------------------------------------------
	bool getRampSlope()
	{
		return auxParam_norm >= 0.5f;
	}
	//--------------------------------------------------------
	// getRectPulseWidth()
	// @returns: Pulse width.
	//--------------------------------------------------------
	bool getRectPulseWidth()
	{
		return auxParam_norm;
	}

	//--------------------------------------------------------
	// initialize()
	// Initialize (UI) values to default values.
	//--------------------------------------------------------
	void initialize();
	//--------------------------------------------------------
	// serialize()
	// @returns : The TS_Oscillator json node.
	//--------------------------------------------------------
	json_t* serialize();
	//--------------------------------------------------------
	// deserialize()
	// @rootJ : (IN) The TS_Oscillator json node.
	//--------------------------------------------------------
	void deserialize(json_t* rootJ);
	//--------------------------------------------------------
	// setPhaseShift_deg()
	// @deg : (IN) The phase shift in degrees.
	//--------------------------------------------------------
	void setPhaseShift_deg(float deg);


};

// https://www.researchgate.net/publication/230561132_A_Simple_Digital_Model_of_the_Diode-Based_Ring-Modulator
struct TS_RingMod {
	// h is a parameter specifying the slope of the linear section. 
	float h = 0.5f;
	// vb is a parameter specifying the equivalent of the diode forward bias voltage. 
	// Per the googles: Silicon diodes have a forward voltage of approximately 0.7 volts. Germanium diodes have a forward voltage of approximately 0.3 volts.
	float v_b = 0.7f;
	// vL is a parameter giving the voltage beyond which the function is linear. 
	float v_L = 1.2f;

	inline float diode_shaping(float v_in)
	{
		if (v_in <= v_b)
		{
			// Zero
			return 0;
		}
		else if (v_in > v_L)
		{
			// Linear
			float temp = (v_L - v_b);
			return h * (v_in - v_L + temp * temp / (2 * (v_L - v_b)));
		}
		else //if (v_in <= v_L)
		{
			// Polynomial
			float temp = (v_in - v_b);
			return h * temp*temp / (2*(v_L - v_b));
		}
	}
	//----------------------------------------------------------
	// ringMod()
	// Emulate ring modulation. (Figure 4 in paper).
	// @v_in : (IN) Voltage input.
	// @v_c : (IN) Carrier voltage.
	// @returns : The modulator input.
	//----------------------------------------------------------
	float ringMod(float v_in, float v_c)
	{
		float voltageA = v_c + v_in / 2; // Top Input
		float voltageB = v_c - v_in / 2; // Bottom Input
		// Top Block - Bottom Block (Figure 4)
		return diode_shaping(voltageA) + diode_shaping(-voltageA) - (diode_shaping(voltageB) + diode_shaping(-voltageB));
	}
};

// A base oscillator (basically a frequency) with N waveform outputs based on this frequency
struct TS_Oscillator {
	// Base param ids for the oscilator
	enum BaseParamIds {
		// A_V: Amplitude (Volts). Should not be = 0 (pointless) and max +/-12 V.
		OSCWF_AMPLITUDE_PARAM,
		// f_Hz: Frequency (Hz)
		OSCWF_FREQUENCY_PARAM,
		// Phi_degrees: Phase Shift (degrees) [0-360]
		OSCWF_PHASE_SHIFT_PARAM,
		// y0_V : Offset (Volts). +/- 10 V?
		OSCWF_OFFSET_PARAM,
		// Sync/Restart waveform.
		OSCWF_SYNC_PARAM,
		// Number of params for an oscillator.
		OSCWF_NUM_PARAMS
	};
	// Base input ids for the oscilator
	enum BaseInputIds {
		// A_V: Amplitude (Volts). [Voltage Range: +/-12 V]
		OSCWF_AMPLITUDE_INPUT,
		// f_Hz: Frequency (Hz). [Voltage Range: +/- 10V]
		OSCWF_FREQUENCY_INPUT,
		// Phi_degrees: Phase Shift (degrees) [0-360].  [Voltage Range: +/- 10V]
		OSCWF_PHASE_SHIFT_INPUT,
		// y0_V : Offset (Volts).  [Voltage Range: +/- 10V]
		OSCWF_OFFSET_INPUT,
		// Sync/Restart waveform.
		OSCWF_SYNC_INPUT,
		// Frequency Modulator Input
		OSCWF_FM_INPUT,
		// Number of inputs for an oscillator.
		OSCWF_NUM_INPUTS
	};
	// Base output ids for the oscillator.
	enum BaseOutputIds {
		// Any time sync happens or oscillator is at 0 phase.
		OSCWF_SYNC_OUTPUT,
		// Number of outputs for an oscillator.
		OSCWF_NUM_OUTPUTS
	};
	// Base light ids for the oscillator. 
	enum BaseLightIds{
		OSCWF_SYNC_LED,
		OSCWF_NUM_LIGHTS
	};

	// Amplitutde (V) (used value).
	float amplitude_V = 1.0f;
	// Frequency (Hz) (used value).
	float frequency_Hz = 500.0f;
	// Phase shift (degrees) (used value).
	float phaseShift_deg = 0.0f;
	// Phase shift (-1 to 1).
	float phaseShift_norm = 0.0f;
	// Phase shift (radians) (used value).
	//float phaseShift_radians = 0.0f;
	// Offset (V) (used value).
	float offset_V = 0.0f;
	// Amplitutde (V) from user inputs (not CV).
	float ui_amplitude_V = 1.0f;
	// Frequency (Hz) from user inputs (not CV).
	float ui_frequency_Hz = 500.0f;
	// Phase shift (degrees) from user inputs (not CV).
	float ui_phaseShift_deg = 0.0f;
	// Offset (V) from user inputs (not CV).
	float ui_offset_V = 0.0f;
	// Phase from 0-1.
	float phase = 0.0f;
	// Shift phase from 0-1.
	float shiftedPhase = 0.0f;
	// The number of output waves.
	int numOutputWaveForms = TROWA_MOSC_DEFAULT_NUM_OSC_OUTPUTS;
	// The output waveforms
	std::vector<TS_OscillatorOutput> outputWaveforms;
	/// TODO: Figure out good h, v_b, and v_L.
	// Ring modulator.
	TS_RingMod ringModulator;

	// For synch button detection
	SchmittTrigger synchTrigger;
	// Sync out
	PulseGenerator synchPulse;
	//// If this oscillator should sync with another, the source oscillator index.
	//int syncSrcOscillatorIx = -1;
	//// If this step, the oscillator is restarted either by sync input or by reaching the end.
	//bool oscillatorRestart = false;

	//--------------------------------------------------------
	// TS_Oscillator()
	// @numOutWaveForms: (IN) The number of output waveforms we will have from this oscillator.
	//--------------------------------------------------------
	TS_Oscillator(int numOutWaveForms);
	//--------------------------------------------------------
	// TS_Oscillator()
	//--------------------------------------------------------
	TS_Oscillator() : TS_Oscillator(TROWA_MOSC_DEFAULT_NUM_OSC_OUTPUTS)
	{
		return;
	}
	//--------------------------------------------------------
	// ~TS_Oscillator()
	//--------------------------------------------------------
	~TS_Oscillator() 
	{
		outputWaveforms.clear();
		return;
	}
	//--------------------------------------------------------
	// initialize()
	// Initialize (UI) values to default values.
	//--------------------------------------------------------
	void initialize();
	//--------------------------------------------------------
	// serialize()
	// @returns : The TS_Oscillator json node.
	//--------------------------------------------------------
	json_t* serialize();
	//--------------------------------------------------------
	// deserialize()
	// @rootJ : (IN) The TS_Oscillator json node.
	//--------------------------------------------------------
	void deserialize(json_t* rootJ);
	//--------------------------------------------------------
	// calculatePhase()
	// @dt : (IN) Time elapsed.
	// @doSync : (IN) If sync / reset requested.
	// @returns: True if shifted phase has reset (gone over 1)
	//--------------------------------------------------------
	bool calculatePhase(float dt, bool doSync);
	//--------------------------------------------------------
	// setPhaseShift_deg()
	// @deg : (IN) The phase shift in degrees.
	//--------------------------------------------------------
	void setPhaseShift_deg(float deg);

	float calcSin();
	float calcSquare();
	float calcTri();
	float calcSaw();

	float calcSin(float phaseShift_n);
	float calcRect(float phaseShift_n, float pulseWidth_n);
	float calcTri(float phaseShift_n);
	float calcSaw(float phaseShift_n, bool posRamp);
};



//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiOscillator
// Simple digital oscillator for drawing.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct multiOscillator : Module {
	// User control parameters
	enum ParamIds {
		SYNC_PARAM,
		OSC_PARAM_START,
		NUM_PARAMS = OSC_PARAM_START // Add #oscillators * OSCWF_NUM_PARAMS to this
	};
	enum InputIds {
		SYNC_INPUT,
		OSC_INPUT_START,
		NUM_INPUTS = OSC_INPUT_START // Add # oscillators * OSCWF_NUM_INPUTS to this
	};
	enum OutputIds {
		SYNC_OUTPUT,
		OSC_OUTPUT_START,
		NUM_OUTPUTS = OSC_OUTPUT_START // Add # oscillators * OSCWF_NUM_OUTPUTS to this Determined by # of channels
	};
	enum LightIds {
		SYNC_LIGHT,
		OSC_LIGHT_START,
		NUM_LIGHTS = OSC_LIGHT_START // Add # oscillators
	};

	// Number of oscillators
	int numberOscillators = TROWA_MOSC_DEFAULT_NUM_OSCILLATORS;
	// Collection of oscillators.
	TS_Oscillator* oscillators = NULL;

	// The number of output signals from the oscillator.
	int numOscillatorOutputs = TROWA_MOSC_DEFAULT_NUM_OSC_OUTPUTS;

	// 3 letter wave form abbreviations.
	static const char* WaveFormAbbr[WaveFormType::NUM_WAVEFORMS];// = { "SIN", "TRI", "SAW", "SQR" };

	// If this has it controls configured.
	bool isInitialized = false;
	const float lightLambda = 0.005f;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiOscillator()
	// @numOscillators: (IN) Number of oscillators.
	// @numOscillatorOutputs: (IN) Number of oscillators output signals (channels per oscillator).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	multiOscillator(int numOscillators, int numOscillatorOutputs);

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiOscillator()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	multiOscillator() : multiOscillator(TROWA_MOSC_DEFAULT_NUM_OSCILLATORS, TROWA_MOSC_DEFAULT_NUM_OSC_OUTPUTS) {
		return;
	}
	~multiOscillator();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// findSyncedOscillators(void)
	// Try to find if osccilator A should be synced to oscillator B.
	// @returns: Number of oscillators that have sync.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	int findSyncedOscillators();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// initializeOscillators(void)
	// Set oscillators to default values.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void initialOscillators() {
		for (int i = 0; i < numberOscillators; i++)
		{
			oscillators[i].initialize();
		}
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// step(void)
	// Process.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// reset(void)
	// Initialize values.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void reset() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomize(void)
	// Randomize button stuff.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void randomize() override
	{
		for (int i = 0; i < numberOscillators; i++)
		{
			for (int j = 0; j < oscillators[i].numOutputWaveForms; j++)
			{
				oscillators[i].outputWaveforms[j].amRingModulation = (rand() % 100 > 50);
			}
		}
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// toJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *toJson() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// fromJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void fromJson(json_t *rootJ) override;

};

#endif // !MODULE_MULTIOSCILLATOR_HPP
