#include "trowaSoftUtilities.hpp"

const char * TROWA_NOTES[TROWA_SEQ_NUM_NOTES] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Split a string
std::vector<std::string> str_split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}


namespace trowaSoft
{
	void TSColorToHSL(NVGcolor color, TSColorHSL* hsv)
	{
		float min;
		float max;
		min = (color.r < color.g) ? ((color.r < color.b) ? color.r : color.b) : ((color.g < color.b) ? color.g : color.b);
		max = (color.r > color.g) ? ((color.r > color.b) ? color.r : color.b) : ((color.g > color.b) ? color.g : color.b);
		float delta = max - min;
		//HSL: 0.7, 0.5, 0.3
		//RGB 0.21, 0.15, 0.45
		// min = 0.15
		// max = 0.45
		// delta = 0.30

		// Hue:
		// r-b/delta/6.0 + 2.0/3.0 = 
		// (0.21-0.15)/0.30/6.0 + 2.0/3.0 = 0.244

		// Sat:
		// delta/max = 0.30/0.45 = 0.667 --> NO

		// 0.7, 0.5, 0.3 --> 0.7, 0.67, 0.46

		hsv->lum = (max+min)/2.0;
		if (max == 0.0 || delta < 0.0001)
		{
			hsv->s = 0;
			hsv->h = 0;
		}
		else {
			if (hsv->lum < 0.5)
				hsv->s = delta / (max + min);
			else
				hsv->s = delta / (2 - max - min);

			if (max == color.r) {
				float tmp = ((color.g - color.b) / delta) / 6.0;
				hsv->h = (tmp - (int)tmp) / 6.0;
			}
			else if (max == color.g) {
				hsv->h = (color.b - color.r) / delta / 6.0 + 1.0/3.0;
			}
			else {
				hsv->h = (color.r - color.g) / delta / 6.0 + 2.0 / 3.0;
			}
			if (hsv->h < 0)
				hsv->h += 1.0;
		}
		return;
	}
}

