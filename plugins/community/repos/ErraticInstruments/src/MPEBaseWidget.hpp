#pragma once
#include <vector>
#include <algorithm>
// #include <jansson.h>
#include "widgets.hpp"
#include "ui.hpp"

// namespace rack {

struct MPEBaseWidget : rack::LedDisplay {
	/** Not owned */
	rack::MidiIO *midiIO = NULL;
	rack::LedDisplayChoice *driverChoice;
	rack::LedDisplaySeparator *driverSeparator;
	rack::LedDisplayChoice *deviceChoice;
	// rack::LedDisplaySeparator *deviceSeparator;
	// rack::LedDisplayChoice *channelChoice;
	MPEBaseWidget();
	void step() override;
};

