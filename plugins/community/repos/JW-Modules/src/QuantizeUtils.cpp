#include "rack.hpp"

namespace rack_plugin_JW_Modules {

struct QuantizeUtils {

	//copied & fixed these scales http://www.grantmuller.com/MidiReference/doc/midiReference/ScaleReference.html
	int SCALE_AEOLIAN        [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_BLUES          [6] = {0, 3, 5, 6, 7, 10}; //FIXED!
	int SCALE_CHROMATIC      [12]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	int SCALE_DIATONIC_MINOR [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_DORIAN         [7] = {0, 2, 3, 5, 7, 9, 10};
	int SCALE_HARMONIC_MINOR [7] = {0, 2, 3, 5, 7, 8, 11};
	int SCALE_INDIAN         [7] = {0, 1, 1, 4, 5, 8, 10};
	int SCALE_LOCRIAN        [7] = {0, 1, 3, 5, 6, 8, 10};
	int SCALE_LYDIAN         [7] = {0, 2, 4, 6, 7, 9, 10};
	int SCALE_MAJOR          [7] = {0, 2, 4, 5, 7, 9, 11};
	int SCALE_MELODIC_MINOR  [9] = {0, 2, 3, 5, 7, 8, 9, 10, 11};
	int SCALE_MINOR          [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_MIXOLYDIAN     [7] = {0, 2, 4, 5, 7, 9, 10};
	int SCALE_NATURAL_MINOR  [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_PENTATONIC     [5] = {0, 2, 4, 7, 9};
	int SCALE_PHRYGIAN       [7] = {0, 1, 3, 5, 7, 8, 10};
	int SCALE_TURKISH        [7] = {0, 1, 3, 5, 7, 10, 11};

	enum NoteEnum {
		NOTE_C, 
		NOTE_C_SHARP,
		NOTE_D,
		NOTE_D_SHARP,
		NOTE_E,
		NOTE_F,
		NOTE_F_SHARP,
		NOTE_G,
		NOTE_G_SHARP,
		NOTE_A,
		NOTE_A_SHARP,
		NOTE_B,
		NUM_NOTES
	};

	enum ScaleEnum {
		AEOLIAN,
		BLUES,
		CHROMATIC,
		DIATONIC_MINOR,
		DORIAN,
		HARMONIC_MINOR,
		INDIAN,
		LOCRIAN,
		LYDIAN,
		MAJOR,
		MELODIC_MINOR,
		MINOR,
		MIXOLYDIAN,
		NATURAL_MINOR,
		PENTATONIC,
		PHRYGIAN,
		TURKISH,
		NONE,
		NUM_SCALES
	};

	// long printIter = 0;
	float closestVoltageInScale(float voltsIn, int rootNote, int currScale){
		int *curScaleArr;
		int notesInScale = 0;
		switch(currScale){
			case AEOLIAN:        curScaleArr = SCALE_AEOLIAN;       notesInScale=LENGTHOF(SCALE_AEOLIAN); break;
			case BLUES:          curScaleArr = SCALE_BLUES;         notesInScale=LENGTHOF(SCALE_BLUES); break;
			case CHROMATIC:      curScaleArr = SCALE_CHROMATIC;     notesInScale=LENGTHOF(SCALE_CHROMATIC); break;
			case DIATONIC_MINOR: curScaleArr = SCALE_DIATONIC_MINOR;notesInScale=LENGTHOF(SCALE_DIATONIC_MINOR); break;
			case DORIAN:         curScaleArr = SCALE_DORIAN;        notesInScale=LENGTHOF(SCALE_DORIAN); break;
			case HARMONIC_MINOR: curScaleArr = SCALE_HARMONIC_MINOR;notesInScale=LENGTHOF(SCALE_HARMONIC_MINOR); break;
			case INDIAN:         curScaleArr = SCALE_INDIAN;        notesInScale=LENGTHOF(SCALE_INDIAN); break;
			case LOCRIAN:        curScaleArr = SCALE_LOCRIAN;       notesInScale=LENGTHOF(SCALE_LOCRIAN); break;
			case LYDIAN:         curScaleArr = SCALE_LYDIAN;        notesInScale=LENGTHOF(SCALE_LYDIAN); break;
			case MAJOR:          curScaleArr = SCALE_MAJOR;         notesInScale=LENGTHOF(SCALE_MAJOR); break;
			case MELODIC_MINOR:  curScaleArr = SCALE_MELODIC_MINOR; notesInScale=LENGTHOF(SCALE_MELODIC_MINOR); break;
			case MINOR:          curScaleArr = SCALE_MINOR;         notesInScale=LENGTHOF(SCALE_MINOR); break;
			case MIXOLYDIAN:     curScaleArr = SCALE_MIXOLYDIAN;    notesInScale=LENGTHOF(SCALE_MIXOLYDIAN); break;
			case NATURAL_MINOR:  curScaleArr = SCALE_NATURAL_MINOR; notesInScale=LENGTHOF(SCALE_NATURAL_MINOR); break;
			case PENTATONIC:     curScaleArr = SCALE_PENTATONIC;    notesInScale=LENGTHOF(SCALE_PENTATONIC); break;
			case PHRYGIAN:       curScaleArr = SCALE_PHRYGIAN;      notesInScale=LENGTHOF(SCALE_PHRYGIAN); break;
			case TURKISH:        curScaleArr = SCALE_TURKISH;       notesInScale=LENGTHOF(SCALE_TURKISH); break;
			case NONE:           return voltsIn;
		}

		//C1 == -2.00, C2 == -1.00, C3 == 0.00, C4 == 1.00
		//B1 == -1.08, B2 == -0.08, B3 == 0.92, B4 == 1.92
		float closestVal = 10.0;
		float closestDist = 10.0;
		float scaleNoteInVolts = 0;
		float distAway = 0;
		int octaveInVolts = int(floorf(voltsIn));
		float voltMinusOct = voltsIn - octaveInVolts;
		// int nextOctaveInVolts = (octaveInVolts + 1) * inverseMult; // If I decide to round to the next octave.
		for (int i=0; i < notesInScale; i++) {
			scaleNoteInVolts = curScaleArr[i] / 12.0;
			distAway = fabs(voltMinusOct - scaleNoteInVolts);
			if(distAway < closestDist){
				closestVal = scaleNoteInVolts;
				closestDist = distAway;
			}

			// If I decide to round to the next octave.  (Ableton does NOT do this in their scale midi effect)
			// float distToNextOct = fabs(nextOctaveInVolts - voltsIn);
			// if(distToNextOct < closestDist){
			// 	closestVal = nextOctaveInVolts;
			// 	closestDist = distToNextOct;
			// }

			// if(printIter%100000==0){
			// 	printf("i:%i, rootNote:%i, voltsIn:%f, octaveInVolts:%i, closestVal:%f, closestDist:%f\n", 
			// 	        i,    rootNote,    voltsIn,    octaveInVolts,    closestVal,    closestDist);
			// 	printIter = 0;
			// }
		}
		// printIter++;
		return octaveInVolts + rootNote/12.0 + closestVal;
	}

	std::string noteName(int note) {
		switch(note){
			case NOTE_C:       return "C";
			case NOTE_C_SHARP: return "C#";
			case NOTE_D:       return "D";
			case NOTE_D_SHARP: return "D#";
			case NOTE_E:       return "E";
			case NOTE_F:       return "F";
			case NOTE_F_SHARP: return "F#";
			case NOTE_G:       return "G";
			case NOTE_G_SHARP: return "G#";
			case NOTE_A:       return "A";
			case NOTE_A_SHARP: return "A#";
			case NOTE_B:       return "B";
			default: return "";
		}
	}

	std::string scaleName(int scale) {
		switch(scale){
			case AEOLIAN:        return "Aeolian";
			case BLUES:          return "Blues";
			case CHROMATIC:      return "Chromat";
			case DIATONIC_MINOR: return "Diat Min";
			case DORIAN:         return "Dorian";
			case HARMONIC_MINOR: return "Harm Min";
			case INDIAN:         return "Indian";
			case LOCRIAN:        return "Locrian";
			case LYDIAN:         return "Lydian";
			case MAJOR:          return "Major";
			case MELODIC_MINOR:  return "Mel Min";
			case MINOR:          return "Minor";
			case MIXOLYDIAN:     return "Mixolyd";
			case NATURAL_MINOR:  return "Nat Min";
			case PENTATONIC:     return "Penta";
			case PHRYGIAN:       return "Phrygian";
			case TURKISH:        return "Turkish";
			case NONE:           return "None";
			default: return "";
		}
	}
};

} // namespace rack_plugin_JW_Modules
