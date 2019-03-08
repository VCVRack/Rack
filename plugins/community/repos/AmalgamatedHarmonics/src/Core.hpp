#pragma once

#include <iostream>

#include "dsp/digital.hpp"

#include "AH.hpp"

namespace rack_plugin_AmalgamatedHarmonics {

struct AHPulseGenerator {
	float time = 0.f;
	float pulseTime = 0.f;
	
	bool ishigh() {
			return time < pulseTime;
	}
	
	bool process(float deltaTime) {
		time += deltaTime;
		return time < pulseTime;
	}

	bool trigger(float pulseTime) {
		// Keep the previous pulseTime if the existing pulse would be held longer than the currently requested one.
		if (time + pulseTime >= this->pulseTime) {
			time = 0.f;
			this->pulseTime = pulseTime;
			return true;
		} else {
			return false;
		}
	}
};

struct BpmCalculator {
	
	float timer = 0.0f;
	int misses = 0;
	float seconds = 0;
	SchmittTrigger gateTrigger;
	
	inline bool checkBeat(int mult) {
		return ( ((timer - mult * seconds) * (timer - mult * seconds) / (seconds * seconds) < 0.2f ) && misses < 4);
	}

	float calculateBPM(float delta, float input) {

		if (gateTrigger.process(input) ) {

			if (timer > 0) {

				float new_seconds;
				bool found = false;

				for(int mult = 1; !found && mult < 20; mult++ )  {

					if (checkBeat(mult)) {
						new_seconds = timer / mult;
						if (mult == 1) {
							misses = 0;
						} else {
							misses++;
						}
					
						found = true;
					};
				
				};

				if (!found) {
					// std::cerr << "default. misses = " << misses << "\n";
					new_seconds = timer;
					misses = 0;
				}

				float a = 0.5f; // params[SMOOTH_PARAM].value;
				seconds = ((1.0f - a) * seconds + a * new_seconds);
				timer -= seconds;

			}

		};

		timer += delta;
		if (seconds < 2.0e-05) {
			return 0.0f;
		} else {
			return 60.0f / seconds;
		}
	};

};


struct ChordDef {
	int number;
	std::string quality;
	int	root[6];
	int	first[6];
	int	second[6];
};

struct Core {

	static constexpr float TRIGGER = 1e-3f;

 	static constexpr float SEMITONE = 1.0f / 12.0f;

	// Reference, midi note to scale
	// 0	1
	// 1	b2 (#1)
	// 2	2
	// 3	b3 (#2)
	// 4	3
	// 5	4
	// 6	b5 (#4)
	// 7	5
	// 8	b6 (#5)
	// 9 	6
	// 10	b7 (#6)
	// 11	7

	// http://www.grantmuller.com/MidiReference/doc/midiReference/ScaleReference.html
	// Although their definition of the Blues scale is wrong
	// Added the octave note to ensure that the last note is correctly processed
	int ASCALE_CHROMATIC      [13]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}; 	// All of the notes
	int ASCALE_IONIAN         [8] = {0, 2, 4, 5, 7, 9, 11, 12};			// 1,2,3,4,5,6,7
	int ASCALE_DORIAN         [8] = {0, 2, 3, 5, 7, 9, 10, 12};			// 1,2,b3,4,5,6,b7
	int ASCALE_PHRYGIAN       [8] = {0, 1, 3, 5, 7, 8, 10, 12};			// 1,b2,b3,4,5,b6,b7
	int ASCALE_LYDIAN         [8] = {0, 2, 4, 6, 7, 9, 10, 12};			// 1,2,3,#4,5,6,7
	int ASCALE_MIXOLYDIAN     [8] = {0, 2, 4, 5, 7, 9, 10, 12};			// 1,2,3,4,5,6,b7 
	int ASCALE_AEOLIAN        [8] = {0, 2, 3, 5, 7, 8, 10, 12};			// 1,2,b3,4,5,b6,b7
	int ASCALE_LOCRIAN        [8] = {0, 1, 3, 5, 6, 8, 10, 12};			// 1,b2,b3,4,b5,b6,b7
	int ASCALE_MAJOR_PENTA    [6] = {0, 2, 4, 7, 9, 12};				// 1,2,3,5,6
	int ASCALE_MINOR_PENTA    [6] = {0, 3, 5, 7, 10, 12};				// 1,b3,4,5,b7
	int ASCALE_HARMONIC_MINOR [8] = {0, 2, 3, 5, 7, 8, 11, 12};			// 1,2,b3,4,5,b6,7
	int ASCALE_BLUES          [7] = {0, 3, 5, 6, 7, 10, 12};			// 1,b3,4,b5,5,b7

