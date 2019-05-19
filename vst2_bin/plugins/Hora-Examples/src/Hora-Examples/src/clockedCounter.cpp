#include "clockedCounter.h"

void clockedCounter::setlength(int _length)
{
    length = _length;
}
void clockedCounter::setrate(float _rate)
{
     rate = _rate;
}
void clockedCounter::setreset(float _reset)
{
    if (_reset != 0)
    {
        count = 0;
    }
}
float clockedCounter::getoutput()
{
    count = count + rate;

    if (count >= length)
    {
        count = count-length;
    }
    output = count;
    return output;
}
