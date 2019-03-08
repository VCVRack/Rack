#pragma once
#include "componentlibrary.hpp"
#include "app.hpp"
#include "asset.hpp"
#include <vector>
#include <jansson.h>
#include "widgets.hpp"
#include <iostream>

using namespace std;

namespace rack_plugin_Bark {

	///Colour--------------------------------------------------
	const NVGcolor BARK_GREEN = nvgRGBA(73, 191, 0, 255);
	const NVGcolor BARK_YELLOW1 = nvgRGBA(255, 212, 42, 255);
	const NVGcolor BARK_YELLOW2 = nvgRGBA(255, 192, 42, 255);
	const NVGcolor BARK_ORANGE = nvgRGBA(250, 123, 0, 255);
	const NVGcolor BARK_RED = nvgRGBA(186, 15, 0, 255);
	const NVGcolor BARK_CLIPPING = nvgRGBA(240, 255, 255, 255);//white
	///Colour--------------------------------------------------
	//const int btnLock;

	////Screw----
	struct BarkScrew1 : SVGScrew {
		BarkScrew1() {
			sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew1.svg"));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew2 : SVGScrew {
		BarkScrew2() {
			sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew2.svg"));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew3 : SVGScrew {
		BarkScrew3() {
			sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew3.svg"));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew4 : SVGScrew {
		BarkScrew4() {
			sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew4.svg"));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew01 : SVGKnob {
		BarkScrew01() {
			minAngle = -6.99 * M_PI;
			maxAngle = 6.99 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkScrew01.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.18f;
		}
		void randomize() override {}
	};

	struct BarkScrew02 : SVGKnob {
		BarkScrew02() {
			minAngle = -2.0 * M_PI;
			maxAngle = 2.0 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkScrew01.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.5f;
		}
		void randomize() override {}
	};

	////Toggle----
	struct BarkSwitch : SVGSwitch, ToggleSwitch {
		BarkSwitch() {
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitch_0.svg")));	//	State=0
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitch_1.svg")));	//	State=1
		}
	};

	struct BarkSwitchSmall : SVGSwitch, ToggleSwitch {
		BarkSwitchSmall() {
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitchSmall_0.svg")));
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitchSmall_1.svg")));
		}
	};

	struct BarkSwitchSmallSide : SVGSwitch, ToggleSwitch {
		BarkSwitchSmallSide() {
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitchSmallSide_0.svg")));
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitchSmallSide_1.svg")));
		}
	};
	
	struct BarkButton1 : SVGSwitch, MomentarySwitch {
		BarkButton1() {
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkButtonReset_0.svg")));
		}
	};

	//struct BarkButtonMinus : SVGSwitch, ToggleSwitch {
	//	BarkButtonMinus() {
	//		addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkButtonMinus.svg")));
	//		addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkButtonMinus_0.svg")));
	//	}
	//};

	//struct BarkButtonPlus : SVGSwitch, ToggleSwitch {	//MomentarySwitch
	//	BarkButtonPlus() {
	//		addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkButtonPlus.svg")));
	//		addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkButtonPlus_0.svg")));
	//	}
	//};

	//struct BarkBtnLockSnap : SVGSwitch, ToggleSwitch {
	//	BarkBtnLockSnap() {
	//		addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkButtonLock.svg")));
	//		addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkButtonUnlock.svg")));
	//	}
	//};

	////Slider----
	struct BarkSlide1 : SVGFader {
		BarkSlide1() {
			///TODO: toggle for snap or fade or momentary button to snap to nearest
			snap = false;
			maxHandlePos = Vec(95.f, 0.0f);
			minHandlePos = Vec(-5.0f, 0.0f);
			background->svg = SVG::load(assetPlugin(plugin, "res/components/Barkslider1.svg"));
			background->wrap();
			background->box.pos = Vec(0.0f, 0.0f);
			box.size = background->box.size;
			handle->svg = SVG::load(assetPlugin(plugin, "res/components/BarksliderHandle1.svg"));
			handle->wrap();
			handle->box.pos = Vec(0.0f, 0.0f);
			speed = 0.5f;
		}
		///flips up/down axis to left/right
		void onDragMove(EventDragMove &e) override {
			EventDragMove e2 = e;
			e2.mouseRel = Vec(e.mouseRel.y, -e.mouseRel.x);
			SVGFader::onDragMove(e2);
		}
		///turns off randomising
		void randomize() override {}	
	};

	////Ports----
	///Port In--
	struct BarkInPort : SVGPort {
		BarkInPort() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkInPort1 : SVGPort {
		BarkInPort1() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort1.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkInPort2 : SVGPort {
		BarkInPort2() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort2.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkPatchPortIn : SVGPort {
		BarkPatchPortIn() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkPatchPortIn.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkInPort350 : SVGPort {
		BarkInPort350() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort350.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};
	///Port Out--
	struct BarkOutPort : SVGPort {
		BarkOutPort() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkOutPort1 : SVGPort {
		BarkOutPort1() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort1.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkOutPort2 : SVGPort {
		BarkOutPort2() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort2.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkPatchPortOut : SVGPort {
		BarkPatchPortOut() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkPatchPortOut.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	struct BarkOutPort350 : SVGPort {
		BarkOutPort350() {
			background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort350.svg"));
			background->wrap();
			box.size = background->box.size;
		}
	};

	////Knobs----
	struct BarkKnob9 : SVGKnob {
		BarkKnob9() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob9.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob24 : SVGKnob {
		BarkKnob24() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob24.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob26 : SVGKnob {
		BarkKnob26() {
			minAngle = -0.829 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob26.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.65f;
		}
	};

	struct BarkKnob30 : SVGKnob {
		BarkKnob30() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob30.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.7f;
		}
	};

	struct BarkKnob40 : SVGKnob {
		BarkKnob40() {
			minAngle = -0.827 * M_PI;
			maxAngle = 0.825 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob40.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.8f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.07f);
		}
	};

	struct BarkKnob57 : SVGKnob {
		BarkKnob57() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob57.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	//struct snapMode : SVGKnob {
	//	ParamWidget *modeSwitch;
	//	void step() override {
	//		snap = (modeSwitch->value > 0.f);
	//		SVGKnob::snap();
	//		//snapMode->modeSwitch = modeSwitch
	//	}
	//};

	struct BarkKnob70 : SVGKnob {
		BarkKnob70() {
			minAngle = -0.83 * M_PI;
			maxAngle = 0.828 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob70.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.5f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.05);
		}
	};

	struct BarkKnob70Snap : SVGKnob {
		BarkKnob70Snap() {
			snap = true;
			minAngle = -0.83 * M_PI;
			maxAngle = 0.828 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob70.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.5f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.05);
		}
	};

	struct BarkKnob84 : SVGKnob {
		BarkKnob84() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob84.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob92 : SVGKnob {
		BarkKnob92() {
			minAngle = -0.83 * M_PI;
			maxAngle = 0.83 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob92.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct KnobTest1 : SVGKnob {
		KnobTest1() {
			setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobTest1.svg")));
			
		}
	};

	///Light----
	struct greenLight : GrayModuleLightWidget {
		greenLight() {
			addBaseColor(BARK_GREEN);
		}
	};

	struct yellowLight1 : GrayModuleLightWidget {
		yellowLight1() {
			addBaseColor(BARK_YELLOW1);
		}
	};

	struct yellowLight2 : GrayModuleLightWidget {
		yellowLight2() {
			addBaseColor(BARK_YELLOW2);
		}
	};

	struct orangeLight : GrayModuleLightWidget {
		orangeLight() {
			addBaseColor(BARK_ORANGE);
		}
	};

	struct redLight : GrayModuleLightWidget {
		redLight() {
			addBaseColor(BARK_RED);
		}
	};

	struct clipLight : GrayModuleLightWidget {
		clipLight() {
			addBaseColor(BARK_CLIPPING);
		}
	};
	struct ParamInLight : GrayModuleLightWidget {
		ParamInLight() {
			addBaseColor(BARK_CLIPPING);
		}
	};

	template <typename BASE>
	struct BiggerLight : BASE {
		BiggerLight() {
			this->box.size = Vec(10, 10);//px
			this->bgColor = nvgRGBA(192, 192, 192, 32);//silver
		}
	};

	template <typename BASE>
	struct BigLight : BASE {
		BigLight() {
			this->box.size = Vec(8, 8);//px
			this->bgColor = nvgRGBA(192, 192, 192, 32);//silver
		}
	};

	template <typename BASE>
	struct SmallerLight : BASE {
		SmallerLight() {
			this->box.size = Vec(4, 4);//px
			this->bgColor = nvgRGBA(192, 192, 192, 45);//silver
		}
	};
	
	template <typename BASE>
	struct SmallerLightFA : BASE {
		SmallerLightFA() {
			this->box.size = Vec(4, 4);//px
			this->bgColor = nvgRGBA(56, 56, 56, 255);//panel
			this->borderColor = nvgRGBA(56, 56, 56, 255);//panel
		}
	};

	template <typename BASE>
	struct SmallestLight : BASE {
		SmallestLight() {
			this->box.size = Vec(3, 3);//px
			this->bgColor = nvgRGBA(192, 192, 192, 45);//silver
		}
	};

} // namespace rack_plugin_Bark
