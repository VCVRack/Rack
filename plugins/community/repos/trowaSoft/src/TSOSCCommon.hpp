#ifndef TSOSCCOMMON_HPP
#define TSOSCCOMMON_HPP

#include <string>


// What client are we talking to. touchOSC and Lemur are a little limited, so they need special treatment.
enum OSCClient : uint8_t {
	// Interface will closely match how the sequencer acts. Accepts multiple parameters.
	GenericClient,
	// touchOSC : Can only take 1 parameter. Only strings or floats as data types.
	// toggleGrid control in touchOSC is addressed by 1-based /row/col/ starting from THE BOTTOM. (ghey)
	touchOSCClient,
	// Lemur : Special treatment not implemented yet.
	//LemurClient,
	NUM_OSC_CLIENTS
};
// The OSC client labels/strings. Currently defined in ConfigWidget.
extern std::string OSCClientStr[NUM_OSC_CLIENTS];
// OSC Client abbreviations.
extern std::string OSCClientAbbr[NUM_OSC_CLIENTS];



//-------- Helpers -------------
namespace touchOSC
{
	// touchOSC color strings for our channels. In future, this should maybe live in sequencer file since this is for sequencers (if we do touchOSC for other types of modules).
	extern const char* ChannelColors[16];

	// Convert the 1-based row, col from a touchOSC multi-push/multi-toggle grid control to a 0-based step index.
	int mcRowCol_to_stepIndex(/*in*/ int row, /*in*/ int col, /*in*/ int numRows, /*in*/ int numCols);
	// Convert the 0-based step index to 1-based row, col from a touchOSC multi-push/multi-toggle grid control.
	void stepIndex_to_mcRowCol(/*in*/ int stepIx, /*in*/ int numRows, /*in*/ int numCols, /*out*/ int* row, /*out*/ int* col);
}





#endif // !TSOSCCOMMON_HPP
