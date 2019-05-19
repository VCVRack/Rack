#include "FreqToRate.h"

void FreqToRate::setinput(float _input)
{
    input = _input;
}
void FreqToRate::setsampleRate(float _base)
{
     base = _base;
}
void FreqToRate::settableLength(float _length)
{
     tableLength = _length;
}
float FreqToRate::getoutput()
{
   float output = input / base * tableLength;
    return output;
}
