#pragma once

#include <utility>

class PitchUtils
{
public:
    static constexpr float semitone = 1.f / 12.f;    // one semitone is a 1/12 volt
    static constexpr float octave = 1.f;
    static std::pair<int, int> cvToPitch(float cv);
    static int cvToSemitone(float cv);
    static float pitchToCV(int octave, int semi);
    static bool isAccidental(float cv);
};

inline std::pair<int, int> PitchUtils::cvToPitch(float cv)
{
     // VCV 0 is C4
    int octave = int(std::floor(cv));
    float remainder = cv - octave;
    octave += 4;
    float s = remainder * 12;
    int semi = int(std::round(s));
    return std::pair<int, int>(octave, semi);
}

inline  int PitchUtils::cvToSemitone(float cv)
{
    auto p = cvToPitch(cv);
    return p.first * 12 + p.second;
}

inline float PitchUtils::pitchToCV(int octave, int semi)
{
    return float(octave - 4) + semi * semitone;
}

inline bool PitchUtils::isAccidental(float cv)
{
    int semi = cvToPitch(cv).second;
    bool ret = false;
    switch (semi) {
        case 1:
        case 3:
        case 6:
        case 8:
        case 10:
            ret = true;
            break;
    }
    return ret;
}
