#include <map>

#include <keyboard.hpp>
#include <midi.hpp>
#include <window/Window.hpp>


namespace rack {
namespace keyboard {


struct Driver;


static const int DRIVER = -11;
static Driver* driver = NULL;
static const int MOUSE_DEVICE_ID = 1000;

enum {
	CMD_OCTAVE_DOWN = -1,
	CMD_OCTAVE_UP = -2,
};

struct DeviceInfo {
	std::string name;
	std::map<int, int> keyMap;
};

static const int deviceCount = 2;

static const std::vector<DeviceInfo> deviceInfos = {
	{
		"QWERTY keyboard (US)",
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
	},
	{
		"Numpad keyboard (US)",
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
	}
};


struct InputDevice : midi::InputDevice {
	int deviceId;
	int octave = 5;
	std::map<int, int> pressedNotes;

	void setDeviceId(int deviceId) {
		this->deviceId = deviceId;
		// Default lowest key of numpad is C1.
		if (deviceId == 1) {
			octave = 3;
		}
	}

	std::string getName() override {
		return deviceInfos[deviceId].name;
	}

	void onKeyPress(int key) {
		// Do nothing if no ports are subscribed
		if (subscribed.empty())
			return;
		const auto& keyMap = deviceInfos[deviceId].keyMap;
		auto it = keyMap.find(key);
		if (it == keyMap.end())
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


struct MouseInputDevice : midi::InputDevice {
	int16_t lastValues[2] = {};

	std::string getName() override {
		return "Mouse";
	}

	void onMouseMove(math::Vec pos) {
		int16_t values[2];
		values[0] = math::clamp((int) std::round(pos.x * 0x3f80), 0, 0x3f80);
		// Flip Y values
		values[1] = math::clamp((int) std::round((1.f - pos.y) * 0x3f80), 0, 0x3f80);

		for (int id = 0; id < 2; id++) {
			if (values[id] != lastValues[id]) {
				// Continuous controller MSB
				midi::Message m;
				m.setStatus(0xb);
				m.setNote(id);
				m.setValue(values[id] >> 7);
				onMessage(m);
				// Continuous controller LSB
				midi::Message m2;
				m2.setStatus(0xb);
				m2.setNote(id + 32);
				m2.setValue(values[id] & 0x7f);
				onMessage(m2);
				lastValues[id] = values[id];
			}
		}
	}
};


struct Driver : midi::Driver {
	InputDevice devices[deviceCount];
	MouseInputDevice mouseDevice;

	Driver() {
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			devices[deviceId].setDeviceId(deviceId);
		}
	}

	std::string getName() override {
		return "Computer keyboard/mouse";
	}

	std::vector<int> getInputDeviceIds() override {
		std::vector<int> deviceIds;
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			deviceIds.push_back(deviceId);
		}
		deviceIds.push_back(MOUSE_DEVICE_ID);
		return deviceIds;
	}

	int getDefaultInputDeviceId() override {
		// QWERTY keyboard device
		return 0;
	}

	midi::InputDevice* getInputDevice(int deviceId) {
		if (deviceId == MOUSE_DEVICE_ID)
			return &mouseDevice;
		if (0 <= deviceId && deviceId < deviceCount)
			return &devices[deviceId];
		return NULL;
	}

	std::string getInputDeviceName(int deviceId) override {
		midi::InputDevice* inputDevice = getInputDevice(deviceId);
		if (!inputDevice)
			return "";
		return inputDevice->getName();
	}

	midi::InputDevice* subscribeInput(int deviceId, midi::Input* input) override {
		midi::InputDevice* inputDevice = getInputDevice(deviceId);
		if (!inputDevice)
			return NULL;
		inputDevice->subscribe(input);
		return inputDevice;
	}

	void unsubscribeInput(int deviceId, midi::Input* input) override {
		midi::InputDevice* inputDevice = getInputDevice(deviceId);
		if (!inputDevice)
			return;
		inputDevice->unsubscribe(input);
	}

	// Events that forward to InputDevices

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

	void onMouseMove(math::Vec pos) {
		mouseDevice.onMouseMove(pos);
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


void mouseMove(math::Vec pos) {
	if (!driver)
		return;
	driver->onMouseMove(pos);
}


} // namespace keyboard
} // namespace rack
