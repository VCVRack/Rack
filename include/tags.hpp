#pragma once
#include <string>


namespace rack {


/** Describes the type(s) of each module
To see comments, turn word wrap on. I'm using inline comments so I can automatically sort the list when more tags are added.
*/
enum ModelTag {
	NO_TAG, // Don't use this in `Model::create(...)`. Instead, just omit the tags entirely.
	AMPLIFIER_TAG,
	ARPEGGIATOR_TAG,
	ATTENUATOR_TAG,
	BLANK_TAG,
	CHORUS_TAG,
	CLOCK_MODULATOR_TAG, // Clock dividers, multipliers, etc.
	CLOCK_TAG,
	COMPRESSOR_TAG,
	CONTROLLER_TAG, // Use only if the artist "performs" with this module. Knobs are not sufficient. Examples: on-screen keyboard, XY pad.
	DELAY_TAG,
	DIGITAL_TAG,
	DISTORTION_TAG,
	DRUM_TAG,
	DUAL_TAG, // The core functionality times two. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Dual module.
	DYNAMICS_TAG,
	EFFECT_TAG,
	ENVELOPE_FOLLOWER_TAG,
	ENVELOPE_GENERATOR_TAG,
	EQUALIZER_TAG,
	EXTERNAL_TAG,
	FILTER_TAG,
	FLANGER_TAG,
	FUNCTION_GENERATOR_TAG,
	GRANULAR_TAG,
	LFO_TAG,
	LIMITER_TAG,
	LOGIC_TAG,
	LOW_PASS_GATE_TAG,
	MIDI_TAG,
	MIXER_TAG,
	MULTIPLE_TAG,
	NOISE_TAG,
	OSCILLATOR_TAG,
	PANNING_TAG,
	PHASER_TAG,
	PHYSICAL_MODELING_TAG,
	QUAD_TAG, // The core functionality times four. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Quad module.
	QUANTIZER_TAG,
	RANDOM_TAG,
	RECORDING_TAG,
	REVERB_TAG,
	RING_MODULATOR_TAG,
	SAMPLE_AND_HOLD_TAG,
	SAMPLER_TAG,
	SEQUENCER_TAG,
	SLEW_LIMITER_TAG,
	SWITCH_TAG,
	SYNTH_VOICE_TAG, // A synth voice must have an envelope built-in.
	TUNER_TAG,
	UTILITY_TAG, // Serves only extremely basic functions, like inverting, max, min, multiplying by 2, etc.
	VISUAL_TAG,
	VOCODER_TAG,
	WAVESHAPER_TAG,
	NUM_TAGS
};


void tagsInit();

extern std::string gTagNames[NUM_TAGS];


} // namespace rack
