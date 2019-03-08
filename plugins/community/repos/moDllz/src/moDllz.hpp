#include "rack.hpp"
#define FONT_FILE  assetPlugin(plugin, "res/bold_led_board-7.ttf")
#define mFONT_FILE  assetPlugin(plugin, "res/ShareTechMono-Regular.ttf")
#include <iomanip> // setprecision
#include <sstream> // stringstream

using namespace rack;

RACK_PLUGIN_DECLARE(moDllz);

#ifdef USE_VST2
#define plugin "moDllz"
#endif // USE_VST2

///////////////////////
// custom components
///////////////////////

///knob44
struct moDllzKnobM : SVGKnob {
    moDllzKnobM() {
      //  box.size = Vec(44, 44);
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/moDllzKnobM.svg")));
        shadow->opacity = 0.f;
    }
};
///knob32
struct moDllzKnob32 : SVGKnob {
    moDllzKnob32() {
       // box.size = Vec(32, 32);
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/moDllzKnob32.svg")));
        shadow->opacity = 0.f;
    }
};
struct moDllzKnob26 : SVGKnob {
    moDllzKnob26() {
      //  box.size = Vec(32, 32);
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/moDllzKnob26.svg")));
        shadow->opacity = 0.f;
    }
};

struct moDllzKnob22 : SVGKnob {
	moDllzKnob22() {
		//  box.size = Vec(32, 32);
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/moDllzKnob22.svg")));
		shadow->opacity = 0.f;
	}
};

///TinyTrim
struct moDllzTTrim : SVGKnob {
  
    moDllzTTrim() {
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/moDllzTTrim.svg")));
        shadow->opacity = 0.f;
    }
};
struct TTrimSnap : moDllzTTrim{
    TTrimSnap(){
        snap = true;
    }
};

///SnapSelector32
struct moDllzSelector32 : SVGKnob {
    moDllzSelector32() {
   //     box.size = Vec(32, 32);
        minAngle = -0.85*M_PI;
        maxAngle = 0.85*M_PI;
        snap = true;
        setSVG(SVG::load(assetPlugin(plugin, "res/moDllzSnap32.svg")));
        shadow->opacity = 0.f;
    }
};

struct moDllzSmSelector : SVGKnob{
    moDllzSmSelector() {
    //    box.size = Vec(36, 36);
        minAngle = -0.5*M_PI;
        maxAngle = 0.5*M_PI;
        snap = true;
        setSVG(SVG::load(assetPlugin(plugin, "res/moDllzSmSelector.svg")));
        shadow->opacity = 0.f;
    }
};

///switch
struct moDllzSwitch : SVGSwitch, ToggleSwitch {
    moDllzSwitch() {
   //     box.size = Vec(10, 20);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitch_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitch_1.svg")));
    }
};
///Horizontal switch
struct moDllzSwitchH : SVGSwitch, ToggleSwitch {
    moDllzSwitchH() {
   //     box.size = Vec(20, 10);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchH_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchH_1.svg")));
    }
};
///Switch with Led
struct moDllzSwitchLed : SVGSwitch, ToggleSwitch {
    moDllzSwitchLed() {
   //     box.size = Vec(10, 18);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchLed_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchLed_1.svg")));
    }
};
///Horizontal switch with Led
struct moDllzSwitchLedH : SVGSwitch, ToggleSwitch {
    moDllzSwitchLedH() {
   //     box.size = Vec(18, 10);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchLedH_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchLedH_1.svg")));
    }
};
///Horizontal switch Triple with Led
struct moDllzSwitchLedHT : SVGSwitch, ToggleSwitch {
    moDllzSwitchLedHT() {
  //      box.size = Vec(24, 10);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchLedHT_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchLedHT_1.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchLedHT_2.svg")));
    }
};

/// Transp Light over led switches
struct TranspOffLight : ModuleLightWidget {
	TranspOffLight() {
		box.size = Vec(10.f,10.f);
		bgColor = COLOR_BLACK_TRANSPARENT;
		borderColor =COLOR_BLACK_TRANSPARENT;
		//addBaseColor(nvgRGBA(0xFF, 0, 0, 0x80));//borderColor = nvgRGBA(0, 0, 0, 0x60);
	}
};


///switch TriState
struct moDllzSwitchT : SVGSwitch, ToggleSwitch {
    moDllzSwitchT() {
     //   box.size = Vec(10, 30);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchT_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchT_1.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchT_2.svg")));
    }
};

