#pragma once
#include "app/common.hpp"
#include "app/LedDisplay.hpp"


namespace rack {


namespace audio {
	struct IO;
}


namespace app {


struct AudioWidget : LedDisplay {
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *sampleRateChoice;
	LedDisplaySeparator *sampleRateSeparator;
	LedDisplayChoice *bufferSizeChoice;
	void setAudioIO(audio::IO *audioIO);
};


} // namespace app
} // namespace rack
