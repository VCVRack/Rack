#if !defined(QWELK_COMMON_H)

#include "rack.hpp"
using namespace rack;

typedef uint8_t byte;

// struct TinyKnob : RoundSmallBlackKnob {
//      TinyKnob();
// };



byte minb(byte a, byte b);
byte maxb(byte a, byte b);
int clampi(int v, int l, int h);
float slew(float v, float i, float sa, float min, float max, float shape);

#define QWELK_COMMON_H
#endif
