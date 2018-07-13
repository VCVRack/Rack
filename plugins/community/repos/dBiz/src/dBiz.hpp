#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(dBiz);

// // #ifdef USE_VST2
// // #define plugin "dBiz"
// // #endif // USE_VST2

namespace rack_plugin_dBiz {

////////////////////
// Colors
///// ///////////////

#define COLOR_BLACK_TRANSPARENT nvgRGBA(0x00, 0x00, 0x00, 0x00)
#define COLOR_BLACK nvgRGB(0x00, 0x00, 0x00)
#define COLOR_WHITE nvgRGB(0xff, 0xff, 0xff)
#define COLOR_RED nvgRGB(0xed, 0x2c, 0x24)
#define COLOR_ORANGE nvgRGB(0xf2, 0xb1, 0x20)
#define COLOR_YELLOW nvgRGB(0xf9, 0xdf, 0x1c)
#define COLOR_GREEN nvgRGB(0x90, 0xc7, 0x3e)
#define COLOR_CYAN nvgRGB(0x22, 0xe6, 0xef)
#define COLOR_BLUE nvgRGB(0x29, 0xb2, 0xef)
#define COLOR_PURPLE nvgRGB(0xd5, 0x2b, 0xed)

////////////////////
// Knobs
////////////////////
struct VerboL : SVGKnob
{
	VerboL()
	{
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/component/VerboL.svg")));
		box.size = Vec(80, 80);
	}
};

struct VerboS : SVGKnob
{
	VerboS()
	{
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/component/VerboS.svg")));
		box.size = Vec(35, 35);
	}
};

struct SmallKnob : SVGKnob
{
	SmallKnob()
	{
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
	}
};

struct SmallOra : SmallKnob
{
	SmallOra()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/SmallOra.svg")));
	}
};

struct SmallOraSnapKnob : SmallOra
{
	SmallOraSnapKnob()
	{
		snap = true;
	};
};

struct LargeOra : SmallOra
{
	LargeOra()
	{
		box.size = Vec(45, 45);
	}
};
struct MicroOra : SmallOra
{
	MicroOra()
	{
		box.size = Vec(25, 25);
	}
};

struct SmallBlu : SmallKnob
{
	SmallBlu()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/SmallBlu.svg")));
	}
};
struct MicroBlu : SmallBlu
{
	MicroBlu()
	{
		box.size = Vec(25, 25);
	}
};

struct LargeBlu : SmallBlu
{
	LargeBlu()
	{
		box.size = Vec(45, 45);
	}
};


struct SmallCre : SmallKnob
{
	SmallCre()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/SmallCre.svg")));
	}
};

struct SmallBla : SmallKnob
{
	SmallBla()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/SmallBla.svg")));
	}
};

struct LargeBla : SmallBla
{
	LargeBla()
	{
		box.size = Vec(45, 45);
	}
};

struct DaviesKnob : SVGKnob
{
	DaviesKnob()
	{
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		box.size = Vec(15, 15);
	}
};

struct DaviesGre : DaviesKnob
{
	DaviesGre()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/DaviesGre.svg")));
	}
};
struct LDaviesGre : DaviesGre
{
	LDaviesGre()
	{
		box.size = Vec(45, 45);
	}
};

struct DaviesWhy : DaviesKnob
{
	DaviesWhy()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/DaviesWhy.svg")));
	}
};
struct LDaviesWhy : DaviesWhy
{
	LDaviesWhy()
	{
		box.size = Vec(45, 45);
	}
};

struct DaviesWhySnapKnob : DaviesWhy
{
	DaviesWhySnapKnob()
	{
		snap = true;
	};
};

struct DaviesAzz : DaviesKnob
{
	DaviesAzz()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/DaviesAzz.svg")));
	}
};
struct LDaviesAzz : DaviesAzz
{
	LDaviesAzz()
	{
		box.size = Vec(45, 45);
	}
};

struct DaviesPur : DaviesKnob
{
	DaviesPur()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/DaviesPur.svg")));
	}
};
struct LDaviesPur : DaviesPur
{
	LDaviesPur()
	{
		box.size = Vec(45, 45);
	}
};

struct DaviesBlu : DaviesKnob
{
	DaviesBlu()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/DaviesBlu.svg")));
	}
};
struct LDaviesBlu : DaviesBlu
{
	LDaviesBlu()
	{
		box.size = Vec(45, 45);
	}
};

struct DaviesRed : DaviesKnob
{
	DaviesRed()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/DaviesRed.svg")));
	}
};
struct LDaviesRed : DaviesRed
{
	LDaviesRed()
	{
		box.size = Vec(45, 45);
	}
};

struct DaviesYel : DaviesKnob
{
	DaviesYel()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/DaviesYel.svg")));
	}
};
struct LDaviesYel : DaviesYel
{
	LDaviesYel()
	{
		box.size = Vec(45, 45);
	}
};

