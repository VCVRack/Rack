#include "Module_multiOscillator.hpp"
#include "util/math.hpp"
#include "Widget_multiOscillator.hpp"



const char* multiOscillator::WaveFormAbbr[WaveFormType::NUM_WAVEFORMS] = { "SIN", "TRI", "SAW", "SQR" };

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//::::::::::::: TS_Oscillator :::::::::::::::::::::::::::::
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

//--------------------------------------------------------
// TS_Oscillator()
// @numOutWaveForms: (IN) The number of output waveforms we will have from this oscillator.
//--------------------------------------------------------
TS_Oscillator::TS_Oscillator(int numOutWaveForms)
{
	numOutputWaveForms = numOutWaveForms;
	if (numOutputWaveForms < 1)
		numOutputWaveForms = 1;
	for (int i = 0; i < numOutputWaveForms; i++)
	{		
		outputWaveforms.push_back(TS_OscillatorOutput());
		outputWaveforms[i].outputChannelNumber = i + 1;
	}
	initialize();
	return;
}
//--------------------------------------------------------
// initialize()
// Initialize (UI) values to default values.
//--------------------------------------------------------
void TS_Oscillator::initialize()
{
	ui_amplitude_V = MOSC_AMPLITUDE_DEFAULT_V;
	ui_frequency_Hz = MOSC_FREQ_DEFAULT_HZ;
	ui_phaseShift_deg = MOSC_PHASE_SHIFT_DEFAULT_DEG;
	ui_offset_V = MOSC_OFFSET_DEFAULT_V;

	for (int i = 0; i < static_cast<int>(outputWaveforms.size()); i++)
	{
		outputWaveforms[i].initialize();
	}

	return;
} // end initialize()
//--------------------------------------------------------
// serialize()
// @returns : The TS_Oscillator json node.
//--------------------------------------------------------
json_t* TS_Oscillator::serialize()
{
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "amplitude_V", json_real(ui_amplitude_V));
	json_object_set_new(rootJ, "frequency_Hz", json_real(ui_frequency_Hz));
	json_object_set_new(rootJ, "phaseShift_deg", json_real(ui_phaseShift_deg));
	json_object_set_new(rootJ, "offset_V", json_real(ui_offset_V));
	json_object_set_new(rootJ, "numWaveforms", json_integer(outputWaveforms.size()));
	json_t* waveformsJ = json_array();
	for (int i = 0; i < static_cast<int>(outputWaveforms.size()); i++)
	{
		json_array_append_new(waveformsJ, outputWaveforms[i].serialize());
	}
	json_object_set_new(rootJ, "waveforms", waveformsJ);

	return rootJ;	
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The TS_Oscillator json node.
//--------------------------------------------------------
void TS_Oscillator::deserialize(json_t* rootJ)
{
	json_t* currJ = NULL;
	if (rootJ) {
		currJ = json_object_get(rootJ, "amplitude_V");
		if (currJ)
			ui_amplitude_V = json_real_value(currJ);
		currJ = json_object_get(rootJ, "frequency_Hz");
		if (currJ)
			ui_frequency_Hz = json_real_value(currJ);
		currJ = json_object_get(rootJ, "phaseShift_deg");
		if (currJ)
			ui_phaseShift_deg = json_real_value(currJ);
		currJ = json_object_get(rootJ, "offset_V");
		if (currJ)
			ui_offset_V = json_real_value(currJ);
		currJ = json_object_get(rootJ, "numWaveforms");
		if (currJ)
			numOutputWaveForms = json_integer_value(currJ);
		if (numOutputWaveForms > static_cast<int>(outputWaveforms.size()))
			numOutputWaveForms = static_cast<int>(outputWaveforms.size());
		json_t* waveformsJ = json_object_get(rootJ, "waveforms");
		for (int i = 0; i < numOutputWaveForms; i++)
		{
			currJ = json_array_get(waveformsJ, i);
			if (currJ)
			{
				outputWaveforms[i].deserialize(currJ);
			}
		}
	}
} // end deserialize()

//--------------------------------------------------------
// setPhaseShift_deg()
// @deg : (IN) The phase shift in degrees.
//--------------------------------------------------------
void TS_Oscillator::setPhaseShift_deg(float deg)
{
	phaseShift_deg = deg;
	phaseShift_norm = deg / 360.0f;
	return;
}

