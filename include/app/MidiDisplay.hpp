#pragma once
#include <app/common.hpp>
#include <app/LedDisplay.hpp>
#include <ui/Menu.hpp>
#include <app/SvgButton.hpp>
#include <midi.hpp>


namespace rack {
namespace app {


struct MidiDriverChoice : LedDisplayChoice {
	midi::Port* port;
	void onAction(const ActionEvent& e) override;
	void step() override;
};


struct MidiDeviceChoice : LedDisplayChoice {
	midi::Port* port;
	void onAction(const ActionEvent& e) override;
	void step() override;
};


struct MidiChannelChoice : LedDisplayChoice {
	midi::Port* port;
	void onAction(const ActionEvent& e) override;
	void step() override;
};


struct MidiDisplay : LedDisplay {
	MidiDriverChoice* driverChoice;
	LedDisplaySeparator* driverSeparator;
	MidiDeviceChoice* deviceChoice;
	LedDisplaySeparator* deviceSeparator;
	MidiChannelChoice* channelChoice;
	void setMidiPort(midi::Port* port);
};


/** A virtual MIDI port graphic that displays an MIDI menu when clicked. */
struct MidiButton : SvgButton {
	midi::Port* port;
	void setMidiPort(midi::Port* port);
	void onAction(const ActionEvent& e) override;
};


/** Appends menu items to the given menu with driver, device, etc.
Useful alternative to putting a MidiDisplay on your module's panel.
*/
void appendMidiMenu(ui::Menu* menu, midi::Port* port);


} // namespace app
} // namespace rack