	enum Notes {
		NOTE_C = 0,
		NOTE_D_FLAT, // C Sharp
		NOTE_D,
		NOTE_E_FLAT, // D Sharp
		NOTE_E,
		NOTE_F,
		NOTE_G_FLAT, //F Sharp
		NOTE_G,
		NOTE_A_FLAT, // G Sharp
		NOTE_A,
		NOTE_B_FLAT, // A Sharp
		NOTE_B,
		NUM_NOTES
	};

	int CIRCLE_FIFTHS [12] = {
		NOTE_C,
		NOTE_G,
		NOTE_D,
		NOTE_A,
		NOTE_E, 
		NOTE_B,
		NOTE_G_FLAT,
		NOTE_D_FLAT,
		NOTE_A_FLAT,
		NOTE_E_FLAT,
		NOTE_B_FLAT,
		NOTE_F
	};
	
	std::string noteNames[12] = {
		"C",
		"C#/Db",
		"D",
		"D#/Eb",
		"E",
		"F",
		"F#/Gb",
		"G",
		"G#/Ab",
		"A",
		"A#/Bb",
		"B",
	};

	enum Scales {
		SCALE_CHROMATIC = 0,
 		SCALE_IONIAN,
		SCALE_DORIAN,
		SCALE_PHRYGIAN,
		SCALE_LYDIAN,
		SCALE_MIXOLYDIAN,
		SCALE_AEOLIAN,
		SCALE_LOCRIAN,
		SCALE_MAJOR_PENTA,
		SCALE_MINOR_PENTA,
		SCALE_HARMONIC_MINOR,
		SCALE_BLUES,
		NUM_SCALES
	};

	std::string scaleNames[12] = {
		"Chromatic",
		"Ionian (Major)",
		"Dorian",
		"Phrygian",
		"Lydian",
		"Mixolydian",
		"Aeolian (Natural Minor)",
		"Locrian",
		"Major Pentatonic",
		"Minor Pentatonic",
		"Harmonic Minor",
		"Blues"
	};

	std::string intervalNames[13] {
		"1",
		"b2",
		"2",
		"b3",
		"3",
		"4",
		"b5",
		"5",
		"b6",
		"6",
		"b7",
		"7",
		"O"
	};
	
	enum Modes {
 		MODE_IONIAN = 0,
		MODE_DORIAN,
		MODE_PHRYGIAN,
		MODE_LYDIAN,
		MODE_MIXOLYDIAN,
		MODE_AEOLIAN,
		MODE_LOCRIAN,
		NUM_MODES
	};
	
	std::string modeNames[7] {
		"Ionian (M)",
		"Dorian",
		"Phrygian",
		"Lydian",
		"Mixolydian",
		"Aeolian (m)",
		"Locrian"
	};

	enum DEGREES {
 		DEGREE_I = 0,
		DEGREE_II,
		DEGREE_III,
		DEGREE_IV,
		DEGREE_V,
		DEGREE_VI,
		DEGREE_VII,
		NUM_DEGREES
	};		
	
	std::string degreeNames[21] { // Degree * 3 + Quality
		"I",
		"i",
		"i°",
		"II",
		"ii",
		"ii°",
		"III",
		"iii",
		"iii°",
		"IV",
		"iv",
		"iv°",
		"V",
		"v",
		"v°",
		"VI",
		"vi",
		"vi°",
		"VII",
		"vii",
		"vii°"
	};
		
	enum Inversion {
		ROOT,
		FIRST_INV,
		SECOND_INV,
		NUM_INV
	};
	
	std::string inversionNames[3] {
		"",
		"(1)",
		"(2)"
	};
	
	enum Quality {
		MAJ = 0,
		MIN,
		DIM,
		NUM_QUALITY
	};

	std::string qualityNames[3] {
		"Maj",
		"Min",
		"Dim"
	};
		
		
	double gaussrand();
	
	int ipow(int base, int exp);
		
	bool debug = false;
	int poll = 100000;
	int stepX = 0;
	
	/*
	* Convert a V/OCT voltage to a quantized pitch, key and scale, and calculate various information about the quantised note.
	*/
	
	float getPitchFromVolts(float inVolts, int inRoot, int inScale, int *outNote, int *outDegree);

	float getPitchFromVolts(float inVolts, float inRoot, float inScale, int *outRoot, int *outScale, int *outNote, int *outDegree);
	
