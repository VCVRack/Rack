#include "tags.hpp"


namespace rack {


std::string gTagNames[NUM_TAGS];


void tagsInit() {
	gTagNames[AMPLIFIER_TAG] = "Amplifier/VCA";
	gTagNames[ATTENUATOR_TAG] = "Attenuator";
	gTagNames[ARPEGGIATOR_TAG] = "Arpeggiator";
	gTagNames[BLANK_TAG] = "Blank";
	gTagNames[CHORUS_TAG] = "Chorus";
	gTagNames[CLOCK_TAG] = "Clock";
	gTagNames[CLOCK_MODULATOR_TAG] = "Clock Modulator";
	gTagNames[COMPRESSOR_TAG] = "Compressor";
	gTagNames[CONTROLLER_TAG] = "Controller";
	gTagNames[DELAY_TAG] = "Delay";
	gTagNames[DIGITAL_TAG] = "Digital";
	gTagNames[DISTORTION_TAG] = "Distortion";
	gTagNames[DRUM_TAG] = "Drum";
	gTagNames[DUAL_TAG] = "Dual/Stereo";
	gTagNames[DYNAMICS_TAG] = "Dynamics";
	gTagNames[EFFECT_TAG] = "Effect";
	gTagNames[ENVELOPE_FOLLOWER_TAG] = "Envelope Follower";
	gTagNames[ENVELOPE_GENERATOR_TAG] = "Envelope Generator";
	gTagNames[EQUALIZER_TAG] = "Equalizer";
	gTagNames[EXTERNAL_TAG] = "External";
	gTagNames[FILTER_TAG] = "Filter/VCF";
	gTagNames[FLANGER_TAG] = "Flanger";
	gTagNames[FUNCTION_GENERATOR_TAG] = "Function Generator";
	gTagNames[GRANULAR_TAG] = "Granular";
	gTagNames[LFO_TAG] = "LFO";
	gTagNames[LIMITER_TAG] = "Limiter";
	gTagNames[LOGIC_TAG] = "Logic";
	gTagNames[LOW_PASS_GATE_TAG] = "Low Pass Gate";
	gTagNames[MIDI_TAG] = "MIDI";
	gTagNames[MIXER_TAG] = "Mixer";
	gTagNames[MULTIPLE_TAG] = "Multiple";
	gTagNames[NOISE_TAG] = "Noise";
	gTagNames[OSCILLATOR_TAG] = "Oscillator/VCO";
	gTagNames[PANNING_TAG] = "Panning";
	gTagNames[PHASER_TAG] = "Phaser";
	gTagNames[PHYSICAL_MODELING_TAG] = "Physical Modeling";
	gTagNames[QUAD_TAG] = "Quad";
	gTagNames[QUANTIZER_TAG] = "Quantizer";
	gTagNames[RANDOM_TAG] = "Random";
	gTagNames[RECORDING_TAG] = "Recording";
	gTagNames[REVERB_TAG] = "Reverb";
	gTagNames[RING_MODULATOR_TAG] = "Ring Modulator";
	gTagNames[SAMPLE_AND_HOLD_TAG] = "Sample and Hold";
	gTagNames[SAMPLER_TAG] = "Sampler";
	gTagNames[SEQUENCER_TAG] = "Sequencer";
	gTagNames[SLEW_LIMITER_TAG] = "Slew Limiter";
	gTagNames[SWITCH_TAG] = "Switch";
	gTagNames[SYNTH_VOICE_TAG] = "Synth Voice";
	gTagNames[TUNER_TAG] = "Tuner";
	gTagNames[UTILITY_TAG] = "Utility";
	gTagNames[VISUAL_TAG] = "Visual";
	gTagNames[VOCODER_TAG] = "Vocoder";
	gTagNames[WAVESHAPER_TAG] = "Waveshaper";
}


} // namespace rack
