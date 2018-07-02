
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <algorithm>

#include "util/math.hpp" // Rack

#include "dsp/fixed.hpp"

using namespace bogaudio::dsp;

int main() {
	{
		fixed_16_16 x = 5;
		fixed_16_16 y = 1;
		x = 3;
		printf("X=%d\n", (int)(x + y));
		y = 2;
		printf("X=%d\n", (int)(x - y));
		x = y + 5;
		printf("X=%d\n", (int)x);
		x = y - 3;
		printf("X=%d\n", (int)x);

		x += 2.5;
		printf("X=%d\n", (int)x);
		printf("X=%f\n", (float)x);

		x = y - 0.3;
		printf("X=%d\n", (int)x);
		printf("X=%f\n", (float)x);
	}

	{
		fixed_32_32 x = 5;
		fixed_32_32 y = 1;
		x = 3;
		printf("X=%d\n", (int)(x + y));
		y = 2;
		printf("X=%d\n", (int)(x - y));
		x = y + 5;
		printf("X=%d\n", (int)x);
		x = y - 3;
		printf("X=%d\n", (int)x);

		x += 2.5;
		printf("X=%d\n", (int)x);
		printf("X=%f\n", (float)x);

		x = y - 0.3;
		printf("X=%d\n", (int)x);
		printf("X=%f\n", (float)x);
	}

	return 0;
}
