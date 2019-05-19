
#include <math.h>
#include <string.h>
#include <stdio.h>

class stringMux {
private:
	char input1[100];
	char input2[100];
	char input3[100];
	char input4[100];
	char input5[100];
        int index;

public:
    stringMux() :
		input1(""),
		input2(""),
		input3(""),
		input4(""),
                input5(""),
                index(0)
		{

		};
        void setstr1(char *_input);
        void setstr2(char *_input);
        void setstr3(char *_input);
        void setstr4(char *_input);
        void setstr5(char *_input);
        void setindex(int _index);
        char * getoutput();
};

