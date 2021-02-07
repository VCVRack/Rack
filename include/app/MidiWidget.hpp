#pragma once
#include <app/common.hpp>
#include <app/LedDisplay.hpp>
#include <ui/Menu.hpp>
#include <app/SvgButton.hpp>
#include <midi.hpp>


namespace rack {
namespace app {


struct MidiWidget : LedDisplay {
	LedDisplayChoice* driverChoice;
	LedDisplaySeparator* driverSeparator;
	LedDisplayChoice* deviceChoice;
	LedDisplaySeparator* deviceSeparator;
	LedDisplayChoice* channelChoice;
	void setMidiPort(midi::Port* port);
};


/** A virtual MIDI port graphic that displays an MIDI menu when clicked. */
struct MidiButton : SvgButton {
	midi::Port* port;
	void setMidiPort(midi::Port* port);
	void onAction(const ActionEvent& e) override;
};


/** Appends menu items to the given menu with driver, device, etc.
Useful alternative to putting a MidiWidget on your module's panel.
*/
void appendMidiMenu(ui::Menu* menu, midi::Port* port);


} // namespace app
} // namespace rack