	/*
 	 * Convert a root note (relative to C, C=0) and positive semi-tone offset from that root to a voltage (1V/OCT, 0V = C4 (or 3??))
	 */
	float getVoltsFromPitch(int inRoot, int inNote){	
		return (inRoot + inNote) * SEMITONE;
	}
	
	float getVoltsFromScale(int scale) {
		return rescale(scale, 0.0f, NUM_SCALES - 1, 0.0f, 10.0f);
	}
	
	int getScaleFromVolts(float volts) {
		return round(rescale(fabs(volts), 0.0f, 10.0f, 0.0f, NUM_SCALES - 1));
	}
	
	int getModeFromVolts(float volts) {
		int mode = round(rescale(fabs(volts), 0.0f, 10.0f, 0.0f, NUM_SCALES - 1));
		return clamp(mode - 1, 0, 6);
	}
	
	float getVoltsFromMode(int mode) {
		// Mode 0 = IONIAN, MODE 6 = LOCRIAN -> Scale 1 - 7
		return rescale(mode + 1, 0.0f, NUM_SCALES - 1, 0.0f, 10.0f);
	}
	
	float getVoltsFromKey(int key) {
		return rescale(key, 0.0f, NUM_NOTES - 1, 0.0f, 10.0f);
	}
	
	int getKeyFromVolts(float volts) {
		return round(rescale(fabs(volts), 0.0f, 10.0f, 0.0f, NUM_NOTES - 1));
	}
	
	void getRootFromMode(int inMode, int inRoot, int inTonic, int *currRoot, int *quality);
	
	const static int NUM_CHORDS = 99;

