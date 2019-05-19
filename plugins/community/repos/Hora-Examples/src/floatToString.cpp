#include "floatToString.h"

void floatToString::setinput(float _input)
{
    input = _input;
}
char * floatToString::getoutput()
{
    float x = input;
    //char * output = (char*)malloc(100);
    sprintf(output,"%f",x);
    return output;
}
