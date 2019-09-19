#pragma once
#include <app/common.hpp>
#include <app/LedDisplay.hpp>
#include <audio.hpp>


namespace rack {
namespace app {


struct AudioWidget : LedDisplay {
	LedDisplayChoice* driverChoice;
	LedDisplaySeparator* driverSeparator;
	LedDisplayChoice* deviceChoice;
	LedDisplaySeparator* deviceSeparator;
	LedDisplayChoice* sampleRateChoice;
	LedDisplaySeparator* sampleRateSeparator;
	LedDisplayChoice* bufferSizeChoice;
	void setAudioPort(audio::Port* port);
};


} // namespace app
} // namespace rack
