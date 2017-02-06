#pragma once
#include "scene.hpp"


namespace rack {

////////////////////
// knobs
////////////////////

struct KnobDavies1900h : SpriteKnob {
	KnobDavies1900h() {
		// box.size = Vec(36, 36);
		// spriteOffset = Vec(-8, -8);
		// spriteSize = Vec(64, 64);
		// minIndex = 44;
		// maxIndex = -46;
		// spriteCount = 120;
		box.size = Vec(42, 42);
		spriteOffset = Vec(-9, -9);
		spriteSize = Vec(60, 60);
		minIndex = 0;
		maxIndex = 119;
		spriteCount = 120;
		spriteImage = Image::load("res/Black Plastic small 01.png");
	}
};

struct KnobDavies1900hWhite : KnobDavies1900h {
	KnobDavies1900hWhite() {
		// spriteImage = Image::load("res/ComponentLibrary/Davies1900hWhite.png");
	}
};

struct KnobDavies1900hBlack : KnobDavies1900h {
	KnobDavies1900hBlack() {
		// spriteImage = Image::load("res/ComponentLibrary/Davies1900hBlack.png");
	}
};

struct KnobDavies1900hRed : KnobDavies1900h {
	KnobDavies1900hRed() {
		// spriteImage = Image::load("res/ComponentLibrary/Davies1900hRed.png");
	}
};

////////////////////
// ports
////////////////////

struct PJ301M : SpriteWidget {
	PJ301M() {
		// box.size = Vec(24, 24);
		// spriteOffset = Vec(-10, -10);
		// spriteSize = Vec(48, 48);
		// spriteImage = Image::load("res/ComponentLibrary/PJ301M.png");
		box.size = Vec(26, 26);
		spriteOffset = Vec(-16, -16);
		spriteSize = Vec(56, 56);
		spriteImage = Image::load("res/port.png");
	}
};
struct InputPortPJ301M : InputPort, PJ301M {};
struct OutputPortPJ301M: OutputPort, PJ301M {};

struct PJ3410 : SpriteWidget {
	PJ3410() {
		// box.size = Vec(31, 31);
		// spriteOffset = Vec(-9, -9);
		// spriteSize = Vec(54, 54);
		// spriteImage = Image::load("res/ComponentLibrary/PJ3410.png");
		box.size = Vec(26, 26);
		spriteOffset = Vec(-12, -12);
		spriteSize = Vec(56, 56);
		spriteImage = Image::load("res/port.png");
	}
};
struct InputPortPJ3410 : InputPort, PJ3410 {};
struct OutputPortPJ3410: OutputPort, PJ3410 {};

struct CL1362 : SpriteWidget {
	CL1362() {
		// box.size = Vec(33, 29);
		// spriteOffset = Vec(-10, -10);
		// spriteSize = Vec(57, 54);
		// spriteImage = Image::load("res/ComponentLibrary/CL1362.png");
		box.size = Vec(26, 26);
		spriteOffset = Vec(-12, -12);
		spriteSize = Vec(56, 56);
		spriteImage = Image::load("res/port.png");

	}
};
struct InputPortCL1362 : InputPort, CL1362 {};
struct OutputPortCL1362 : OutputPort, CL1362 {};


////////////////////
// panels
////////////////////

struct LightPanel : Panel {
	LightPanel() {
		backgroundColor = nvgRGB(0xe8, 0xe8, 0xe8);
		borderColor = nvgRGB(0xac, 0xac, 0xac);
	}
};

struct DarkPanel : Panel {
	DarkPanel() {
		backgroundColor = nvgRGB(0x0f, 0x0f, 0x0f);
		borderColor = nvgRGB(0x5e, 0x5e, 0x5e);
	}
};


} // namespace rack