//--------------------------------------------------------
// calculatePhase()
// @dt : (IN) Time elapsed.
// @doSync : (IN) If sync / reset requested.
// @returns: True if shifted phase has reset (gone over 1)
//--------------------------------------------------------
bool TS_Oscillator::calculatePhase(float dt, bool doSync)
{
	bool waveReset = doSync;
	if (doSync)
	{
		phase = 0;
	}
	else
	{
		// VCO clamps this, not sure if we need to
		float dPhase = clamp(frequency_Hz * dt, 0.f, 0.5f);
		phase = eucmod(phase + dPhase, 1.0f);
	}
	float prevSPhi = shiftedPhase;
	shiftedPhase = eucmod(phase + phaseShift_norm, 1.0f);
	if (!waveReset)
		waveReset = prevSPhi > shiftedPhase;
	return waveReset;
}

//--------------------------------------------------------
// calcSin()
// Sine wave.
// Calculate with amplitude and offset.
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
//--------------------------------------------------------
float TS_Oscillator::calcSin(float phaseShift_n)
{
	return amplitude_V * sinf(eucmod(1.0f + shiftedPhase + phaseShift_n, 1.0f) * 2.0f * NVG_PI) + offset_V;
} // end calcSin()
//--------------------------------------------------------
// calcRect()
// Rectangle wave.
// Calculate with amplitude and offset.
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
// @pulseWidth_n : (IN) Normalized pulse width (0-1). Really should be like 0.01 to 0.99 or something.
//--------------------------------------------------------
float TS_Oscillator::calcRect(float phaseShift_n, float pulseWidth_n)
{
	float val = 0.0f;
	if (eucmod(1.0f + shiftedPhase + phaseShift_n, 1.0f) < pulseWidth_n)
		val = amplitude_V;
	else
		val = -amplitude_V;
	return val + offset_V;
} // end calcRect
//--------------------------------------------------------
// calcTri()
// Triange wave.
// Calculate with amplitude and offset.
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
//--------------------------------------------------------
float TS_Oscillator::calcTri(float phaseShift_n)
{
	float p_n = eucmod(1.0f + shiftedPhase + phaseShift_n, 1.0f);
	float val = 0.0f;
	if (p_n < 0.25f)
		val = 4.0f * p_n; // 0 to 1 (positive slope)
	else if (p_n < 0.75f)
		val = 2.0f - 4.0f * p_n; // -1 to 0 (positive slope)
	else
		val = -4.0f + 4.f * p_n;
	return amplitude_V * val + offset_V;
} // end calcTri()
//--------------------------------------------------------
// calcSaw()
// Sawtooth wave.
// Calculate with amplitude and offset. (centered around 0)
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
// @posRamp: (IN) True for positive ramp, false for negative ramp.
//--------------------------------------------------------
float TS_Oscillator::calcSaw(float phaseShift_n, bool posRamp)
{
	float p_n = eucmod(1.0f + shiftedPhase + phaseShift_n, 1.0f);
	float val = 0.0f;
	float a_v = 2 * amplitude_V;
	if (posRamp)
		val = -amplitude_V + a_v * p_n; // Going up /|/|/|
	else
		val = amplitude_V - a_v * p_n; // Going down \|\|\|
	return val + offset_V;
} // end calcSaw()

