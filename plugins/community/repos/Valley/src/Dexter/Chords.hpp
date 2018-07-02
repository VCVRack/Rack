#ifndef DSJ_AMN_VALLEY_CHORDS_HPP
#define DSJ_AMN_VALLEY_CHORDS_HPP
#include <string>
#include <vector>

const int kMaxChordSize = 8;
const int kChordSize = 8;
const float kSemi = 0.083333333333;
const float kCent = kSemi / 100.0;
const float kOct = 1.0;
const float kThird = kSemi * 4.0;
const float kFourth = kSemi * 5.0;
const float kFifth = kSemi * 7.0;

enum Chords {
    CHORD_ONE_NOTE,
    CHORD_MINOR_2ND,
    CHORD_MAJOR_2ND,
    CHORD_MINOR_3RD,
    CHORD_MAJOR_3RD,
    CHORD_4TH,
    CHORD_TRITONE,
    CHORD_5TH,
    CHORD_AUG_5TH,
    CHORD_6TH,
    CHORD_MINOR_7TH_INT,
    CHORD_MAJOR_7TH_INT,
    CHORD_OCTAVE,
    CHORD_SUB_OCTAVE,
    CHORD_OCT_2OCT,
    CHORD_MINOR_TRIAD,
    CHORD_MAJOR_TRIAD,
    CHORD_SUS_TRIAD,
    CHORD_AUG_TRIAD,
    CHORD_DIM_TRIAD,
    CHORD_MAJOR_6TH,
    CHORD_MAJOR_7TH,
    CHORD_DOM_7TH,
    CHORD_MINOR_7TH,
    CHORD_HALFDIM_7TH,
    CHORD_DIM_7TH,
    CHORD_SUS_7TH,
    CHORD_DOM_9TH,
    CHORD_DOM_MIN_9TH,
    CHORD_MAJOR_9TH,
    CHORD_MINOR_9TH,
    CHORD_MAJOR_6_9,
    CHORD_MINOR_6_9,
    CHORD_9TH_FLAT_5TH,
    CHORD_9TH_SHARP_5TH,
    CHORD_DOM_11TH,
    CHORD_MINOR_11TH,
    CHORD_UNISON4,
    CHORD_UNISON8,
    NUM_CHORDS
};

static float chord_oneNote[1]     = {0};                    static int chord_oneNoteSize = 1;
static float chord_minor2nd[2]    = {0, 1};                 static int chord_minor2ndSize = 2;
static float chord_major2nd[2]    = {0, 2};                 static int chord_major2ndSize = 2;
static float chord_minor3rd[2]    = {0, 3};                 static int chord_minor3rdSize = 2;
static float chord_major3rd[2]    = {0, 4};                 static int chord_major3rdSize = 2;
static float chord_4th[2]         = {0, 5};                 static int chord_4thSize = 2;
static float chord_tritone[2]     = {0, 6};                 static int chord_tritoneSize = 2;
static float chord_5th[2]         = {0, 7};                 static int chord_5thSize = 2;
static float chord_aug5th[2]      = {0, 8};                 static int chord_aug5thSize = 2;
static float chord_6th[2]         = {0, 9};                 static int chord_6thSize = 2;
static float chord_minor7thInt[2] = {0, 10};                static int chord_minor7thIntSize = 2;
static float chord_major7thInt[2] = {0, 11};                static int chord_major7thIntSize = 2;
static float chord_octave[2]      = {0, 12};                static int chord_octaveSize = 2;
static float chord_subOctave[2]   = {0, -12};               static int chord_subOctaveSize = 2;
static float chord_oct2Octave[3]  = {0, 12, 24};            static int chord_oct2OctaveSize = 3;
static float chord_minorTriad[3]  = {0, 3, 7};              static int chord_minorTriadSize = 3;
static float chord_majorTriad[3]  = {0, 4, 7};              static int chord_majorTriadSize = 3;
static float chord_susTriad[3]    = {0, 5, 7};              static int chord_susTriadSize = 3;
static float chord_augTriad[3]    = {0, 4, 8};              static int chord_augTriadSize = 3;
static float chord_dimTriad[3]    = {0, 3, 6};              static int chord_dimTriadSize = 3;
static float chord_major6th[4]    = {0, 4, 7, 9};           static int chord_major6thSize = 4;
static float chord_major7th[4]    = {0, 4, 7, 11};          static int chord_major7thSize = 4;
static float chord_dom7th[4]      = {0, 4, 7, 10};          static int chord_dom7thSize = 4;
static float chord_minor7th[4]    = {0, 3, 7, 10};          static int chord_minor7thSize = 4;
static float chord_halfDim7th[4]  = {0, 3, 6, 10};          static int chord_halfDim7thSize = 4;
static float chord_dim7th[4]      = {0, 3, 6, 9};           static int chord_dim7thSize = 4;
static float chord_sus7th[4]      = {0, 5, 7, 10};          static int chord_sus7thSize = 4;
static float chord_dom9th[5]      = {0, 4, 7, 10, 14};      static int chord_dom9thSize = 5;
static float chord_domMin9th[5]   = {0, 4, 7, 10, 13};      static int chord_domMin9thSize = 5;
static float chord_major9th[5]    = {0, 4, 7, 11, 14};      static int chord_major9thSize = 5;
static float chord_minor9th[5]    = {0, 3, 7, 10, 14};      static int chord_minor9thSize = 5;
static float chord_major69[5]     = {0, 4, 7, 9, 14};       static int chord_major69Size = 5;
static float chord_minor69[5]     = {0, 3, 7, 9, 14};       static int chord_minor69Size = 5;
static float chord_9thFlat5th[5]  = {0, 4, 6, 10, 14};      static int chord_9thFlat5thSize = 5;
static float chord_9thSharp5th[5] = {0, 4, 8, 10, 14};      static int chord_9thSharp5thSize = 5;
static float chord_dom11th[5]     = {0, 7, 10, 14, 18};     static int chord_dom11thSize = 5;
static float chord_minor11th[6]   = {0, 3, 7, 10, 14, 17};  static int chord_minor11thSize = 6;
static float chord_unison4[1]     = {0};                    static int chord_unison4Size = 1;
static float chord_unison8[1]     = {0};                    static int chord_unison8Size = 1;

