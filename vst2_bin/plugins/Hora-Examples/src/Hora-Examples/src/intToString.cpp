#include "intToString.h"

void intToString::setinput(int _input)
{
    input = _input;
}
char * intToString::getoutput()
{
    int x = input;
    //char * output = (char*)malloc(100);
    sprintf(output, "%d", x);
    return output;
}
