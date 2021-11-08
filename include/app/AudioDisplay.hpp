#pragma once
#include <app/common.hpp>
#include <app/LedDisplay.hpp>
#include <ui/Menu.hpp>
#include <app/SvgButton.hpp>
#include <audio.hpp>


namespace rack {
namespace app {


struct AudioDriverChoice : LedDisplayChoice {
	audio::Port* port;
	void onAction(const ActionEvent& e) override;
	void step() override;
};


struct AudioDeviceChoice : LedDisplayChoice {
	audio::Port* port;
	void onAction(const ActionEvent& e) override;
	void step() override;
};


struct AudioSampleRateChoice : LedDisplayChoice {
	audio::Port* port;
	void onAction(const ActionEvent& e) override;
	void step() override;
};


struct AudioBlockSizeChoice : LedDisplayChoice {
	audio::Port* port;
	void onAction(const ActionEvent& e) override;
	void step() override;
};


struct AudioDeviceMenuChoice : AudioDeviceChoice {
	void onAction(const ActionEvent& e) override;
};


/** Designed for Audio-8 and Audio-16 module. */
struct AudioDisplay : LedDisplay {
	AudioDriverChoice* driverChoice;
	LedDisplaySeparator* driverSeparator;
	AudioDeviceChoice* deviceChoice;
	LedDisplaySeparator* deviceSeparator;
	AudioSampleRateChoice* sampleRateChoice;
	LedDisplaySeparator* sampleRateSeparator;
	AudioBlockSizeChoice* bufferSizeChoice;
	void setAudioPort(audio::Port* port);
};


/** A virtual audio port graphic that displays an audio menu when clicked. */
struct AudioButton : SvgButton {
	audio::Port* port;
	void setAudioPort(audio::Port* port);
	void onAction(const ActionEvent& e) override;
};


/** Appends menu items to the given menu with driver, device, etc.
Useful alternative to putting an AudioDisplay on your module's panel.
*/
void appendAudioMenu(ui::Menu* menu, audio::Port* port);


} // namespace app
} // namespace rack