//--------------------------------------------------------
// calcSin()
// Calculates A*sin(wt + phi) + C.
// A = amplitude_V, w = frequency_radiansps, phi = phaseShift_radians, C = offset_V.
//--------------------------------------------------------
float TS_Oscillator::calcSin()
{
	return amplitude_V * sinf(shiftedPhase * 2.0f * NVG_PI) + offset_V;
}
//--------------------------------------------------------
// calcSquare()
// Pseudo calculates A*SIGN(sin(wt + phi)) + C.
// A = amplitude_V, w = frequency_radiansps, phi = phaseShift_radians, C = offset_V.
// SIGN() = +1 for positive, -1 for negative, 0 for 0.
//--------------------------------------------------------
float TS_Oscillator::calcSquare()
{
	float val = 0.0f;
	if (shiftedPhase < 0.5f)
		val = amplitude_V;
	else
		val = -amplitude_V;
	return val + offset_V;
}
//--------------------------------------------------------
// calcTri()
// Pseudo calculates A*SIGN(sin(wt + phi)) + C.
// A = amplitude_V, w = frequency_radiansps, phi = phaseShift_radians, C = offset_V.
// SIGN() = +1 for positive, -1 for negative, 0 for 0.
//--------------------------------------------------------
float TS_Oscillator::calcTri()
{
	float val = 0.0f;
	if (shiftedPhase < 0.25f)
		val = 4.0f * shiftedPhase; // 0 to 1 (positive slope)
	else if (shiftedPhase < 0.75f)
		val = 2.0f - 4.0f * shiftedPhase; // -1 to 0 (positive slope)
	else
		val = -4.0f + 4.f * shiftedPhase;
	return amplitude_V * val + offset_V;
}
//--------------------------------------------------------
// calcSaw()
// Sawtooth wave, positive ramp.
//--------------------------------------------------------
float TS_Oscillator::calcSaw()
{
	float val = 0.0f;
	if (shiftedPhase < 0.5f)
		val = 2.f * shiftedPhase;
	else
		val = -2.f + 2.f * shiftedPhase;
	return amplitude_V * val + offset_V;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//::::::::::::: multiOscillator :::::::::::::::::::::::::::
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiOscillator()
// Create a module with numOscillators oscillators.
// @numOscillators: (IN) Number of oscillators
// @numOscillatorOutputs: (IN) Number of oscillators.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiOscillator::multiOscillator(int numOscillators, int numOscillatorOutputs) :
	Module(NUM_PARAMS + numOscillators * (TS_Oscillator::OSCWF_NUM_PARAMS + numOscillatorOutputs * TS_OscillatorOutput::OUT_NUM_PARAMS),
		   NUM_INPUTS + numOscillators * (TS_Oscillator::OSCWF_NUM_INPUTS + numOscillatorOutputs * TS_OscillatorOutput::OUT_NUM_INPUTS),
		   NUM_OUTPUTS + numOscillators * (TS_Oscillator::OSCWF_NUM_OUTPUTS + numOscillatorOutputs * TS_OscillatorOutput::OUT_NUM_OUTPUTS),
		   NUM_LIGHTS + numOscillators * (TS_Oscillator::OSCWF_NUM_LIGHTS + numOscillatorOutputs * TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS))
{
	this->numberOscillators = numOscillators;
	this->oscillators = new TS_Oscillator[numberOscillators];
	this->numOscillatorOutputs = numOscillatorOutputs;
	for (int i = 0; i < numOscillators; i++)
	{
		oscillators[i] = TS_Oscillator(numOscillatorOutputs);
	}
	//this->theOscillator = new TS_Oscillator(numOscillatorOutputs);
	initialOscillators();
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up or ram.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiOscillator::~multiOscillator()
{
	isInitialized = false;
	if (oscillators != NULL)
		delete[] oscillators;
	oscillators = NULL;
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Initialize values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiOscillator::reset()
{
	for (int i = 0; i < numberOscillators; i++)
	{
		oscillators[i].initialize();
	}
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// toJson(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t *multiOscillator::toJson()
{
	json_t* rootJ = json_object();
	// version
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));
	json_object_set_new(rootJ, "numOsc", json_integer(numberOscillators));
	json_object_set_new(rootJ, "numOutputs", json_integer(numOscillatorOutputs));

	json_t* oscillatorsJ = json_array();
	for (int i = 0; i < numberOscillators; i++)
	{
		// Input
		json_array_append_new(oscillatorsJ, oscillators[i].serialize());
	}
	json_object_set_new(rootJ, "oscillators", oscillatorsJ);
	return rootJ;
} // end toJson()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// fromJson(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void multiOscillator::fromJson(json_t *rootJ)
{
	json_t* currJ = NULL;
	int nOscillators = numberOscillators;
	currJ = json_object_get(rootJ, "numOsc");
	if (currJ)
	{
		nOscillators = json_integer_value(currJ);
		if (nOscillators > numberOscillators)
			nOscillators = numberOscillators;
	}
	currJ = json_object_get(rootJ, "numOutputs");
	if (currJ)
	{
		numOscillatorOutputs = json_integer_value(currJ);
	}

	json_t* oscillatorsJ = json_object_get(rootJ, "oscillators");
	for (int i = 0; i < nOscillators; i++)
	{
		currJ = json_array_get(oscillatorsJ, i);
		if (currJ)
		{
			oscillators[i].deserialize(currJ);
		}
	} // end loop through osccilators
	return;
} // end fromJson()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
// Process
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiOscillator::step()
{
	float dt = engineGetSampleTime();

	// Get Oscillator CV and User Inputs
	for (int osc = 0; osc < numberOscillators; osc++)
	{
		bool sync = false;
		bool oscillatorReset = false; // If this oscillator is at 0 phase.
		TS_Oscillator* theOscillator = &(oscillators[osc]);
		int baseInputId = InputIds::OSC_INPUT_START + osc * (TS_Oscillator::BaseInputIds::OSCWF_NUM_INPUTS + numOscillatorOutputs * TS_OscillatorOutput::BaseInputIds::OUT_NUM_INPUTS);
		int baseParamId = ParamIds::OSC_PARAM_START + osc * (TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS + numOscillatorOutputs * TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS);
		int baseOutputId = OutputIds::OSC_OUTPUT_START + osc * (TS_Oscillator::BaseOutputIds::OSCWF_NUM_OUTPUTS + numOscillatorOutputs * TS_OscillatorOutput::BaseOutputIds::OUT_NUM_OUTPUTS);
		int baseLightId = LightIds::OSC_LIGHT_START + osc * (TS_Oscillator::BaseLightIds::OSCWF_NUM_LIGHTS + numOscillatorOutputs * TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS);

		//------------------------------
		// Sync this oscillator
		//------------------------------
		if (lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value > 0)
			lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value -= lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value / lightLambda * dt;
		else if (lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value < 0)
			lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value = 0;
		if (theOscillator->synchTrigger.process(params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_SYNC_PARAM].value + inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_SYNC_INPUT].value))
		{
			lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value = 1.0f;
			sync = true;
		} // end if

		//------------------------------
		// Values In (Add Input + Knob)
		//------------------------------
		// *> Amplitude (V):
		theOscillator->ui_amplitude_V = params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_AMPLITUDE_PARAM].value;
		float a = theOscillator->ui_amplitude_V;
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_AMPLITUDE_INPUT].active)
			a += inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_AMPLITUDE_INPUT].value;
		theOscillator->amplitude_V = a;

		// *> Frequency (Hz):
		// User Knob:
