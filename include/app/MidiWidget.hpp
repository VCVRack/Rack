#pragma once
#include "app/common.hpp"
#include "app/LedDisplay.hpp"


namespace rack {


namespace midi {
	struct IO;
}


struct MidiWidget : LedDisplay {
	/** Not owned */
	midi::IO *midiIO = NULL;
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *channelChoice;
	MidiWidget();
	void step() override;
};


} // namespace rack
