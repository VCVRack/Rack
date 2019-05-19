
#include <math.h>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <deque>
#include <vector>

class clockedCounter {
private:
        int length;
        float rate;
        float count;
        float reset;
	float output;
        std::vector<float> buf;
public:
    clockedCounter() :
                length(0),
                rate(0),
                count(0),
                reset(0),
                output(0)
		{

		};
        void setlength(int _length);
        void setrate(float _rate);
        void setreset(float _reset);
        float getoutput();
};