#if TROWA_MOSC_FREQ_KNOB_NEEDS_CONVERSION
		theOscillator->ui_frequency_Hz = rescale(params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_FREQUENCY_PARAM].value, TROWA_MOSC_F_KNOB_MIN_V, TROWA_MOSC_F_KNOB_MAX_V, MOSC_FREQ_MIN_HZ, MOSC_FREQ_MAX_HZ);
#else
		theOscillator->ui_frequency_Hz = params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_FREQUENCY_PARAM].value;
#endif
		float f = theOscillator->ui_frequency_Hz;
		// Add Input:
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_FREQUENCY_INPUT].active) {
			// CV frequency
			f = clamp(f + VoltageToFrequency(inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_FREQUENCY_INPUT].value), MOSC_FREQ_MIN_HZ, MOSC_FREQ_MAX_HZ);
		}
		theOscillator->frequency_Hz = f; // Actual Frequency

		// *> Phase Shift (deg): 
		theOscillator->ui_phaseShift_deg = rescale(params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_PHASE_SHIFT_PARAM].value, TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
		float phi = theOscillator->ui_phaseShift_deg;
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_PHASE_SHIFT_INPUT].active) {
			phi += rescale(inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_PHASE_SHIFT_INPUT].value, TROWA_MOSC_INPUT_MIN_V, TROWA_MOSC_INPUT_MAX_V, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
		}
		theOscillator->setPhaseShift_deg(phi);

		// *> Offset (V):
		theOscillator->ui_offset_V = params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_OFFSET_PARAM].value;
		float c = theOscillator->ui_offset_V;
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_OFFSET_INPUT].active)
			c += inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_OFFSET_INPUT].value;
		theOscillator->offset_V = c;

		//------------------------------
		// Calculate phase
		//------------------------------
		oscillatorReset = theOscillator->calculatePhase(dt, sync);

		//------------------------------
		// Sync Output
		//------------------------------
		if (oscillatorReset)
		{
			theOscillator->synchPulse.trigger(1e-4); // 1e-3
		}
		outputs[baseOutputId + TS_Oscillator::BaseOutputIds::OSCWF_SYNC_OUTPUT].value = (theOscillator->synchPulse.process(dt)) ? 10.0f : 0.0f;

		//------------------------------
		// Each output channel
		//------------------------------
		baseParamId += TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS;
		baseInputId += TS_Oscillator::BaseInputIds::OSCWF_NUM_INPUTS;
		baseOutputId += TS_Oscillator::BaseOutputIds::OSCWF_NUM_OUTPUTS;
		baseLightId += TS_Oscillator::BaseLightIds::OSCWF_NUM_LIGHTS;
		for (int i = 0; i < theOscillator->numOutputWaveForms; i++)
		{
			float type = clamp((int)rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_OSC_TYPE_PARAM].value, TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, 0, WaveFormType::NUM_WAVEFORMS), 0, WaveFormType::NUM_WAVEFORMS - 1);
			theOscillator->outputWaveforms[i].ui_waveFormType = static_cast<WaveFormType>(int(type));
			if (inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_OSC_TYPE_INPUT].active) {
				type = clamp((int)rescale(inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_OSC_TYPE_INPUT].value,
					TROWA_MOSC_TYPE_INPUT_MIN_V, TROWA_MOSC_TYPE_INPUT_MAX_V, 0, WaveFormType::NUM_WAVEFORMS), 0, WaveFormType::NUM_WAVEFORMS - 1);
			}
			theOscillator->outputWaveforms[i].waveFormType = static_cast<WaveFormType>(int(type));

			// *> Phase shift for this output
			float phi = rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_PHASE_SHIFT_PARAM].value,
				TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V,
				MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
			theOscillator->outputWaveforms[i].ui_phaseShift_deg = phi;
			if (inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_PHASE_SHIFT_INPUT].active) {
				phi += rescale(inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_PHASE_SHIFT_INPUT].value,
					TROWA_MOSC_INPUT_MIN_V, TROWA_MOSC_INPUT_MAX_V, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
			}
			theOscillator->outputWaveforms[i].setPhaseShift_deg(phi);

			// *> Aux parameter: (Currently only rect/square and saw/ramp)
			float aux = 0.5f;
			//switch (theOscillator->outputWaveforms[i].waveFormType)
			//{
			//case WaveFormType::WAVEFORM_SQR:
			//case WaveFormType::WAVEFORM_SAW:
			//	aux = rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM].value, TROWA_MOSC_AUX_MIN_V, TROWA_MOSC_AUX_MAX_V, 0.0f, 1.0f);
			//	break;
			//default:
			//	break;
			//}
			if (inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AUX_INPUT].active) {
				aux = clamp(rescale(inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AUX_INPUT].value, TROWA_MOSC_AUX_MIN_V, TROWA_MOSC_AUX_MAX_V, 0.f, 1.f), 0.f, 1.f);
			}
			else
			{
				aux = rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM].value, TROWA_MOSC_KNOB_AUX_MIN_V, TROWA_MOSC_KNOB_AUX_MAX_V, 0.0f, 1.0f);
			}
			theOscillator->outputWaveforms[i].auxParam_norm = aux;

			// *> AM Type (digital = false, ring = true)
			if (theOscillator->outputWaveforms[i].amRingModulationTrigger.process(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_TYPE_PARAM].value))
			{
				theOscillator->outputWaveforms[i].amRingModulation = !theOscillator->outputWaveforms[i].amRingModulation;
				//info("[Ch %d] AM Button Click id %d. Ring Mod = %d.", i + 1, baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_TYPE_PARAM, theOscillator->outputWaveforms[i].amRingModulation);
			}
			lights[baseLightId + TS_OscillatorOutput::BaseLightIds::OUT_AM_MODE_LED].value = (theOscillator->outputWaveforms[i].amRingModulation) ? 1.0f : 0.0f;

			//------------------------------------------
			// Calculate an output/outputs
			//------------------------------------------
			if (outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_RAW_SIGNAL].active ||
				(outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL].active && inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AM_INPUT].active))
			{
				// Calculate the RAW output:
				float rawOutput = 0.0f;
				float phi_n = theOscillator->outputWaveforms[i].phaseShift_norm;
				switch (theOscillator->outputWaveforms[i].waveFormType)
				{
				case WaveFormType::WAVEFORM_SIN:
					rawOutput = theOscillator->calcSin(phi_n);
					break;
				case WaveFormType::WAVEFORM_TRI:
					rawOutput = theOscillator->calcTri(phi_n);
					break;
				case WaveFormType::WAVEFORM_SQR:
					rawOutput = theOscillator->calcRect(phi_n, theOscillator->outputWaveforms[i].auxParam_norm);
					break;
				case WaveFormType::WAVEFORM_SAW:
					rawOutput = theOscillator->calcSaw(phi_n, theOscillator->outputWaveforms[i].getRampSlope());
					break;
				default:
					break;
				}
				outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_RAW_SIGNAL].value = rawOutput;

				// Calculate AM output:
				if (outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL].active)
				{
					float modOutput = 0.f;
					float modulator = inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AM_INPUT].value;
					float modWeight = params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_MIX_PARAM].value; //rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_MIX_PARAM].value, TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, 0.f, 1.f)
					if (theOscillator->outputWaveforms[i].amRingModulation)
					{
						// Ring modulation
						modOutput = theOscillator->ringModulator.ringMod(modulator, rawOutput);
					}
					else
					{
						// Digital modulation (just mulitply)
						modOutput = modulator * rawOutput;
					}
					outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL].value = modWeight * modOutput + (1.0f - modWeight)*rawOutput;
				} // end calculate the multiplied signal
			} // end calculate the raw signal

			baseParamId += TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS;
			baseInputId += TS_OscillatorOutput::BaseInputIds::OUT_NUM_INPUTS;
			baseOutputId += TS_OscillatorOutput::BaseOutputIds::OUT_NUM_OUTPUTS;
			baseLightId += TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS;
		} // end loop through output signals/channels
	} // end loop through oscillators
	return;
} // end step()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//::::::::::::: TS_OscillatorOutput :::::::::::::::::::::::::::::
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

