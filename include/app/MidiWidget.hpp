#pragma once
#include "common.hpp"
#include "LedDisplay.hpp"


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
