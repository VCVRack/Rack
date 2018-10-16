#pragma once
#include "app/common.hpp"
#include "app/LedDisplay.hpp"


namespace rack {


struct MidiIO;


struct MidiWidget : LedDisplay {
	/** Not owned */
	MidiIO *midiIO = NULL;
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *channelChoice;
	MidiWidget();
	void step() override;
};


} // namespace rack
