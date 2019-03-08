#include "rack.hpp"
using namespace rack;

namespace rack_plugin_Alikins {

// TODO: mv to header
// float A440_VOLTAGE = 4.75f;
// https://vcvrack.com/manual/VoltageStandards.html#pitch-and-frequencies
// 0v = C4 = 261.626f
float VCO_BASELINE_VOLTAGE = 0.0f;
float VCO_BASELINE_FREQ = 261.626f;
float VCO_BASELINE_NOTE_OCTAVE_OFFSET = 4.0f;

//float VCO_BASELINE_VOLTAGE = 4.75f;
//float VCO_BASELINE_FREQ = 440.0f;


int A440_MIDI_NUMBER = 69;

// bogaudio lfo, defaults to 0v == 16.35f
// float LFO_BASELINE_FREQ = 16.35f;

// https://vcvrack.com/manual/VoltageStandards.html#pitch-and-frequencies
float LFO_BASELINE_VOLTAGE = 0.0f;
float LFO_BASELINE_FREQ = 2.0f;


// FIXME: can/should be inline
// FIXME: likely should be a NoteInfo type/struct/object
// These are assuming
//   440.0 hz == A4 == 0.75v  (A440)
//   261.626f == C4 == 0v
float freq_to_cv(float freq) {
    float volts = log2f(freq / VCO_BASELINE_FREQ * powf(2.0f, VCO_BASELINE_VOLTAGE));
    // debug("freq_to_vc freq=%f -> vc volts=%f (vco_baseline_freq=%f)",
    //       freq, volts, VCO_BASELINE_VOLTAGE);
    return volts;
}

float lfo_freq_to_cv(float lfo_freq) {
    float volts = log2f(lfo_freq / LFO_BASELINE_FREQ * powf(2.0f, LFO_BASELINE_VOLTAGE));
    // debug("lfo_freq_to_cv: lfo_freq=%f volts=%f LFO_BASELINE_VOLTAGE=%f",
    //      lfo_freq, volts, LFO_BASELINE_VOLTAGE);
    return volts;
}

float cv_to_freq(float volts) {
    float freq = VCO_BASELINE_FREQ / powf(2.0f, VCO_BASELINE_VOLTAGE) * powf(2.0f, volts);
    // debug("cv_to_freq: cv volts=%f -> freq=%f (vco_baseline_freq=%f)",
    //      volts, freq, VCO_BASELINE_FREQ);
    return freq;
}

float lfo_cv_to_freq(float volts) {
    // TODO: figure out what a common LFO baseline is
    float freq = LFO_BASELINE_FREQ / powf(2.0f, LFO_BASELINE_VOLTAGE) * powf(2.0f, volts);
    // debug("lfo_cv_to_freq freq=%f volts=%f ", freq, volts);
    return freq;
}

// can return negative
double volts_of_nearest_note(float volts) {
    double res = round( (volts * 12.0) )  / 12.0;
    return res;
}

int volts_to_note(float volts) {
    int res = abs(static_cast<int>( roundf( ( volts * 12.0f) ) ) ) % 12;
    // FIXME: ugly, sure there is a more elegant way to do this
    if (volts < 0.0f && res > 0) {
        res = 12 - res;
    }

    // debug("volts_to_note: volts=%f res=%d", volts, res);
    return res;
}

int volts_to_octave(float volts) {
    int octave = floor(volts) + VCO_BASELINE_NOTE_OCTAVE_OFFSET;
    //debug("volts_to_octaves: volts=%f -> octave=%d (offset_from_baseline=%f, v+ofb=%f)",
    //      volts, octave, offset_from_baseline, volts+offset_from_baseline);
    return octave;
}

float volts_to_note_cents(float volts) {
    float nearest_note = volts_of_nearest_note(volts);
    double cent_volt = 1.0f / 12.0f / 100.0f;

    float offset_cents = (volts - nearest_note)/cent_volt;
    // debug("volts: %f volts_of_nearest: %f volts-volts_nearest: %f offset_cents %f",
    //     volts, nearest_note, volts-nearest_note, offset_cents);

    return offset_cents;
}

int volts_to_midi(float volts) {
    int midi_note = floor(volts * 12.0f) + 21;
    return midi_note;
}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;
