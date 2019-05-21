#include "keyboard.hpp"
#include "midi.hpp"
#include "window.hpp"
#include <map>


namespace rack {
namespace keyboard {


struct Driver;


static const int DRIVER = -11;
static Driver *driver = NULL;

enum {
	CMD_OCTAVE_DOWN = -1,
	CMD_OCTAVE_UP = -2,
};

static const int deviceCount = 2;

static const std::vector<std::map<int, int>> deviceKeyNote = {
	{
		{GLFW_KEY_GRAVE_ACCENT, CMD_OCTAVE_DOWN},
		{GLFW_KEY_1, CMD_OCTAVE_UP},

		{GLFW_KEY_Z, 0},
		{GLFW_KEY_S, 1},
		{GLFW_KEY_X, 2},
		{GLFW_KEY_D, 3},
		{GLFW_KEY_C, 4},
		{GLFW_KEY_V, 5},
		{GLFW_KEY_G, 6},
		{GLFW_KEY_B, 7},
		{GLFW_KEY_H, 8},
		{GLFW_KEY_N, 9},
		{GLFW_KEY_J, 10},
		{GLFW_KEY_M, 11},
		{GLFW_KEY_COMMA, 12},
		{GLFW_KEY_L, 13},
		{GLFW_KEY_PERIOD, 14},
		{GLFW_KEY_SEMICOLON, 15},
		{GLFW_KEY_SLASH, 16},

		{GLFW_KEY_Q, 12},
		{GLFW_KEY_2, 13},
		{GLFW_KEY_W, 14},
		{GLFW_KEY_3, 15},
		{GLFW_KEY_E, 16},
		{GLFW_KEY_R, 17},
		{GLFW_KEY_5, 18},
		{GLFW_KEY_T, 19},
		{GLFW_KEY_6, 20},
		{GLFW_KEY_Y, 21},
		{GLFW_KEY_7, 22},
		{GLFW_KEY_U, 23},
		{GLFW_KEY_I, 24},
		{GLFW_KEY_9, 25},
		{GLFW_KEY_O, 26},
		{GLFW_KEY_0, 27},
		{GLFW_KEY_P, 28},
		{GLFW_KEY_LEFT_BRACKET, 29},
		{GLFW_KEY_EQUAL, 30},
		{GLFW_KEY_RIGHT_BRACKET, 31},
	},
	{
		{GLFW_KEY_KP_DIVIDE, CMD_OCTAVE_DOWN},
		{GLFW_KEY_KP_MULTIPLY, CMD_OCTAVE_UP},

		{GLFW_KEY_KP_0, 0},
		{GLFW_KEY_KP_DECIMAL, 2},
		{GLFW_KEY_KP_ENTER, 3},

		{GLFW_KEY_KP_1, 4},
		{GLFW_KEY_KP_2, 5},
		{GLFW_KEY_KP_3, 6},

		{GLFW_KEY_KP_4, 8},
		{GLFW_KEY_KP_5, 9},
		{GLFW_KEY_KP_6, 10},
		{GLFW_KEY_KP_ADD, 11},

		{GLFW_KEY_KP_7, 12},
		{GLFW_KEY_KP_8, 13},
		{GLFW_KEY_KP_9, 14},
	},
};


struct InputDevice : midi::InputDevice {
	int deviceId;
	int octave = 5;
	std::map<int, int> pressedNotes;

	void onKeyPress(int key) {
		// Do nothing if no ports are subscribed
		if (subscribed.empty())
			return;
		auto keyNote = deviceKeyNote[deviceId];
		auto it = keyNote.find(key);
		if (it == keyNote.end())
			return;
		int note = it->second;

		if (note < 0) {
			if (note == CMD_OCTAVE_DOWN)
				octave--;
			else if (note == CMD_OCTAVE_UP)
				octave++;
			octave = math::clamp(octave, 0, 9);
			return;
		}

		note += 12 * octave;
		if (note > 127)
			return;

		// MIDI note on
		midi::Message msg;
		msg.setStatus(0x9);
		msg.setNote(note);
		msg.setValue(127);
		onMessage(msg);

		pressedNotes[key] = note;
	}

	void onKeyRelease(int key) {
		// Do nothing if no ports are subscribed
		if (subscribed.empty())
			return;
		auto it = pressedNotes.find(key);
		if (it == pressedNotes.end())
			return;

		int note = it->second;
		// MIDI note off
		midi::Message msg;
		msg.setStatus(0x8);
		msg.setNote(note);
		msg.setValue(127);
		onMessage(msg);

		pressedNotes.erase(it);
	}
};


struct Driver : midi::Driver {
	InputDevice devices[deviceCount];

	Driver() {
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			devices[deviceId].deviceId = deviceId;
		}
		devices[1].octave = 3;
	}

	std::string getName() override {return "Computer keyboard";}

	std::vector<int> getInputDeviceIds() override {
		std::vector<int> deviceIds;
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			deviceIds.push_back(deviceId);
		}
		return deviceIds;
	}

	std::string getInputDeviceName(int deviceId) override {
		if (deviceId == 0)
			return "QWERTY keyboard (US)";
		else if (deviceId == 1)
			return "Numpad keyboard (US)";
		return "";
	}

	midi::InputDevice *subscribeInput(int deviceId, midi::Input *input) override {
		if (!(0 <= deviceId && deviceId < deviceCount))
			return NULL;
		devices[deviceId].subscribe(input);
		return &devices[deviceId];
	}

	void unsubscribeInput(int deviceId, midi::Input *input) override {
		if (!(0 <= deviceId && deviceId < deviceCount))
			return;
		devices[deviceId].unsubscribe(input);
	}

	void onKeyPress(int key) {
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			devices[deviceId].onKeyPress(key);
		}
	}

	void onKeyRelease(int key) {
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			devices[deviceId].onKeyRelease(key);
		}
	}
};


void init() {
	driver = new Driver;
	midi::addDriver(DRIVER, driver);
}

void press(int key) {
	if (!driver)
		return;
	driver->onKeyPress(key);
}

void release(int key) {
	if (!driver)
		return;
	driver->onKeyRelease(key);
}


} // namespace keyboard
} // namespace rack
