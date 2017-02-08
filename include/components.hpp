#pragma once
#include "scene.hpp"


namespace rack {

////////////////////
// Knobs
////////////////////

struct KnobDavies1900h : SpriteKnob {
	KnobDavies1900h() {
		box.size = Vec(36, 36);
		spriteOffset = Vec(-2, -2);
		spriteSize = Vec(42, 42);
		minIndex = 44;
		maxIndex = -46;
		spriteCount = 120;
	}
};

struct KnobDavies1900hWhite : KnobDavies1900h {
	KnobDavies1900hWhite() {
		spriteImage = Image::load("res/ComponentLibrary/Davies1900hWhite.png");
	}
};

struct KnobDavies1900hBlack : KnobDavies1900h {
	KnobDavies1900hBlack() {
		spriteImage = Image::load("res/ComponentLibrary/Davies1900hBlack.png");
	}
};

struct KnobDavies1900hRed : KnobDavies1900h {
	KnobDavies1900hRed() {
		spriteImage = Image::load("res/ComponentLibrary/Davies1900hRed.png");
	}
};

struct BefacoBigKnob : SpriteKnob {
	BefacoBigKnob() {
		box.size = Vec(75, 75);
		spriteOffset = Vec(-2, -2);
		spriteSize = Vec(81, 81);
		minIndex = 44;
		maxIndex = -46;
		spriteCount = 120;
		spriteImage = Image::load("res/ComponentLibrary/BefacoBigKnob.png");
	}
};

struct BefacoTinyKnob : SpriteKnob {
	BefacoTinyKnob() {
		box.size = Vec(26, 26);
		spriteOffset = Vec(-2, -2);
		spriteSize = Vec(32, 32);
		minIndex = 44;
		maxIndex = -46;
		spriteCount = 120;
		spriteImage = Image::load("res/ComponentLibrary/BefacoTinyKnob.png");
	}
};

struct BefacoSlidePot : SpriteKnob {
	BefacoSlidePot() {
		box.size = Vec(12, 122);
		spriteOffset = Vec(-2, -6);
		spriteSize = Vec(18, 134);
		minIndex = 97;
		maxIndex = 0;
		spriteCount = 98;
		spriteImage = Image::load("res/ComponentLibrary/BefacoSlidePot.png");
	}
};

////////////////////
// Jacks
////////////////////

struct PJ301M : SpriteWidget {
	PJ301M() {
		box.size = Vec(24, 24);
		spriteOffset = Vec(-2, -2);
		spriteSize = Vec(30, 30);
		spriteImage = Image::load("res/ComponentLibrary/PJ301M.png");
	}
};
struct InputPortPJ301M : InputPort, PJ301M {};
struct OutputPortPJ301M: OutputPort, PJ301M {};

struct PJ3410 : SpriteWidget {
	PJ3410() {
		box.size = Vec(32, 32);
		spriteOffset = Vec(-1, -1);
		spriteSize = Vec(36, 36);
		spriteImage = Image::load("res/ComponentLibrary/PJ3410.png");
	}
};
struct InputPortPJ3410 : InputPort, PJ3410 {};
struct OutputPortPJ3410: OutputPort, PJ3410 {};

struct CL1362 : SpriteWidget {
	CL1362() {
		box.size = Vec(33, 29);
		spriteOffset = Vec(-2, -2);
		spriteSize = Vec(39, 36);
		spriteImage = Image::load("res/ComponentLibrary/CL1362.png");
	}
};
struct InputPortCL1362 : InputPort, CL1362 {};
struct OutputPortCL1362 : OutputPort, CL1362 {};

////////////////////
// Misc
////////////////////

/** If you don't add these to your ModuleWidget, it will fall out of the rack... */
struct Screw : SpriteWidget {
	Screw() {
		box.size = Vec(15, 14);
		spriteOffset = Vec(0, 0);
		spriteSize = Vec(15, 14);
	}
};

struct BlackScrew : Screw {
	BlackScrew() {
		spriteImage = Image::load("res/ComponentLibrary/ScrewBlack.png");
	}
};

struct SilverScrew : Screw {
	SilverScrew() {
		spriteImage = Image::load("res/ComponentLibrary/ScrewSilver.png");
	}
};

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
