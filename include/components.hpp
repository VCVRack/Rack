#pragma once
#include "scene.hpp"


namespace rack {

#define SCHEME_BLACK nvgRGB(0x00, 0x00, 0x00)
#define SCHEME_WHITE nvgRGB(0xff, 0xff, 0xff)
#define SCHEME_RED nvgRGB(0xed, 0x2c, 0x24)
#define SCHEME_ORANGE nvgRGB(0xf2, 0xb1, 0x20)
#define SCHEME_YELLOW nvgRGB(0xf9, 0xdf, 0x1c)
#define SCHEME_GREEN nvgRGB(0x90, 0xc7, 0x3e)
#define SCHEME_CYAN nvgRGB(0x22, 0xe6, 0xef)
#define SCHEME_BLUE nvgRGB(0x29, 0xb2, 0xef)
#define SCHEME_PURPLE nvgRGB(0xd5, 0x2b, 0xed)

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

template <typename BASE>
struct PJ301M : BASE {
	PJ301M() {
		this->box.size = Vec(24, 24);
		this->spriteOffset = Vec(-2, -2);
		this->spriteSize = Vec(30, 30);
		this->spriteImage = Image::load("res/ComponentLibrary/PJ301M.png");
	}
};
typedef PJ301M<InputPort> InputPortPJ301M;
typedef PJ301M<OutputPort> OutputPortPJ301M;

template <typename BASE>
struct PJ3410 : BASE {
	PJ3410() {
		this->box.size = Vec(32, 32);
		this->spriteOffset = Vec(-1, -1);
		this->spriteSize = Vec(36, 36);
		this->spriteImage = Image::load("res/ComponentLibrary/PJ3410.png");
	}
};
typedef PJ3410<InputPort> InputPortPJ3410;
typedef PJ3410<OutputPort> OutputPortPJ3410;

template <typename BASE>
struct CL1362 : BASE {
	CL1362() {
		this->box.size = Vec(33, 29);
		this->spriteOffset = Vec(-2, -2);
		this->spriteSize = Vec(39, 36);
		this->spriteImage = Image::load("res/ComponentLibrary/CL1362.png");
	}
};
typedef CL1362<InputPort> InputPortCL1362;
typedef CL1362<OutputPort> OutputPortCL1362;

////////////////////
// Lights
////////////////////

struct ValueLight : Light {
	float *value;
};

struct RedValueLight : ValueLight {
	void step() {
		float v = sqrtBipolar(getf(value));
		color = nvgLerpRGBA(SCHEME_BLACK, SCHEME_RED, v);
	}
};

struct GreenRedPolarityLight : ValueLight {
	void step() {
		float v = sqrtBipolar(getf(value));
		if (v >= 0.0)
			color = nvgLerpRGBA(SCHEME_BLACK, SCHEME_GREEN, v);
		else
			color = nvgLerpRGBA(SCHEME_BLACK, SCHEME_RED, -v);
	}
};

template <typename BASE>
struct LargeLight : BASE {
	LargeLight() {
		this->box.size = Vec(20, 20);
	}
};

template <typename BASE>
struct MediumLight : BASE {
	MediumLight() {
		this->box.size = Vec(12, 12);
	}
};

template <typename BASE>
struct SmallLight : BASE {
	SmallLight() {
		this->box.size = Vec(8, 8);
	}
};

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
