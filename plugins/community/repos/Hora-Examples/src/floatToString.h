
#include <math.h>
#include <string.h>
#include <stdio.h>
class floatToString {
private:
	float input;
	char output[100];
public:
    floatToString() :
		input(0),
		output("")
		{

		};
        void setinput(float _input);
        char * getoutput();
};

