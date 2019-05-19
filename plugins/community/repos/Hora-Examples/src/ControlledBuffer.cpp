#include "ControlledBuffer.h"

void ControlledBuffer::setinput(float _input)
{
    input = _input;
}
void ControlledBuffer::setrec(float _rec)
{
     rec = _rec;
}
void ControlledBuffer::setindex(float _index)
{

    if (_index < buf.size())
    {
        rawIndex = _index;
        index = int(_index);
        if (index > rawIndex && index > 0)
        {
            index = index-1;
        }

    }
    else
    {
        /*int j = buf.size();
        index = _index%j;*/
        index = 0;
        rawIndex = 0;
    }
    if (index > buf.size())
    {
        index = 0;
        rawIndex = 0;
    }
}
void ControlledBuffer::setreset(float _reset)
{
    if (_reset != 0)
    {
        index = 0;
        rawIndex = 0;
    }
}
void ControlledBuffer::setread(float _read)
{
     read = _read;
}
float ControlledBuffer::getoutput()
{
    if (rec != 0 && wasOnRec == false)
    {
        buf.clear();
        index = 0;
        wasOnRec = true;
        buf.push_back(input);
        output = 0;
    }
    else if (rec != 0)
    {
        buf.push_back(input);
        output = 0;
    }
    else if (read != 0)
    {

        if (buf.size()>0 && index < buf.size())
        {
            float delta = 0;
            float deltaValue = 0;
            float lValue =  0;
            float cValue = 0;

                lValue =  buf.at(index);


            if (index < buf.size() - 1)
            {
                cValue = buf.at(index + 1);
            }
            else
            {
                cValue = buf.at(0);
            }

                delta = rawIndex - index;
                if (cValue > lValue)
                {
                    deltaValue = cValue - lValue;
                    output = lValue + (delta * deltaValue) ;
                }
                else
                {
                    deltaValue = lValue - cValue;
                    output = lValue - (delta * deltaValue) ;
                }
        }
        else
        {
            output = 0;
        }
    }
    if (rec == 0)
    {
        wasOnRec = false;
    }
    if (read == 0)
    {
    output = 0;
    }
    return output;
}
int ControlledBuffer::getbufSize()
{
    return buf.size();
}