struct RoundAzz : DaviesKnob
{
	RoundAzz()
	{
		box.size = Vec(30, 30);
		setSVG(SVG::load(assetPlugin(plugin, "res/component/RoundAzz.svg")));
	}
};
struct RoundRed : DaviesKnob
{
	RoundRed()
	{
		box.size = Vec(30, 30);
		setSVG(SVG::load(assetPlugin(plugin, "res/component/RoundRed.svg")));
	}
};
struct RoundWhy : DaviesKnob
{
	RoundWhy()
	{
		box.size = Vec(30, 30);
		setSVG(SVG::load(assetPlugin(plugin, "res/component/RoundWhy.svg")));
	}
};

struct RoundWhySnapKnob : RoundWhy
{
	RoundWhySnapKnob()
	{
		snap = true;
	};
};

struct LRoundWhy : RoundWhy
{
	LRoundWhy()
	{
		box.size = Vec(45, 45);
	}
};

struct RoundBlu : DaviesKnob
{
	RoundBlu()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/RoundBlu.svg")));
	}
};

struct LRoundBlu : RoundBlu
{
	LRoundBlu()
	{
		box.size = Vec(45, 45);
	}
};

struct FlatA : DaviesKnob
{
	FlatA()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/FlatA.svg")));
		box.size = Vec(30, 30);
	}
};
struct FlatASnap : FlatA
{
	FlatASnap()
	{
		snap = true;
	}
};

struct FlatR : DaviesKnob
{
	FlatR()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/FlatR.svg")));
		box.size = Vec(30, 30);
	}
};
struct FlatS : DaviesKnob
{
	FlatS()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/FlatS.svg")));
		box.size = Vec(30, 30);
	}
};
struct FlatG : DaviesKnob
{
	FlatG()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/component/FlatG.svg")));
		box.size = Vec(30, 30);
	}
};

// struct DaviesKnobSnapKnob : DaviesKnob, SnapKnob {};

//////////////////////
//slider
///////////////////

struct SlidePot : SVGFader
{
	SlidePot()
	{
		Vec margin = Vec(3.5, 3.5);
		maxHandlePos = Vec(-1, -2).plus(margin);
		minHandlePos = Vec(-1, 87).plus(margin);
		background->svg = SVG::load(assetPlugin(plugin, "res/component/SlidePot.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = SVG::load(assetPlugin(plugin, "res/component/SlidePotHandle.svg"));
		handle->wrap();
	}
};

struct SlidePot2 : SVGFader
{
	SlidePot2()
	{
		Vec margin = Vec(3.5, 3.5);
		maxHandlePos = Vec(-10, -2).plus(margin);
		minHandlePos = Vec(-10, 87).plus(margin);
		background->svg = SVG::load(assetPlugin(plugin, "res/component/SlidePot.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = SVG::load(assetPlugin(plugin, "res/component/SlidePotHandle2.svg"));
		handle->wrap();
	}
};
////////////////////
// Lights
////////////////////

struct OrangeLight : GrayModuleLightWidget
{
	OrangeLight()
	{
		addBaseColor(COLOR_ORANGE);
	}
};

struct CyanLight : GrayModuleLightWidget
{
	CyanLight()
	{
		addBaseColor(COLOR_CYAN);
	}
};

struct WhiteLight : GrayModuleLightWidget
{
	WhiteLight()
	{
		addBaseColor(COLOR_WHITE);
	}
};


template <typename BASE>
struct BigLight : BASE
{
	BigLight()
	{
		this->box.size = Vec(20, 20);
	}
};

template <typename BASE>
struct HugeLight : BASE
{
	HugeLight()
	{
		this->box.size = Vec(24, 24);
	}
};

struct OBPLight : GrayModuleLightWidget
{
	OBPLight()
	{
		addBaseColor(COLOR_ORANGE);
		addBaseColor(COLOR_BLUE);
		addBaseColor(COLOR_PURPLE);
	}
};

////////////////////
// Jacks
////////////////////

struct PJ301MRPort : SVGPort
{
	PJ301MRPort()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/component/PJ301MR.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301MLPort : SVGPort
{
	PJ301MLPort()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/component/PJ301ML.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301MIPort : SVGPort
{
	PJ301MIPort()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/component/PJ301MA.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301MOrPort : SVGPort
{
	PJ301MOrPort()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/component/PJ301MO.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301MOPort : SVGPort
{
	PJ301MOPort()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/component/PJ301MB.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301MCPort : SVGPort
{
	PJ301MCPort()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/component/PJ301MW.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

//
////////////////////////
//  SWITCHES
////////////////////////////////////////////////

struct CKSSS : SVGSwitch, ToggleSwitch
{
	CKSSS()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/component/CKSS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/component/CKSS_1.svg")));
	}
};

struct LEDB : SVGSwitch, ToggleSwitch
{
	LEDB()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/component/LEDB_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/component/LEDB_1.svg")));
	}
};

struct MCKSSS : SVGSwitch, ToggleSwitch
{
	MCKSSS()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/component/MCKSSS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/component/MCKSSS_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/component/MCKSSS_2.svg")));
	}
};

struct BPush : SVGSwitch, MomentarySwitch
{
	BPush()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/component/BPush_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/component/BPush_1.svg")));
	}
};

} // namespace rack_plugin_dBiz
