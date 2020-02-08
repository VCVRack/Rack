#include <map>
#include <tag.hpp>
#include <string.hpp>


namespace rack {
namespace tag {


/** See https://vcvrack.com/manual/Manifest#modules-tags for documentation of tags. */
const std::vector<std::vector<std::string>> tagAliases = {
	{"Arpeggiator"},
	{"Attenuator"},
	{"Blank"},
	{"Chorus"},
	{"Clock generator", "Clock"},
	{"Clock modulator"},
	{"Compressor"},
	{"Controller"},
	{"Delay"},
	{"Digital"},
	{"Distortion"},
	{"Drum", "Drums", "Percussion"},
	{"Dual"},
	{"Dynamics"},
	{"Effect"},
	{"Envelope follower"},
	{"Envelope generator"},
	{"Equalizer", "EQ"},
	{"Expander"},
	{"External"},
	{"Filter", "VCF", "Voltage controlled filter"},
	{"Flanger"},
	{"Function generator"},
	{"Granular"},
	{"Hardware clone", "Hardware"},
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
	{"Speech"},
	{"Switch"},
	{"Synth voice"},
	{"Tuner"},
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


std::string getTag(int tagId) {
	assert(0 <= tagId && tagId < (int) tagAliases.size());
	return tagAliases[tagId][0];
}


} // namespace tag
} // namespace rack
