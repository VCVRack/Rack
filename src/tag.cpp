#include <tag.hpp>
#include <string.hpp>

#include <map>


namespace rack {
namespace tag {


/** List of allowed tags in human display form, alphabetized.
All tags here should be in sentence caps for display consistency.
However, tags are case-insensitive in plugin metadata.
*/
const std::set<std::string> allowedTags = {
	"Arpeggiator",
	"Attenuator", // With a level knob and not much else.
	"Blank", // No parameters or ports. Serves no purpose except visual.
	"Chorus",
	"Clock generator",
	"Clock modulator", // Clock dividers, multipliers, etc.
	"Compressor", // With threshold, ratio, knee, etc parameters.
	"Controller", // Use only if the artist "performs" with this module. Simply having knobs is not enough. Examples: on-screen keyboard, XY pad.
	"Delay",
	"Digital",
	"Distortion",
	"Drum",
	"Dual", // The core functionality times two. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Dual module.
	"Dynamics",
	"Effect",
	"Envelope follower",
	"Envelope generator",
	"Equalizer",
	"Expander", // Expands the functionality of a "mother" module when placed next to it. Expanders should inherit the tags of its mother module.
	"External",
	"Filter",
	"Flanger",
	"Function generator",
	"Granular",
	"Limiter",
	"Logic",
	"Low-frequency oscillator",
	"Low-pass gate",
	"MIDI",
	"Mixer",
	"Multiple",
	"Noise",
	"Oscillator",
	"Panning",
	"Phaser",
	"Physical modeling",
	"Polyphonic",
	"Quad", // The core functionality times four. If multiple channels are a requirement for the module to exist (ring modulator, mixer, etc), it is not a Quad module.
	"Quantizer",
	"Random",
	"Recording",
	"Reverb",
	"Ring modulator",
	"Sample and hold",
	"Sampler",
	"Sequencer",
	"Slew limiter",
	"Switch",
	"Synth voice", // A synth voice must have, at the minimum, a built-in oscillator and envelope.
	"Tuner",
	"Utility", // Serves only extremely basic functions, like inverting, max, min, multiplying by 2, etc.
	"Visual",
	"Vocoder",
	"Voltage-controlled amplifier",
	"Waveshaper",
};


/** List of common synonyms for allowed tags.
Aliases and tags must be lowercase.
*/
const std::map<std::string, std::string> tagAliases = {
	{"amplifier", "voltage-controlled amplifier"},
	{"clock", "clock generator"},
	{"drums", "drum"},
	{"eq", "equalizer"},
	{"lfo", "low-frequency oscillator"},
	{"low frequency oscillator", "low-frequency oscillator"},
	{"low pass gate", "low-pass gate"},
	{"lowpass gate", "low-pass gate"},
	{"percussion", "drum"},
	{"poly", "polyphonic"},
	{"s&h", "sample and hold"},
	{"sample & hold", "sample and hold"},
	{"vca", "voltage-controlled amplifier"},
	{"vcf", "filter"},
	{"vco", "oscillator"},
	{"voltage controlled amplifier", "voltage-controlled amplifier"},
	{"voltage controlled filter", "filter"},
	{"voltage controlled oscillator", "oscillator"},
};


std::string normalize(const std::string& tag) {
	std::string lowercaseTag = string::lowercase(tag);
	// Transform aliases
	auto it = tagAliases.find(lowercaseTag);
	if (it != tagAliases.end())
		lowercaseTag = it->second;
	// Find allowed tag
	for (const std::string& allowedTag : allowedTags) {
		if (lowercaseTag == string::lowercase(allowedTag))
			return allowedTag;
	}
	return "";
}


} // namespace tag
} // namespace rack
