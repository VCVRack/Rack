#include "TSOSCCommon.hpp"
#include <string>


// The OSC client labels/strings. 
std::string OSCClientStr[NUM_OSC_CLIENTS] = { "Generic", "touchOSC" };// , "Lemur" };
std::string OSCClientAbbr[NUM_OSC_CLIENTS] = { "Gen", "tOSC" };// , "Lemr" };


namespace touchOSC
{
	// touchOSC color strings for our channels.
	// touchOSC brown or gray is like our Pink
	// touchOSC blue is like our cyan
	// touchOSC pink is like our purple kinda
	const char* ChannelColors[16] = {
		"red", // COLOR_TS_RED
		"orange", // COLOR_DARK_ORANGE
		"yellow", // COLOR_YELLOW
		"green", // COLOR_TS_GREEN
		"blue", // COLOR_CYAN
		"purple", // COLOR_TS_BLUE
		"pink", // COLOR_PURPLE
		"gray", // COLOR_PINK

		"red", // COLOR_TS_RED
		"orange", // COLOR_DARK_ORANGE
		"yellow", // COLOR_YELLOW
		"green", // COLOR_TS_GREEN
		"blue", // COLOR_CYAN
		"purple", // COLOR_TS_BLUE
		"pink", // COLOR_PURPLE
		"gray" // COLOR_PINK
	};

	// Convert the 1-based row, col from a touchOSC multi-push/multi-toggle grid control to a 0-based step index.
	int mcRowCol_to_stepIndex(/*in*/ int row, /*in*/ int col, /*in*/ int numRows, /*in*/ int numCols)
	{
		// In touchOSC, the rows are flipped (i.e. the top row is numRows and the bottom row is 1)
		// touchOSC row starts at 1, col starts at 1
		return (numRows - row) * numCols + (col - 1);
	}
	// Convert the 0-based step index to 1-based row, col from a touchOSC multi-push/multi-toggle grid control.
	void stepIndex_to_mcRowCol(/*in*/ int stepIx, /*in*/ int numRows, /*in*/ int numCols, /*out*/ int* row, /*out*/ int* col)
	{
		// In touchOSC, the rows are flipped (i.e. the top row is numRows and the bottom row is 1)
		// touchOSC row starts at 1, col starts at 1
		*row = numRows - stepIx / numCols; // Should yield 1 to numRows
		*col = stepIx % numCols + 1;  // Should yield 1 to numCols
		return;
	}
}
