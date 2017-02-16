#pragma once
#include "scene.hpp"


namespace rack {


enum ColorNames {
	COLOR_BLACK,
	COLOR_WHITE,
	COLOR_RED,
	COLOR_ORANGE,
	COLOR_YELLOW,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_BLUE,
	COLOR_PURPLE,
	NUM_COLORS
};

extern const NVGcolor colors[NUM_COLORS];

////////////////////
// Knobs
////////////////////

struct SynthTechAlco : SpriteKnob {
	SynthTechAlco() {
		box.size = Vec(45, 45);
		spriteOffset = Vec(-3, -2);
		spriteSize = Vec(51, 51);
		minIndex = 49;
		maxIndex = -51;
		spriteCount = 120;
		spriteImage = Image::load("res/ComponentLibrary/SynthTechAlco.png");
	}
};

struct Davies1900hKnob : SVGKnob {
	Davies1900hKnob() {
		box.size = Vec(36, 36);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
	}
};

struct Davies1900hWhiteKnob : Davies1900hKnob {
	Davies1900hWhiteKnob() {
		setSVG(SVG::load("res/ComponentLibrary/Davies1900hWhite.svg"));
	}
};

struct Davies1900hBlackKnob : Davies1900hKnob {
	Davies1900hBlackKnob() {
		setSVG(SVG::load("res/ComponentLibrary/Davies1900hBlack.svg"));
	}
};

struct Davies1900hRedKnob : Davies1900hKnob {
	Davies1900hRedKnob() {
		setSVG(SVG::load("res/ComponentLibrary/Davies1900hRed.svg"));
	}
};

struct BefacoBigKnob : SVGKnob {
	BefacoBigKnob() {
		box.size = Vec(75, 75);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load("res/ComponentLibrary/BefacoBigKnob.svg"));
	}
};

struct BefacoTinyKnob : SVGKnob {
	BefacoTinyKnob() {
		box.size = Vec(26, 26);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load("res/ComponentLibrary/BefacoTinyKnob.svg"));
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

struct PJ301MPort : Port {
	PJ301MPort() {
		box.size = Vec(24, 24);
		spriteOffset = Vec(-2, -2);
		spriteSize = Vec(30, 30);
		spriteImage = Image::load("res/ComponentLibrary/PJ301M.png");
	}
};

struct PJ3410Port : Port {
	PJ3410Port() {
		box.size = Vec(32, 31);
		spriteOffset = Vec(-1, -1);
		spriteSize = Vec(36, 36);
		spriteImage = Image::load("res/ComponentLibrary/PJ3410.png");
	}
};

struct CL1362Port : Port {
	CL1362Port() {
		box.size = Vec(33, 29);
		spriteOffset = Vec(-2, -2);
		spriteSize = Vec(39, 36);
		spriteImage = Image::load("res/ComponentLibrary/CL1362.png");
	}
};

////////////////////
// Lights
////////////////////

struct ValueLight : Light {
	float *value;
};

template <int COLOR>
struct ColorValueLight : ValueLight {
	void step() {
		float v = sqrtBipolar(getf(value));
		color = nvgLerpRGBA(colors[COLOR_BLACK], colors[COLOR], v);
	}
};

typedef ColorValueLight<COLOR_RED> RedValueLight;
typedef ColorValueLight<COLOR_YELLOW> YellowValueLight;
typedef ColorValueLight<COLOR_GREEN> GreenValueLight;

template <int COLOR_POS, int COLOR_NEG>
struct PolarityLight : ValueLight {
	void step() {
		float v = sqrtBipolar(getf(value));
		if (v >= 0.0)
			color = nvgLerpRGBA(colors[COLOR_BLACK], colors[COLOR_POS], v);
		else
			color = nvgLerpRGBA(colors[COLOR_BLACK], colors[COLOR_NEG], -v);
	}
};

typedef PolarityLight<COLOR_GREEN, COLOR_RED> GreenRedPolarityLight;

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
		borderColor = nvgRGB(0xa1, 0xa1, 0xa1);
	}
};

struct DarkPanel : Panel {
	DarkPanel() {
		backgroundColor = nvgRGB(0x17, 0x17, 0x17);
		borderColor = nvgRGB(0x5e, 0x5e, 0x5e);
	}
};


} // namespace rack
