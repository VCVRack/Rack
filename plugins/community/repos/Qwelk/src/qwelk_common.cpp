#include "util/math.hpp"
#include "qwelk_common.h"

using namespace rack;


// TinyKnob::TinyKnob()
// {
//     box.size = Vec(20, 20);
// }


byte minb(byte a, byte b) {return a < b ? a : b;}
byte maxb(byte a, byte b) {return a > b ? a : b;}
int clampi(int v, int l, int h) {return (v < l) ? l : ((v > h) ? h : v);}

float slew(float v, float i, float sa, float min, float max, float shape)
{
    float ret = v;

    if (i > v) {
        float s = max * powf(min / max, sa);
        ret += s * crossfade(1.0, (1 / 10.0) * (i - v), shape) / engineGetSampleRate();
        if (ret > i)
            ret = i;
    } else if (i < v) {
        float s = max * powf(min / max, sa);
        ret -= s * crossfade(1.0, (1 / 10.0) * (v - i), shape) / engineGetSampleRate();
        if (ret < i)
            ret = i;
    }
    
    return ret;
}