	ChordDef ChordTable[NUM_CHORDS] {
		{	0	,"None",{	-24	,	-24	,	-24	,	-24	,	-24	,	-24	},{	-24	,	-24	,	-24	,	-24	,	-24	,	-24	},{	-24	,	-24	,	-24	,	-24	,	-24	,	-24	}},
		{	1	,"",{	0	,	4	,	7	,	-24	,	-20	,	-17	},{	12	,	4	,	7	,	-12	,	-20	,	-17	},{	12	,	16	,	7	,	-12	,	-8	,	-17	}},
		{	2	,"M#5",{	0	,	4	,	8	,	-24	,	-20	,	-16	},{	12	,	4	,	8	,	-12	,	-20	,	-16	},{	12	,	16	,	8	,	-12	,	-8	,	-16	}},
		{	3	,"M#5add9",{	0	,	4	,	8	,	14	,	-24	,	-20	},{	12	,	4	,	8	,	14	,	-24	,	-20	},{	12	,	16	,	8	,	14	,	-12	,	-20	}},
		{	4	,"M13",{	0	,	4	,	7	,	11	,	14	,	21	},{	12	,	4	,	7	,	11	,	14	,	21	},{	12	,	16	,	7	,	11	,	14	,	21	}},
		{	5	,"M6",{	0	,	4	,	7	,	21	,	-24	,	-20	},{	12	,	4	,	7	,	21	,	-24	,	-20	},{	12	,	16	,	7	,	21	,	-12	,	-20	}},
		{	6	,"M6#11",{	0	,	4	,	7	,	9	,	18	,	-24	},{	12	,	4	,	7	,	9	,	18	,	-24	},{	12	,	16	,	7	,	9	,	18	,	-24	}},
		{	7	,"M6/9",{	0	,	4	,	7	,	9	,	14	,	-24	},{	12	,	4	,	7	,	9	,	14	,	-24	},{	12	,	16	,	7	,	9	,	14	,	-24	}},
		{	8	,"M6/9#11",{	0	,	4	,	7	,	9	,	14	,	18	},{	12	,	4	,	7	,	9	,	14	,	18	},{	12	,	16	,	7	,	9	,	14	,	18	}},
		{	9	,"M7#11",{	0	,	4	,	7	,	11	,	18	,	-24	},{	12	,	4	,	7	,	11	,	18	,	-24	},{	12	,	16	,	7	,	11	,	18	,	-24	}},
		{	10	,"M7#5",{	0	,	4	,	8	,	11	,	-24	,	-20	},{	12	,	4	,	8	,	11	,	-24	,	-20	},{	12	,	16	,	8	,	11	,	-12	,	-20	}},
		{	11	,"M7#5sus4",{	0	,	5	,	8	,	11	,	-24	,	-19	},{	12	,	5	,	8	,	11	,	-24	,	-19	},{	12	,	17	,	8	,	11	,	-12	,	-19	}},
		{	12	,"M7#9#11",{	0	,	4	,	7	,	11	,	15	,	18	},{	12	,	4	,	7	,	11	,	15	,	18	},{	12	,	16	,	7	,	11	,	15	,	18	}},
		{	13	,"M7add13",{	0	,	4	,	7	,	9	,	11	,	14	},{	12	,	4	,	7	,	9	,	11	,	14	},{	12	,	16	,	7	,	9	,	11	,	14	}},
		{	14	,"M7b5",{	0	,	4	,	6	,	11	,	-24	,	-20	},{	12	,	4	,	6	,	11	,	-24	,	-20	},{	12	,	16	,	6	,	11	,	-12	,	-20	}},
		{	15	,"M7b6",{	0	,	4	,	8	,	11	,	-24	,	-20	},{	12	,	4	,	8	,	11	,	-24	,	-20	},{	12	,	16	,	8	,	11	,	-12	,	-20	}},
		{	16	,"M7b9",{	0	,	4	,	7	,	11	,	13	,	-20	},{	12	,	4	,	7	,	11	,	13	,	-20	},{	12	,	16	,	7	,	11	,	13	,	-20	}},
		{	17	,"M7sus4",{	0	,	5	,	7	,	11	,	-24	,	-19	},{	12	,	5	,	7	,	11	,	-24	,	-19	},{	12	,	17	,	7	,	11	,	-12	,	-19	}},
		{	18	,"M9",{	0	,	4	,	7	,	11	,	14	,	-24	},{	12	,	4	,	7	,	11	,	14	,	-24	},{	12	,	16	,	7	,	11	,	14	,	-24	}},
		{	19	,"M9#11",{	0	,	4	,	7	,	11	,	14	,	18	},{	12	,	4	,	7	,	11	,	14	,	18	},{	12	,	16	,	7	,	11	,	14	,	18	}},
		{	20	,"M9#5",{	0	,	4	,	8	,	11	,	14	,	-24	},{	12	,	4	,	8	,	11	,	14	,	-24	},{	12	,	16	,	8	,	11	,	14	,	-24	}},
		{	21	,"M9#5sus4",{	0	,	5	,	8	,	11	,	14	,	-24	},{	12	,	5	,	8	,	11	,	14	,	-24	},{	12	,	17	,	8	,	11	,	14	,	-24	}},
		{	22	,"M9b5",{	0	,	4	,	6	,	11	,	14	,	-24	},{	12	,	4	,	6	,	11	,	14	,	-24	},{	12	,	16	,	6	,	11	,	14	,	-24	}},
		{	23	,"M9sus4",{	0	,	5	,	7	,	11	,	14	,	-24	},{	12	,	5	,	7	,	11	,	14	,	-24	},{	12	,	17	,	7	,	11	,	14	,	-24	}},
		{	24	,"Madd9",{	0	,	4	,	7	,	14	,	-24	,	-20	},{	12	,	4	,	7	,	14	,	-24	,	-20	},{	12	,	16	,	7	,	14	,	-12	,	-20	}},
		{	25	,"Maj7",{	0	,	4	,	7	,	11	,	-24	,	-20	},{	12	,	4	,	7	,	11	,	-24	,	-20	},{	12	,	16	,	7	,	11	,	-12	,	-20	}},
		{	26	,"Mb5",{	0	,	4	,	6	,	-24	,	-20	,	-18	},{	12	,	4	,	6	,	-12	,	-20	,	-18	},{	12	,	16	,	6	,	-12	,	-8	,	-18	}},
		{	27	,"Mb6",{	0	,	4	,	20	,	-24	,	-20	,	-4	},{	12	,	4	,	20	,	-12	,	-20	,	-4	},{	12	,	16	,	20	,	-12	,	-8	,	-4	}},
		{	28	,"Msus2",{	0	,	2	,	7	,	-24	,	-22	,	-17	},{	12	,	2	,	7	,	-12	,	-22	,	-17	},{	12	,	14	,	7	,	-12	,	-10	,	-17	}},
		{	29	,"Msus4",{	0	,	5	,	7	,	-24	,	-19	,	-17	},{	12	,	5	,	7	,	-12	,	-19	,	-17	},{	12	,	17	,	7	,	-12	,	-7	,	-17	}},
		{	30	,"Maddb9",{	0	,	4	,	7	,	13	,	-24	,	-20	},{	12	,	4	,	7	,	13	,	-24	,	-20	},{	12	,	16	,	7	,	13	,	-12	,	-20	}},
		{	31	,"7",{	0	,	4	,	7	,	10	,	-24	,	-20	},{	12	,	4	,	7	,	10	,	-24	,	-20	},{	12	,	16	,	7	,	10	,	-12	,	-20	}},
		{	32	,"9",{	0	,	4	,	7	,	10	,	14	,	-24	},{	12	,	4	,	7	,	10	,	14	,	-24	},{	12	,	16	,	7	,	10	,	14	,	-24	}},
		{	33	,"11",{	0	,	7	,	10	,	14	,	17	,	-24	},{	12	,	7	,	10	,	14	,	17	,	-24	},{	12	,	19	,	10	,	14	,	17	,	-24	}},
		{	34	,"13",{	0	,	4	,	7	,	10	,	14	,	21	},{	12	,	4	,	7	,	10	,	14	,	21	},{	12	,	16	,	7	,	10	,	14	,	21	}},
		{	35	,"11b9",{	0	,	7	,	10	,	13	,	17	,	-24	},{	12	,	7	,	10	,	13	,	17	,	-24	},{	12	,	19	,	10	,	13	,	17	,	-24	}},
		{	36	,"13#9",{	0	,	4	,	7	,	10	,	15	,	21	},{	12	,	4	,	7	,	10	,	15	,	21	},{	12	,	16	,	7	,	10	,	15	,	21	}},
		{	37	,"13b5",{	0	,	4	,	6	,	9	,	10	,	14	},{	12	,	4	,	6	,	9	,	10	,	14	},{	12	,	16	,	6	,	9	,	10	,	14	}},
		{	38	,"13b9",{	0	,	4	,	7	,	10	,	13	,	21	},{	12	,	4	,	7	,	10	,	13	,	21	},{	12	,	16	,	7	,	10	,	13	,	21	}},
		{	39	,"13no5",{	0	,	4	,	10	,	14	,	21	,	-24	},{	12	,	4	,	10	,	14	,	21	,	-24	},{	12	,	16	,	10	,	14	,	21	,	-24	}},
		{	40	,"13sus4",{	0	,	5	,	7	,	10	,	14	,	21	},{	12	,	5	,	7	,	10	,	14	,	21	},{	12	,	17	,	7	,	10	,	14	,	21	}},
		{	41	,"69#11",{	0	,	4	,	7	,	9	,	14	,	18	},{	12	,	4	,	7	,	9	,	14	,	18	},{	12	,	16	,	7	,	9	,	14	,	18	}},
		{	42	,"7#11",{	0	,	4	,	7	,	10	,	18	,	-24	},{	12	,	4	,	7	,	10	,	18	,	-24	},{	12	,	16	,	7	,	10	,	18	,	-24	}},
		{	43	,"7#11b13",{	0	,	4	,	7	,	10	,	18	,	20	},{	12	,	4	,	7	,	10	,	18	,	20	},{	12	,	16	,	7	,	10	,	18	,	20	}},
		{	44	,"7#5",{	0	,	4	,	8	,	10	,	-24	,	-20	},{	12	,	4	,	8	,	10	,	-24	,	-20	},{	12	,	16	,	8	,	10	,	-12	,	-20	}},
		{	45	,"7#5#9",{	0	,	4	,	8	,	10	,	15	,	-24	},{	12	,	4	,	8	,	10	,	15	,	-24	},{	12	,	16	,	8	,	10	,	15	,	-24	}},
		{	46	,"7#5b9",{	0	,	4	,	8	,	10	,	13	,	-24	},{	12	,	4	,	8	,	10	,	13	,	-24	},{	12	,	16	,	8	,	10	,	13	,	-24	}},
		{	47	,"7#5b9#11",{	0	,	4	,	8	,	10	,	13	,	18	},{	12	,	4	,	8	,	10	,	13	,	18	},{	12	,	16	,	8	,	10	,	13	,	18	}},
		{	48	,"7#5sus4",{	0	,	5	,	8	,	10	,	-24	,	-19	},{	12	,	5	,	8	,	10	,	-24	,	-19	},{	12	,	17	,	8	,	10	,	-12	,	-19	}},
		{	49	,"7#9",{	0	,	4	,	7	,	10	,	15	,	-24	},{	12	,	4	,	7	,	10	,	15	,	-24	},{	12	,	16	,	7	,	10	,	15	,	-24	}},
		{	50	,"7#9#11",{	0	,	4	,	7	,	10	,	15	,	18	},{	12	,	4	,	7	,	10	,	15	,	18	},{	12	,	16	,	7	,	10	,	15	,	18	}},
		{	51	,"7#9b13",{	0	,	4	,	7	,	10	,	15	,	20	},{	12	,	4	,	7	,	10	,	15	,	20	},{	12	,	16	,	7	,	10	,	15	,	20	}},
		{	52	,"7add6",{	0	,	4	,	7	,	10	,	21	,	-24	},{	12	,	4	,	7	,	10	,	21	,	-24	},{	12	,	16	,	7	,	10	,	21	,	-24	}},
		{	53	,"7b13",{	0	,	4	,	10	,	20	,	-24	,	-20	},{	12	,	4	,	10	,	20	,	-24	,	-20	},{	12	,	16	,	10	,	20	,	-12	,	-20	}},
		{	54	,"7b5",{	0	,	4	,	6	,	10	,	-24	,	-20	},{	12	,	4	,	6	,	10	,	-24	,	-20	},{	12	,	16	,	6	,	10	,	-12	,	-20	}},
		{	55	,"7b6",{	0	,	4	,	7	,	8	,	10	,	-24	},{	12	,	4	,	7	,	8	,	10	,	-24	},{	12	,	16	,	7	,	8	,	10	,	-24	}},
		{	56	,"7b9",{	0	,	4	,	7	,	10	,	13	,	-24	},{	12	,	4	,	7	,	10	,	13	,	-24	},{	12	,	16	,	7	,	10	,	13	,	-24	}},
		{	57	,"7b9#11",{	0	,	4	,	7	,	10	,	13	,	18	},{	12	,	4	,	7	,	10	,	13	,	18	},{	12	,	16	,	7	,	10	,	13	,	18	}},
		{	58	,"7b9#9",{	0	,	4	,	7	,	10	,	13	,	15	},{	12	,	4	,	7	,	10	,	13	,	15	},{	12	,	16	,	7	,	10	,	13	,	15	}},
		{	59	,"7b9b13",{	0	,	4	,	7	,	10	,	13	,	20	},{	12	,	4	,	7	,	10	,	13	,	20	},{	12	,	16	,	7	,	10	,	13	,	20	}},
		{	60	,"7no5",{	0	,	4	,	10	,	-24	,	-20	,	-14	},{	12	,	4	,	10	,	-12	,	-20	,	-14	},{	12	,	16	,	10	,	-12	,	-8	,	-14	}},
		{	61	,"7sus4",{	0	,	5	,	7	,	10	,	-24	,	-19	},{	12	,	5	,	7	,	10	,	-24	,	-19	},{	12	,	17	,	7	,	10	,	-12	,	-19	}},
		{	62	,"7sus4b9",{	0	,	5	,	7	,	10	,	13	,	-24	},{	12	,	5	,	7	,	10	,	13	,	-24	},{	12	,	17	,	7	,	10	,	13	,	-24	}},
		{	63	,"7sus4b9b13",{	0	,	5	,	7	,	10	,	13	,	20	},{	12	,	5	,	7	,	10	,	13	,	20	},{	12	,	17	,	7	,	10	,	13	,	20	}},
		{	64	,"9#11",{	0	,	4	,	7	,	10	,	14	,	18	},{	12	,	4	,	7	,	10	,	14	,	18	},{	12	,	16	,	7	,	10	,	14	,	18	}},
		{	65	,"9#5",{	0	,	4	,	8	,	10	,	14	,	-24	},{	12	,	4	,	8	,	10	,	14	,	-24	},{	12	,	16	,	8	,	10	,	14	,	-24	}},
		{	66	,"9#5#11",{	0	,	4	,	8	,	10	,	14	,	18	},{	12	,	4	,	8	,	10	,	14	,	18	},{	12	,	16	,	8	,	10	,	14	,	18	}},
		{	67	,"9b13",{	0	,	4	,	10	,	14	,	20	,	-24	},{	12	,	4	,	10	,	14	,	20	,	-24	},{	12	,	16	,	10	,	14	,	20	,	-24	}},
		{	68	,"9b5",{	0	,	4	,	6	,	10	,	14	,	-24	},{	12	,	4	,	6	,	10	,	14	,	-24	},{	12	,	16	,	6	,	10	,	14	,	-24	}},
		{	69	,"9no5",{	0	,	4	,	10	,	14	,	-24	,	-20	},{	12	,	4	,	10	,	14	,	-24	,	-20	},{	12	,	16	,	10	,	14	,	-12	,	-20	}},
		{	70	,"9sus4",{	0	,	5	,	7	,	10	,	14	,	-24	},{	12	,	5	,	7	,	10	,	14	,	-24	},{	12	,	17	,	7	,	10	,	14	,	-24	}},
		{	71	,"m",{	0	,	3	,	7	,	-24	,	-21	,	-17	},{	12	,	3	,	7	,	-12	,	-21	,	-17	},{	12	,	15	,	7	,	-12	,	-9	,	-17	}},
		{	72	,"m#5",{	0	,	3	,	8	,	-24	,	-21	,	-16	},{	12	,	3	,	8	,	-12	,	-21	,	-16	},{	12	,	15	,	8	,	-12	,	-9	,	-16	}},
		{	73	,"m11",{	0	,	3	,	7	,	10	,	14	,	17	},{	12	,	3	,	7	,	10	,	14	,	17	},{	12	,	15	,	7	,	10	,	14	,	17	}},
		{	74	,"m11A5",{	0	,	3	,	8	,	10	,	14	,	17	},{	12	,	3	,	8	,	10	,	14	,	17	},{	12	,	15	,	8	,	10	,	14	,	17	}},
		{	75	,"m11b5",{	0	,	3	,	10	,	14	,	17	,	18	},{	12	,	3	,	10	,	14	,	17	,	18	},{	12	,	15	,	10	,	14	,	17	,	18	}},
		{	76	,"m6",{	0	,	3	,	5	,	7	,	21	,	-24	},{	12	,	3	,	5	,	7	,	21	,	-24	},{	12	,	15	,	5	,	7	,	21	,	-24	}},
		{	77	,"m69",{	0	,	3	,	7	,	9	,	14	,	-24	},{	12	,	3	,	7	,	9	,	14	,	-24	},{	12	,	15	,	7	,	9	,	14	,	-24	}},
		{	78	,"m7",{	0	,	3	,	7	,	10	,	-24	,	-21	},{	12	,	3	,	7	,	10	,	-24	,	-21	},{	12	,	15	,	7	,	10	,	-12	,	-21	}},
		{	79	,"m7#5",{	0	,	3	,	8	,	10	,	-24	,	-21	},{	12	,	3	,	8	,	10	,	-24	,	-21	},{	12	,	15	,	8	,	10	,	-12	,	-21	}},
		{	80	,"m7add11",{	0	,	3	,	7	,	10	,	17	,	-24	},{	12	,	3	,	7	,	10	,	17	,	-24	},{	12	,	15	,	7	,	10	,	17	,	-24	}},
		{	81	,"m7b5",{	0	,	3	,	6	,	10	,	-24	,	-21	},{	12	,	3	,	6	,	10	,	-24	,	-21	},{	12	,	15	,	6	,	10	,	-12	,	-21	}},
		{	82	,"m9",{	0	,	3	,	7	,	10	,	14	,	-24	},{	12	,	3	,	7	,	10	,	14	,	-24	},{	12	,	15	,	7	,	10	,	14	,	-24	}},
		{	83	,"#5",{	0	,	3	,	8	,	10	,	14	,	-24	},{	12	,	3	,	8	,	10	,	14	,	-24	},{	12	,	15	,	8	,	10	,	14	,	-24	}},
		{	84	,"m9b5",{	0	,	3	,	10	,	14	,	18	,	-24	},{	12	,	3	,	10	,	14	,	18	,	-24	},{	12	,	15	,	10	,	14	,	18	,	-24	}},
		{	85	,"mMaj7",{	0	,	3	,	7	,	11	,	-24	,	-21	},{	12	,	3	,	7	,	11	,	-24	,	-21	},{	12	,	15	,	7	,	11	,	-12	,	-21	}},
		{	86	,"mMaj7b6",{	0	,	3	,	7	,	8	,	11	,	-24	},{	12	,	3	,	7	,	8	,	11	,	-24	},{	12	,	15	,	7	,	8	,	11	,	-24	}},
		{	87	,"mM9",{	0	,	3	,	7	,	11	,	14	,	-24	},{	12	,	3	,	7	,	11	,	14	,	-24	},{	12	,	15	,	7	,	11	,	14	,	-24	}},
		{	88	,"mM9b6",{	0	,	3	,	7	,	8	,	11	,	14	},{	12	,	3	,	7	,	8	,	11	,	14	},{	12	,	15	,	7	,	8	,	11	,	14	}},
		{	89	,"mb6M7",{	0	,	3	,	8	,	11	,	-24	,	-21	},{	12	,	3	,	8	,	11	,	-24	,	-21	},{	12	,	15	,	8	,	11	,	-12	,	-21	}},
		{	90	,"mb6b9",{	0	,	3	,	8	,	13	,	-24	,	-21	},{	12	,	3	,	8	,	13	,	-24	,	-21	},{	12	,	15	,	8	,	13	,	-12	,	-21	}},
		{	91	,"dim",{	0	,	3	,	6	,	-24	,	-21	,	-18	},{	12	,	3	,	6	,	-12	,	-21	,	-18	},{	12	,	15	,	6	,	-12	,	-9	,	-18	}},
		{	92	,"dim7",{	0	,	3	,	6	,	21	,	-24	,	-21	},{	12	,	3	,	6	,	21	,	-24	,	-21	},{	12	,	15	,	6	,	21	,	-12	,	-21	}},
		{	93	,"dim7M7",{	0	,	3	,	6	,	9	,	11	,	-24	},{	12	,	3	,	6	,	9	,	11	,	-24	},{	12	,	15	,	6	,	9	,	11	,	-24	}},
		{	94	,"dimM7",{	0	,	3	,	6	,	11	,	-24	,	-21	},{	12	,	3	,	6	,	11	,	-24	,	-21	},{	12	,	15	,	6	,	11	,	-12	,	-21	}},
		{	95	,"sus24",{	0	,	2	,	5	,	7	,	-24	,	-22	},{	12	,	2	,	5	,	7	,	-24	,	-22	},{	12	,	14	,	5	,	7	,	-12	,	-22	}},
		{	96	,"augadd#9",{	0	,	4	,	8	,	15	,	-24	,	-20	},{	12	,	4	,	8	,	15	,	-24	,	-20	},{	12	,	16	,	8	,	15	,	-12	,	-20	}},
		{	97	,"madd4",{	0	,	3	,	5	,	7	,	-24	,	-21	},{	12	,	3	,	5	,	7	,	-24	,	-21	},{	12	,	15	,	5	,	7	,	-12	,	-21	}},
		{	98	,"madd9",{	0	,	3	,	7	,	14	,	-24	,	-21	},{	12	,	3	,	7	,	14	,	-24	,	-21	},{	12	,	15	,	7	,	14	,	-12	,	-21	}},		
	};
		
