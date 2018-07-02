#pragma once

namespace bogaudio {
namespace dsp {

const float referenceFrequency = 261.626; // C4; frequency at which Rack 1v/octave CVs are zero.
const float referenceSemitone = 60.0; // C4; value of C4 in semitones is arbitrary here, so have it match midi note numbers when rounded to integer.
const float twelfthRootTwo = 1.0594630943592953;
const float logTwelfthRootTwo = logf(1.0594630943592953);

inline float frequencyToSemitone(float frequency) {
	return logf(frequency / referenceFrequency) / logTwelfthRootTwo + referenceSemitone;
}

inline float semitoneToFrequency(float semitone) {
	return powf(twelfthRootTwo, semitone - referenceSemitone) * referenceFrequency;
}

inline float frequencyToCV(float frequency) {
	return log2f(frequency / referenceFrequency);
}

inline float cvToFrequency(float cv) {
	return powf(2.0, cv) * referenceFrequency;
}

inline float cvToSemitone(float cv) {
	return frequencyToSemitone(cvToFrequency(cv));
}

inline float semitoneToCV(float semitone) {
	return frequencyToCV(semitoneToFrequency(semitone));
}

} // namespace dsp
} // namespace bogaudio
