#pragma once
#include "common.hpp"
#include "LedDisplay.hpp"


namespace rack {


struct AudioIO;


struct AudioWidget : LedDisplay {
	/** Not owned */
	AudioIO *audioIO = NULL;
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *sampleRateChoice;
	LedDisplaySeparator *sampleRateSeparator;
	LedDisplayChoice *bufferSizeChoice;
	AudioWidget();
	void step() override;
};


} // namespace rack