	int ModeQuality[7][7] {
		{MAJ,MIN,MIN,MAJ,MAJ,MIN,DIM}, // Ionian
		{MIN,MIN,MAJ,MAJ,MIN,DIM,MAJ}, // Dorian
		{MIN,MAJ,MAJ,MIN,DIM,MAJ,MIN}, // Phrygian
		{MAJ,MAJ,MIN,DIM,MAJ,MIN,MIN}, // Lydian
		{MAJ,MIN,DIM,MAJ,MIN,MIN,MAJ}, // Mixolydian
		{MIN,DIM,MAJ,MIN,MIN,MAJ,MAJ}, // Aeolian
		{DIM,MAJ,MIN,MIN,MAJ,MAJ,MIN}  // Locrian
	};

	int ModeOffset[7][7] {
		{0,0,0,0,0,0,0},     // Ionian
		{0,0,-1,-1,0,0,-1},  // Dorian
		{0,-1,-1,0,0,-1,-1}, // Phrygian
		{0,0,0,1,0,0,0},     // Lydian
		{0,0,0,0,0,0,-1},    // Mixolydian
		{0,0,-1,0,0,-1,-1},  // Aeolian
		{0,-1,-1,0,-1,-1,-1} // Locrian
	};

	// NOTE_C = 0,
	// NOTE_D_FLAT, // C Sharp
	// NOTE_D,
	// NOTE_E_FLAT, // D Sharp
	// NOTE_E,
	// NOTE_F,
	// NOTE_G_FLAT, //F Sharp
	// NOTE_G,
	// NOTE_A_FLAT, // G Sharp
	// NOTE_A,
	// NOTE_B_FLAT, // A Sharp
	// NOTE_B,

	//0	1	2	3	4	5	6	7	8	9	10	11	12
	int tonicIndex[13] {1, 3, 5, 0, 2, 4, 6, 1, 3, 5, 0, 2, 4};
	int scaleIndex[7] {5, 3, 1, 6, 4, 2, 0};
	int noteIndex[13] { 
		NOTE_G_FLAT,
		NOTE_D_FLAT,
		NOTE_A_FLAT,
		NOTE_E_FLAT,
		NOTE_B_FLAT,
		NOTE_F,
		NOTE_C,
		NOTE_G,
		NOTE_D,
		NOTE_A,
		NOTE_E,		
		NOTE_B,
		NOTE_G_FLAT};	
			
		
};

Core & CoreUtil();

} // namespace rack_plugin_AmalgamatedHarmonics