///switch TriState Horizontal
struct moDllzSwitchTH : SVGSwitch, ToggleSwitch {
    moDllzSwitchTH() {
       // box.size = Vec(30, 10);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchTH_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchTH_1.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzSwitchTH_2.svg")));
    }
};

///Momentary Button
struct moDllzMoButton : SVGSwitch, MomentarySwitch {
    moDllzMoButton() {
    //    box.size = Vec(48, 27);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzMoButton_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzMoButton_1.svg")));
    }
};

///Momentary Label Clear Button
struct moDllzClearButton : SVGSwitch, MomentarySwitch {
    moDllzClearButton() {
   //     box.size = Vec(38, 13);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzClearButton.svg")));
    }
};

///Momentary Round Button
struct moDllzRoundButton : SVGSwitch, MomentarySwitch {
    moDllzRoundButton() {
    //    box.size = Vec(14, 14);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzRoundButton.svg")));
    }
};


///Momentary PulseUp
struct moDllzPulseUp : SVGSwitch, MomentarySwitch {
    moDllzPulseUp() {
    //    box.size = Vec(12, 12);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzPulse2Up.svg")));
    }
};
///Momentary PulseDown
struct moDllzPulseDwn : SVGSwitch, MomentarySwitch {
    moDllzPulseDwn() {
    //    box.size = Vec(12, 12);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzPulse2Dwn.svg")));
    }
};

///MuteGate
struct moDllzMuteG : SVGSwitch, MomentarySwitch {
    moDllzMuteG() {
    //    box.size = Vec(67, 12);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzMuteG.svg")));
    }
};
///MuteGateP
struct moDllzMuteGP : SVGSwitch, MomentarySwitch {
    moDllzMuteGP() {
   //     box.size = Vec(26, 11);
        addFrame(SVG::load(assetPlugin(plugin, "res/moDllzMuteGP.svg")));
    }
};
///MIDIPanic
struct moDllzMidiPanic : SVGSwitch, MomentarySwitch {
    moDllzMidiPanic() {
   //     box.size = Vec(38, 12);
        addFrame(SVG::load(assetPlugin(plugin, "res/midireset3812.svg")));
    }
};


/// Cursor  Buttons L R

struct moDllzcursorL : SVGSwitch, MomentarySwitch {
	moDllzcursorL() {
		addFrame(SVG::load(assetPlugin(plugin, "res/cursorL_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/cursorL_1.svg")));
	}
};
struct moDllzcursorR : SVGSwitch, MomentarySwitch {
	moDllzcursorR() {
		addFrame(SVG::load(assetPlugin(plugin, "res/cursorR_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/cursorR_1.svg")));
	}
};
/// UpDown + - Buttons

struct minusButton : SVGSwitch, MomentarySwitch {
	minusButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/minusButton_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/minusButton_1.svg")));
	}
};
struct plusButton : SVGSwitch, MomentarySwitch {
	plusButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/plusButton_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/plusButton_1.svg")));
	}
};


///Jacks
struct moDllzPort : SVGPort {
        moDllzPort() {
            background->svg = SVG::load(assetPlugin(plugin, "res/moDllzPort.svg"));
            background->wrap();
            box.size = background->box.size;
        }
};
struct moDllzPortDark : SVGPort {
    moDllzPortDark() {
        background->svg = SVG::load(assetPlugin(plugin, "res/moDllzPortDark.svg"));
        background->wrap();
        box.size = background->box.size;
    }
};


/////////////////////////////////////////////
///// TEST DISPLAY //////
//
//struct testDisplay : TransparentWidget {
//    testDisplay() {
//        font = Font::load(FONT_FILE);
//    }
//    float mdfontSize = 16.f;
//    std::shared_ptr<Font> font;
//    std::string displayedVal;
//    float *valP;
//    float val = 0.f;
//    void draw(NVGcontext* vg)
//    {
//        val = *valP;
//        displayedVal = std::to_string(val);
//        nvgFillColor(vg, nvgRGB(0xFF,0xFF,0x00));
//        nvgFontSize(vg, mdfontSize);
//        //nvgTextLetterSpacing(vg, 2.0f);
//        //nvgTextAlign(vg, NVG_ALIGN_CENTER);
//        nvgTextBox(vg, 0.0f, 20.0f, box.size.x, displayedVal.c_str(), NULL);
//
//    }
//
//};
////////////////////////////////////////////
///////////////////////////////////////////


