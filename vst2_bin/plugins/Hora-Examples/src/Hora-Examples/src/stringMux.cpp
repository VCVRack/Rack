#include "stringMux.h"

void stringMux::setstr1(char *_input)
{
    strcpy(input1, _input);
}
void stringMux::setstr2(char *_input)
{
    strcpy(input2, _input);
}
void stringMux::setstr3(char *_input)
{
    strcpy(input3, _input);
}
void stringMux::setstr4(char *_input)
{
    strcpy(input4, _input);
}
void stringMux::setstr5(char *_input)
{
    strcpy(input5, _input);
}
void stringMux::setindex(int _index)
{
    index = _index;
}

char * stringMux::getoutput()
{

    //char* output = (char*)malloc(100);
    if (index == 0)
    {
        return input1;
        //strcpy(output, input1);
    }
    else if (index == 1)
    {
        return input2;
        //strcpy(output, input2);
    }
    else if (index == 2)
    {
        return input3;
        //strcpy(output, input3);
    }
    else if (index == 3)
    {
        return input4;
        //strcpy(output, input4);
    }
    else if (index == 4)
    {
        return input5;
        //strcpy(output, input5);
    }
    else
    {
        strcpy(input5, "");
        return input5;
        //strcpy(output, "");
    }
    //free(output);
    //return output;
}