static float* chords[NUM_CHORDS] = {
    chord_oneNote, chord_minor2nd, chord_major2nd, chord_minor3rd, chord_major3rd,
    chord_4th, chord_tritone, chord_5th, chord_aug5th, chord_6th, chord_minor7thInt, chord_major7thInt,
    chord_octave, chord_subOctave, chord_oct2Octave, chord_minorTriad, chord_majorTriad,
    chord_susTriad, chord_augTriad, chord_dimTriad, chord_major6th, chord_major7th, chord_dom7th,
    chord_minor7th, chord_halfDim7th, chord_dim7th, chord_sus7th, chord_dom9th, chord_domMin9th,
    chord_major9th, chord_minor9th, chord_major69, chord_minor69, chord_9thFlat5th,
    chord_9thSharp5th, chord_dom11th, chord_minor11th, chord_unison4, chord_unison8
};

static int chordSizes[NUM_CHORDS] = {
    chord_oneNoteSize, chord_minor2ndSize, chord_major2ndSize, chord_minor3rdSize,
    chord_major3rdSize, chord_4thSize, chord_tritoneSize, chord_5thSize, chord_aug5thSize,
    chord_6thSize, chord_minor7thIntSize, chord_major7thIntSize, chord_octaveSize, chord_subOctaveSize,
    chord_oct2OctaveSize, chord_minorTriadSize, chord_majorTriadSize, chord_susTriadSize,
    chord_augTriadSize, chord_dimTriadSize, chord_major6thSize, chord_major7thSize,
    chord_dom7thSize, chord_minor7thSize, chord_halfDim7thSize, chord_dim7thSize, chord_sus7thSize,
    chord_dom9thSize, chord_domMin9thSize, chord_major9thSize, chord_minor9thSize,
    chord_major69Size, chord_minor69Size, chord_9thFlat5thSize, chord_9thSharp5thSize,
    chord_dom11thSize, chord_minor11thSize, chord_unison4Size, chord_unison8Size
};

static std::string chordNames[NUM_CHORDS] = {"Single", "Min 2", "Maj 2", "Min 3", "Maj 3", "4th", "Tritone",
                                             "5th", "Aug5 Int", "6th", "Min7 Int", "Maj7 Int", "Octave", "SubOct",
                                             "2 Oct", "MinTriad", "MajTriad", "SusTriad", "AugTriad", "DimTriad",
                                             "Maj 6th", "Maj 7th", "Dom 7th", "Min 7th", "HalfDim7", "Dim 7th",
                                             "Sus 7th", "Dom 9th", "Dom m9", "Maj 9th", "Min 9th", "Maj6th9",
                                             "m6th9", "9b5th", "9#5th", "Dom 11th", "Min 11th", "Uni 5",
                                             "Uni 7"};

std::vector<float> invertChord(const std::vector<float>& chord, int inversions, long fullInversion) {
    std::vector<float> invertedChord = chord;
    int shifts = 0;
    float lastNote = chord.back();
    int globalOctave = 0;
    int localOctave = 1;
    int direction = 1;

    if(inversions < 0) {
        direction = -1;
    }
    shifts = abs(inversions) % chord.size();
    globalOctave = (int)((float)inversions / (float)chord.size());
    if(fullInversion) {
        globalOctave *= 1 + (int)(lastNote / 12.f);
        localOctave = 1 + (int)(lastNote / 12.f);
    }

    for(auto i = 0; i < shifts; ++i) {
        if(direction == 1) {
            invertedChord[i] += 12 * localOctave;
        }
        else {
            invertedChord[chord.size() - 1 - i] += -12 * localOctave;
        }
    }
    for(auto& i : invertedChord) {
        i += 12 * globalOctave;
    }

    return invertedChord;
}

std::vector<float> getChord(int chord, float detune, long inversions, long fullInversion) {
    std::vector<float> chordNotes(chords[chord], chords[chord] + chordSizes[chord]);
    if((Chords)chord == CHORD_ONE_NOTE) {
        return chordNotes;
    }
    if((Chords)chord == CHORD_UNISON4) {
        for(unsigned long i = 0; i < 4; ++i) {
            chordNotes.push_back(chordNotes[0]);
        }
    }
    else if((Chords)chord == CHORD_UNISON8) {
        for(unsigned long i = 0; i < 6; ++i) {
            chordNotes.push_back(chordNotes[0]);
        }
    }
    chordNotes = invertChord(chordNotes, inversions, fullInversion);

    float detuneTable[8] = {2.0, -2.0, 1.5, -1.5, 1.0, -1.0, 0.5, -0.5};
    for(unsigned long i = 0; i < chordNotes.size(); ++i) {
        chordNotes[i] *= kSemi;
        chordNotes[i] += detune * detuneTable[i];
    }
    return chordNotes;
}

#endif
