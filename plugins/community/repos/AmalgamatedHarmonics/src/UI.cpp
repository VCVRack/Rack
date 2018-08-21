#include "UI.hpp"

namespace rack_plugin_AmalgamatedHarmonics {

void UI::calculateKeyboard(int inKey, float spacing, float xOff, float yOff, float *x, float *y, int *scale) {

	// Sanitise input
	int key = inKey % 12;

	// White
	int whiteO [7] = {0, 1, 2, 3, 4, 5, 6};		// 'spaces' occupied by keys
	int whiteK [7] = {0, 2, 4, 5, 7, 9, 11};	// scale mapped to key
	// Black
	int blackO [5] = {0, 1, 3, 4, 5};			// 'spaces' occupied by keys
	int blackK [5] = {1, 3, 6, 8, 10};			// scale mapped to keys

	float bOffset = xOff + (spacing / 2);
	float bSpace = spacing;
	// Make an equilateral triangle pattern, tri height = base * (sqrt(3) / 2)
	float bDelta = bSpace * sqrt(3) * 0.5;

	// Are we black or white key?, check black keys
	for (int i = 0; i < 5; i++) {
		if (blackK[i] == key) {
			*x = bOffset + bSpace * blackO[i];
			*y = yOff - bDelta;
			*scale = blackK[i];
			return;
		}
	}

	int wOffset = xOff;
	int wSpace = spacing;
	
	for (int i = 0; i < 7; i++) {
		if (whiteK[i] == key) {
			*x = wOffset + wSpace * whiteO[i];
			*y = yOff;
			*scale = whiteK[i];
			return;
		}
	}
}

Vec UI::getPosition(int type, int xSlot, int ySlot, bool xDense, bool yDense) {

	float *xArray;
	float *yArray;
	
	switch(type) {
		case KNOB: 
		if (xDense) { xArray = X_KNOB_COMPACT; } else { xArray = X_KNOB; }
		if (yDense) { yArray = Y_KNOB_COMPACT; } else { yArray = Y_KNOB; }
		break;
		case PORT: 
		if (xDense) { xArray = X_PORT_COMPACT; } else { xArray = X_PORT; }
		if (yDense) { yArray = Y_PORT_COMPACT; } else { yArray = Y_PORT; }
		break;
		case BUTTON:
		if (xDense) { xArray = X_BUTTON_COMPACT; } else { xArray = X_BUTTON; }
		if (yDense) { yArray = Y_BUTTON_COMPACT; } else { yArray = Y_BUTTON; }
		break;
		case LIGHT: 
		if (xDense) { xArray = X_LIGHT_COMPACT; } else { xArray = X_LIGHT; }
		if (yDense) { yArray = Y_LIGHT_COMPACT; } else { yArray = Y_LIGHT; }
		break;
		case TRIMPOT: 
		if (xDense) { xArray = X_TRIMPOT_COMPACT; } else { xArray = X_TRIMPOT; }
		if (yDense) { yArray = Y_TRIMPOT_COMPACT; } else { yArray = Y_TRIMPOT; }
		break;
		default:
		if (xDense) { xArray = X_KNOB_COMPACT; } else { xArray = X_KNOB; }
		if (yDense) { yArray = Y_KNOB_COMPACT; } else { yArray = Y_KNOB; }
	}

	return Vec(xArray[0] + xArray[1] * xSlot, yArray[0] + yArray[1] * ySlot);
	
}

} // namespace rack_plugin_AmalgamatedHarmonics
