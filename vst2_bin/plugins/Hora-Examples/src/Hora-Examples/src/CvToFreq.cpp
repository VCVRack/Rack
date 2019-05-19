#include "CvToFreq.h"

void CvToFreq::setinput(float _input)
{
    input = _input;
}
void CvToFreq::setbase(float _base)
{
     base = _base;
}
float CvToFreq::getoutput()
{
   float output = powf(2.0, input) * base;
    return output;
}
