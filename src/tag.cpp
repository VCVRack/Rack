#include <tag.hpp>
#include <string.hpp>
#include <map>


namespace rack {
namespace tag {


const std::vector<std::vector<std::string>> tagAliases = {
	{"Arpeggiator"}, // With a level knob and not much else.
	{"Attenuator"}, // No parameters or ports. Serves no purpose except visual.
	{"Blank"},
	{"Chorus"},
	{"Clock generator", "Clock"}, // Clock dividers, multipliers, etc.
	{"Clock modulator"}, // With threshold, ratio, knee, etc parameters.
	{"Compressor"}, // Use only if the artist "performs" with this module. Simply having knobs is not enough. Examples: on-screen keyboard, XY pad.
	{"Controller"},
	{"Delay"},
	{"Digital"},
	{"Distortion"},
	{"Drum", "Drums", "Percussion"}, // The core functionality times two. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Dual module.
	{"Dual"},
	{"Dynamics"},
	{"Effect"},
	{"Envelope follower"},
	{"Envelope generator"},
	{"Equalizer", "EQ"}, // Expands the functionality of a "mother" module when placed next to it. Expanders should inherit the tags of its mother module.
	{"Expander"},
	{"External"},
	{"Filter", "VCF", "Voltage controlled filter"},
	{"Flanger"},
	{"Function generator"},
	{"Granular"},
	{"Hardware clone", "Hardware"}, // Clones the functionality *and* appearance of a real-world hardware module.
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
	{"Polyphonic", "Poly"}, // The core functionality times four. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Quad module.
	{"Quad"},
	{"Quantizer"},
	{"Random"},
	{"Recording"},
	{"Reverb"},
	{"Ring modulator"},
	{"Sample and hold", "S&H", "Sample & hold"},
	{"Sampler"},
	{"Sequencer"},
	{"Slew limiter"},
	{"Switch"}, // A synth voice must have, at the minimum, a built-in oscillator and envelope.
	{"Synth voice"},
	{"Tuner"}, // Serves only extremely basic functions, like inverting, max, min, multiplying by 2, etc.
	{"Utility"},
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
