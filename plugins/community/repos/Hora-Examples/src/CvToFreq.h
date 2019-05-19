
#include <math.h>

class CvToFreq {
private:
	float input;
	float base;
	float output;
public:
    CvToFreq() :
		input(0),
                base(0),
		output(0)
		{

		};
        void setinput(float _input);
        void setbase(float _base);
        float getoutput();
};