//--------------------------------------------------------
// initialize()
//--------------------------------------------------------
void TS_OscillatorOutput::initialize()
{
	phaseShift_deg = 0;
	phaseShift_norm = 0;
	amRingModulation = false;
	auxParam_norm = 0.5f;
	return;
}
//--------------------------------------------------------
// setPhaseShift_deg()
// @deg : (IN) The phase shift in degrees.
//--------------------------------------------------------
void TS_OscillatorOutput::setPhaseShift_deg(float deg)
{
	phaseShift_deg = deg;
	phaseShift_norm = deg / 360.0f;
	return;
}
//--------------------------------------------------------
// serialize()
// @returns : The TS_Oscillator json node.
//--------------------------------------------------------
json_t* TS_OscillatorOutput::serialize()
{
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "wavetype", json_integer(waveFormType));
	json_object_set_new(rootJ, "phaseShift_deg", json_real(ui_phaseShift_deg));
	json_object_set_new(rootJ, "auxParam_norm", json_real(auxParam_norm));
	json_object_set_new(rootJ, "amRingMod", json_integer(amRingModulation));
	return rootJ;
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The TS_Oscillator json node.
//--------------------------------------------------------
void TS_OscillatorOutput::deserialize(json_t* rootJ)
{
	json_t* currJ = NULL;
	if (rootJ) {
		currJ = json_object_get(rootJ, "wavetype");
		if (currJ)
			waveFormType = static_cast<WaveFormType>( json_integer_value(currJ) );
		currJ = json_object_get(rootJ, "phaseShift_deg");
		if (currJ)
			ui_phaseShift_deg = json_real_value(currJ);
		currJ = json_object_get(rootJ, "auxParam_norm");
		if (currJ)
			auxParam_norm = json_real_value(currJ);
		currJ = json_object_get(rootJ, "amRingMod");
		if (currJ)
			amRingModulation = json_integer_value(currJ) > 0;
	}
} // end deserialize()


// Model for trowa multiOscillator
RACK_PLUGIN_MODEL_INIT(trowaSoft, MultiOscillator) {
   Model* modelMultiOscillator = Model::create<multiOscillator, multiOscillatorWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "multiWave", /*name*/ "multiWave", /*Tags*/ OSCILLATOR_TAG, RING_MODULATOR_TAG);
   return modelMultiOscillator;
}
