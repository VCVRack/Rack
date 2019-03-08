
#pragma once

#include "rack.hpp"

#include <vector>
#include <string>
#include <sstream>

/**
 * Displays semitones in a Label. Also Octaves.
 */
class SemitoneDisplay
{
public:
    SemitoneDisplay(rack::Module* module) :
        module(module)
    {

    }
    void step();


    /**
     * pass in the label component that will be displaying semitones
     */
    void setSemiLabel(rack::Label* l, int id)
    {
        semiLabel = l;
        semiXPosition = l->box.pos.x;
        semiParameterId = id;
    }

    /**
     * pass in the label component that will be displaying octaves
     */
    void setOctLabel(rack::Label* l, int id)
    {
        octLabel = l;
        octXPosition = l->box.pos.x;
        octParameterId = id;
    }
private:
    rack::Module* module;
    rack::Label* semiLabel = nullptr;
    rack::Label* octLabel = nullptr;
    float semiXPosition = 0;
    float octXPosition = 0;
    int semiParameterId = -1;
    int octParameterId = -1;

    int lastSemi = 100;
    int lastOct = 100;

    const std::vector<std::string> names = {
        "C",
        "C#",
        "D",
        "D#",
        "E",
        "F",
        "F#",
        "G",
        "G#",
        "A",
        "A#",
        "B"
    };
};

inline void SemitoneDisplay::step()
{
    int curSemi = 0;
    int curOct = 0;
    if (semiParameterId >= 0) {
        curSemi = (int) std::round(module->params[semiParameterId].value);
    }
    if (octParameterId >= 0) {
        curOct = (int) std::round(module->params[octParameterId].value);
    }

    if (curSemi != lastSemi || curOct != lastOct) {
        lastSemi = curSemi;
        lastOct = curOct;

       
        int semi = lastSemi;
        int oct = lastOct;
        if (semi < 0) {
            semi += 12;
            --oct;
        }

        if (semiLabel) {
            std::stringstream so;
            so << "Semi " << names[semi];
            semiLabel->text = so.str();
        }
        if (octLabel) {
            std::stringstream so;
            so << "Oct " << (5 + oct);
            octLabel->text = so.str();
        }
    }
}