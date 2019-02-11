#pragma once
#include "app/common.hpp"
#include "app/LedDisplay.hpp"


namespace rack {


namespace midi {
	struct Port;
}


namespace app {


struct MidiWidget : LedDisplay {
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *channelChoice;
	void setMidiPort(midi::Port *port);
};


} // namespace app
} // namespace rack
