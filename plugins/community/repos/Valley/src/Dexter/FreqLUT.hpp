#ifndef DSJ_VALLEY_DEXTER_FREQLUT
#define DSJ_VALLEY_DEXTER_FREQLUT

#include <vector>
#include <cmath>

class FreqLUT {
public:
    FreqLUT();
    float getFrequency(float pitch);
private:
    std::vector<float> _lut;
    float _resolution;
    float _inputLow, _inputHigh;
    float _pitch;
    long _pos;
    float _frac;

    void makeLUT();
};

#endif // DSJ_VALLEY_DEXTER_FREQLUT
