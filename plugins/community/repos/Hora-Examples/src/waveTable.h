
#include <math.h>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <deque>
#include <vector>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "rack.hpp"


using namespace rack;
using namespace std;

class waveTable {
private:


        int index;
        float rawIndex;
        float reset;
	float output;
        std::vector<float> buf;
        char tableName[100];
        bool wasSet;
        std::vector<std::string> words;
public:
    waveTable() :


                index(0),
                rawIndex(0),
                reset(0),
                output(0),
                buf(),
                tableName(""),
                wasSet(false),
                words()
		{

		};
        void settableName(char *name);
        void setreset(float _reset);
        void setindex(float _index);
        float getoutput();
        int getbufSize();
};

