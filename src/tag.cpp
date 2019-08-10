#include <tag.hpp>
#include <string.hpp>
#include <map>


namespace rack {
namespace tag {


const std::vector<std::vector<std::string>> tagAliases = {
	{"Arpeggiator"},
	{"Attenuator"}, // With a level knob and not much else.
	{"Blank"}, // No parameters or ports. Serves no purpose except visual.
	{"Chorus"},
	{"Clock generator", "Clock"},
	{"Clock modulator"}, // Clock dividers, multipliers, etc.
	{"Compressor"}, // With threshold, ratio, knee, etc parameters.
	{"Controller"}, // Use only if the artist "performs" with this module. Simply having knobs is not enough. Examples: on-screen keyboard, XY pad.
	{"Delay"},
	{"Digital"},
	{"Distortion"},
	{"Drum", "Drums", "Percussion"},
	{"Dual"}, // The core functionality times two. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Dual module.
	{"Dynamics"},
	{"Effect"},
	{"Envelope follower"},
	{"Envelope generator"},
	{"Equalizer", "EQ"},
	{"Expander"}, // Expands the functionality of a "mother" module when placed next to it. Expanders should inherit the tags of its mother module.
	{"External"},
	{"Filter", "VCF", "Voltage controlled filter"},
	{"Flanger"},
	{"Function generator"},
	{"Granular"},
	{"Limiter"},
	{"Logic"},
	{"Low-frequency oscillator", "LFO", "Low frequency oscillator"},
	{"Low-pass gate", "Low pass gate", "Lowpass gate"},
	{"MIDI"},
	{"Mixer"},
	{"Multiple"},
	{"Noise"},
	{"Oscillator", "VCO", "Voltage controlled oscillator"},
	{"Panning", "Pan"},
	{"Phaser"},
	{"Physical modeling"},
	{"Polyphonic", "Poly"},
	{"Quad"}, // The core functionality times four. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Quad module.
	{"Quantizer"},
	{"Random"},
	{"Recording"},
	{"Reverb"},
	{"Ring modulator"},
	{"Sample and hold", "S&H", "Sample & hold"},
	{"Sampler"},
	{"Sequencer"},
	{"Slew limiter"},
	{"Switch"},
	{"Synth voice"}, // A synth voice must have, at the minimum, a built-in oscillator and envelope.
	{"Tuner"},
	{"Utility"}, // Serves only extremely basic functions, like inverting, max, min, multiplying by 2, etc.
	{"Visual"},
	{"Vocoder"},
	{"Voltage-controlled amplifier", "Amplifier", "VCA", "Voltage controlled amplifier"},
	{"Waveshaper"},
};


int findId(const std::string& tag) {
	std::string lowercaseTag = string::lowercase(tag);
	for (int tagId = 0; tagId < (int) tagAliases.size(); tagId++) {
		for (const std::string& alias : tagAliases[tagId]) {
			if (string::lowercase(alias) == lowercaseTag)
				return tagId;
		}
	}
	return -1;
}


} // namespace tag
} // namespace rack
