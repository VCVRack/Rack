#pragma once
#include "app/common.hpp"
#include "app/LedDisplay.hpp"


namespace rack {


namespace midi {
	struct IO;
}


namespace app {


struct MidiWidget : LedDisplay {
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *channelChoice;
	void setMidiIO(midi::IO *midiIO);
};


} // namespace app
} // namespace rack
