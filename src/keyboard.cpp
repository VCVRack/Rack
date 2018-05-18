#include "keyboard.hpp"
#include <GLFW/glfw3.h>


namespace rack {


void KeyboardInputDevice::processKey(int key, bool released) {
	int note = -1;
	switch (key) {
		case GLFW_KEY_Z: note = 0; break;
		case GLFW_KEY_S: note = 1; break;
		case GLFW_KEY_X: note = 2; break;
		case GLFW_KEY_D: note = 3; break;
		case GLFW_KEY_C: note = 4; break;
		case GLFW_KEY_V: note = 5; break;
		case GLFW_KEY_G: note = 6; break;
		case GLFW_KEY_B: note = 7; break;
		case GLFW_KEY_H: note = 8; break;
		case GLFW_KEY_N: note = 9; break;
		case GLFW_KEY_J: note = 10; break;
		case GLFW_KEY_M: note = 11; break;
		case GLFW_KEY_COMMA: note = 12; break;
		case GLFW_KEY_L: note = 13; break;
		case GLFW_KEY_PERIOD: note = 14; break;
		case GLFW_KEY_SEMICOLON: note = 15; break;
		case GLFW_KEY_SLASH: note = 16; break;

		case GLFW_KEY_Q: note = 12; break;
		case GLFW_KEY_2: note = 13; break;
		case GLFW_KEY_W: note = 14; break;
		case GLFW_KEY_3: note = 15; break;
		case GLFW_KEY_E: note = 16; break;
		case GLFW_KEY_R: note = 17; break;
		case GLFW_KEY_5: note = 18; break;
		case GLFW_KEY_T: note = 19; break;
		case GLFW_KEY_6: note = 20; break;
		case GLFW_KEY_Y: note = 21; break;
		case GLFW_KEY_7: note = 22; break;
		case GLFW_KEY_U: note = 23; break;
		case GLFW_KEY_I: note = 24; break;
		case GLFW_KEY_9: note = 25; break;
		case GLFW_KEY_O: note = 26; break;
		case GLFW_KEY_0: note = 27; break;
		case GLFW_KEY_P: note = 28; break;
		case GLFW_KEY_LEFT_BRACKET: note = 29; break;
		case GLFW_KEY_EQUAL: note = 30; break;
		case GLFW_KEY_RIGHT_BRACKET: note = 31; break;

		case GLFW_KEY_GRAVE_ACCENT: {
			if (!released)
				octave--;
		} break;
		case GLFW_KEY_1: {
			if (!released)
				octave++;
		} break;
		default: break;
	}

	octave = clamp(octave, 0, 9);
	if (note < 0)
		return;
	note += 12 * octave;

	if (note > 127)
		return;

	MidiMessage msg;
	msg.cmd = ((!released ? 0x9 : 0x8) << 4);
	msg.data1 = note;
	msg.data2 = 127;
	onMessage(msg);
}


std::vector<int> KeyboardInputDriver::getDeviceIds() {
	return {0};
}

std::string KeyboardInputDriver::getDeviceName(int deviceId) {
	if (deviceId == 0)
		return "QWERTY keyboard (US)";
	return "";
}

MidiInputDevice *KeyboardInputDriver::getDevice(int deviceId) {
	return &device;
}


static KeyboardInputDriver *driver = NULL;

void keyboardPress(int key) {
	if (!driver)
		return;
	driver->device.processKey(key, false);
}

void keyboardRelease(int key) {
	if (!driver)
		return;
	driver->device.processKey(key, true);
}

KeyboardInputDriver *keyboardGetInputDriver() {
	// Lazily create driver
	if (!driver) {
		driver = new KeyboardInputDriver();
	}
	return driver;
}


} // namespace rack
