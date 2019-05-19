
#include <math.h>
#include <string.h>
#include <stdio.h>
class intToString {
private:
	int input;
	char output[100];
public:
    intToString() :
		input(0),
		output("")
		{

		};
        void setinput(int _input);
        char * getoutput();
};

