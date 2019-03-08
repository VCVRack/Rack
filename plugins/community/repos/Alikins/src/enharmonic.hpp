#include "rack.hpp"
using namespace rack;

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

namespace rack_plugin_Alikins {

// using namespace rack;

std::vector<std::string> note_name_vec = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

enum NoteName {
    C_NATURAL,
    C_SHARP,
    D_NATURAL,
    D_SHARP,
    E_NATURAL,
    F_NATURAL,
    F_SHARP,
    G_NATURAL,
    G_SHARP,
    A_NATURAL,
    A_SHARP,
    B_NATURAL
};

// add a NoteValue obj based on volts, with
// methods for voltToText, voltToNote, etc
struct NoteOct {
    std::string name;
    std::string octave;
    std::string flag;
    int rank;

    NoteOct() {
        // if no octave number, assume it is octave 4
        name = "C";
        octave = "4";
        flag = "";
        rank = 0;
    }
};

// Build a map of note_name ('C4', 'Ab3', etc) to it's CV voltage
std::map<std::string, float> gen_note_name_map() {

    double volt = -10.0;
    std::string note_name;
    std::map<std::string, float> note_name_map;
    std::vector<std::string>::iterator it;
    double semi = 1.0/12.0;

    // FIXME: add a map of note name (including enharmonic) to voltage offset from C
    //        then just iterate over it for each octave
    for (int i = -6; i <= 14; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            // debug("oct=%d note=%s volt=%f ", i, note_name_vec[j].c_str(), volt);
            note_name_map[stringf("%s%d",
                                  note_name_vec[j].c_str(), i)] = volt;
            volt += semi;
        }
    }
    return note_name_map;
}

std::map<std::string, std::string> gen_enharmonic_name_map() {
    std::map<std::string, std::string> enharmonic_map;

    enharmonic_map["c"] = "C";
    enharmonic_map["C"] = "C";

    enharmonic_map["C#"] = "C#";
    enharmonic_map["c#"] = "C#";
    enharmonic_map["Db"] = "C#";
    enharmonic_map["db"] = "C#";

    enharmonic_map["d"] = "D";
    enharmonic_map["D"] = "D";

    enharmonic_map["D#"] = "D#";
    enharmonic_map["d#"] = "D#";
    enharmonic_map["Eb"] = "D#";
    enharmonic_map["eb"] = "D#";

    enharmonic_map["E"] = "E";
    enharmonic_map["e"] = "E";
    enharmonic_map["Fb"] = "E";
    enharmonic_map["fb"] = "E";

    enharmonic_map["E#"] = "F";
    enharmonic_map["e#"] = "F";
    enharmonic_map["F"] = "F";
    enharmonic_map["f"] = "F";

    enharmonic_map["F#"] = "F#";
    enharmonic_map["f#"] = "F#";
    enharmonic_map["Gb"] = "F#";
    enharmonic_map["Gb"] = "F#";

    enharmonic_map["G"] = "G";
    enharmonic_map["g"] = "G";

    enharmonic_map["G#"] = "G#";
    enharmonic_map["g#"] = "G#";
    enharmonic_map["Ab"] = "G#";
    enharmonic_map["ab"] = "G#";

    enharmonic_map["A"] = "A";
    enharmonic_map["a"] = "A";

    enharmonic_map["A#"] = "A#";
    enharmonic_map["a#"] = "A#";
    enharmonic_map["Bb"] = "A#";
    enharmonic_map["bb"] = "A#";

    enharmonic_map["B"] = "B";
    enharmonic_map["b"] = "B";
    enharmonic_map["Cb"] = "B";
    enharmonic_map["cb"] = "B";

    enharmonic_map["B#"] = "C";
    enharmonic_map["b#"] = "C";

    return enharmonic_map;
}

std::map<std::string, std::string> enharmonic_name_map = gen_enharmonic_name_map();
std::map<std::string, float> note_name_to_volts_map = gen_note_name_map();

NoteOct* parseNote(std::string text) {
    // split into 'stuff before any int or -' and a number like string
    // ie C#11 -> C# 11,  A-4 -> A 4
    std::size_t note_flag_found_loc = text.find_last_of("#♯b♭");

    std::string note_flag = "";
    if(note_flag_found_loc!=std::string::npos){
        note_flag = text[note_flag_found_loc];
    }

    std::size_t found = text.find_first_of("-0123456789");

    // if no octave number, assume it is octave 4
    std::string note_name = text;
    std::string note_oct = "4";

    if(found != std::string::npos){
        note_name = text.substr(0, found);
        note_oct = text.substr(found, text.length());
    }

    // debug("parseNote note_name: %s, note_oct: %s", note_name.c_str(), note_oct.c_str());

    std::string can_note_name = enharmonic_name_map[note_name];
    // debug("parseNote can_not_name: %s", can_note_name.c_str());

    NoteOct *noteOct = new NoteOct();
    noteOct->name = can_note_name;
    noteOct->octave = note_oct;
    noteOct->flag = note_flag;

    return noteOct;
}

std::string getCanNoteId(NoteOct *noteOct) {
    std::string can_note_name = enharmonic_name_map[noteOct->name];

    std::string can_note_id = stringf("%s%s", can_note_name.c_str(), noteOct->octave.c_str());

    return can_note_id;
}

} // namespace rack_plugin_Alikins
