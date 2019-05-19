
#include <math.h>

class FreqToRate {
private:
	float input;
	float base;
        float tableLength;
	float output;
public:
    FreqToRate() :
		input(0),
                base(0),
                tableLength(0),
		output(0)
		{

		};
        void setinput(float _input);
        void setsampleRate(float _base);
        void settableLength(float _length);
        float getoutput();
};

