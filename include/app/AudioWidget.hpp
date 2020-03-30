#pragma once
#include <app/common.hpp>
#include <app/LedDisplay.hpp>
#include <ui/Menu.hpp>
#include <audio.hpp>


namespace rack {
namespace app {


/** Designed for Audio-8 and Audio-16 module. */
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


/** Designed for Audio-2 module. */
struct AudioDeviceWidget : LedDisplay {
	LedDisplayChoice* deviceChoice;
	void setAudioPort(audio::Port* port);
};


/** Appends menu items to the given menu with driver, device, etc.
Useful alternative to putting an AudioWidget on your module's panel.
*/
void appendAudioMenu(ui::Menu* menu, audio::Port* port);


} // namespace app
} // namespace rack
