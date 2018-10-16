#pragma once
#include "app/common.hpp"
#include "app/LedDisplay.hpp"


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
