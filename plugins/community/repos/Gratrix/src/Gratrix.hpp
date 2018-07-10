#ifndef GTX__GRATRIX_HPP
#define GTX__GRATRIX_HPP


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <array>
#include <cmath>
#include <limits>
#include "rack.hpp"


#define GTX__N          6
#define GTX__2PI        6.283185307179586476925
#define GTX__IO_RADIUS  26.0
#define GTX__SAVE_SVG   0
#define GTX__WIDGET()   // do { std::cout << "Gratrix Module : " << __FUNCTION__ << "();" << std::endl; } while(0);


using namespace rack;

RACK_PLUGIN_DECLARE(Gratrix);

#ifdef USE_VST2
#define plugin "Gratrix"
#endif // USE_VST2

namespace rack_plugin_Gratrix {

struct MicroModule
{
	std::vector<Param>  params;
	std::vector<Input>  inputs;
	std::vector<Output> outputs;
	std::vector<Light>  lights;

	MicroModule(int numParams, int numInputs, int numOutputs, int numLights = 0)
	{
		params.resize(numParams);
		inputs.resize(numInputs);
		outputs.resize(numOutputs);
		lights.resize(numLights);
	}
};


//============================================================================================================
//! \brief Simple cache structure.

template <typename T> struct Cache
{
	bool valid;
	T    value;

	Cache()
	{
		reset();
	}

	void reset()
	{
		valid = false;
	}

	void set(const T &that)
	{
		valid = true;
		value = that;
	}

	bool test(const T &that)
	{
		return valid && value != that;
	}
};


//============================================================================================================
//! \name UI Port components

struct PortInMed : SVGPort
{
	PortInMed()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/components/PortInMedium.svg"));
		background->wrap();
		box.size = background->box.size;
	}

	static Vec size() { return Vec(25.0, 25.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(12.5, 12.5); }  // Copied from SVG so no need to pre-load.
};

struct PortOutMed : SVGPort
{
	PortOutMed()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/components/PortOutMedium.svg"));
		background->wrap();
		box.size = background->box.size;
	}

	static Vec size() { return Vec(25.0, 25.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(12.5, 12.5); }  // Copied from SVG so no need to pre-load.
};

struct PortInSml : SVGPort
{
	PortInSml()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/components/PortInSmall.svg"));
		background->wrap();
		box.size = background->box.size;
	}

	static Vec size() { return Vec(18, 18); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec( 9,  9); }  // Copied from SVG so no need to pre-load.
};

struct PortOutSml : SVGPort
{
	PortOutSml()
	{
		background->svg = SVG::load(assetPlugin(plugin, "res/components/PortOutSmall.svg"));
		background->wrap();
		box.size = background->box.size;
	}

	static Vec size() { return Vec(18, 18); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec( 9,  9); }  // Copied from SVG so no need to pre-load.
};


//============================================================================================================
//! \name UI Knob components

struct KnobFreeHug : RoundKnob
{
	KnobFreeHug()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobFreeHuge.svg")));
		box.size = Vec(56, 56);
	}

	static Vec size() { return Vec(56.0, 56.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(28.0, 28.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobSnapHug : RoundKnob
{
	KnobSnapHug()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobSnapHuge.svg")));
		box.size = Vec(56, 56);
		snap = true;
	}

	static Vec size() { return Vec(56.0, 56.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(28.0, 28.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobFreeLrg : RoundKnob
{
	KnobFreeLrg()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobFreeLarge.svg")));
		box.size = Vec(46, 46);
	}

	static Vec size() { return Vec(46.0, 46.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(23.0, 23.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobSnapLrg : RoundKnob
{
	KnobSnapLrg()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobSnapLarge.svg")));
		box.size = Vec(46, 46);
		snap = true;
	}

	static Vec size() { return Vec(46.0, 46.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(23.0, 23.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobFreeMed : RoundKnob
{
	KnobFreeMed()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobFreeMedium.svg")));
		box.size = Vec(38, 38);
	}

	static Vec size() { return Vec(38.0, 38.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(19.0, 19.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobSnapMed : RoundKnob
{
	KnobSnapMed()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobSnapMedium.svg")));
		box.size = Vec(38, 38);
		snap = true;
	}

	static Vec size() { return Vec(38.0, 38.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(19.0, 19.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobFreeSml : RoundKnob
{
	KnobFreeSml()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobFreeSmall.svg")));
		box.size = Vec(28, 28);
	}

	static Vec size() { return Vec(28.0, 28.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(14.0, 14.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobSnapSml : RoundKnob
{
	KnobSnapSml()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobSnapSmall.svg")));
		box.size = Vec(28, 28);
		snap = true;
	}

	static Vec size() { return Vec(28.0, 28.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(14.0, 14.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobFreeTny : RoundKnob
{
	KnobFreeTny()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobFreeTiny.svg")));
		box.size = Vec(28, 28);
	}

	static Vec size() { return Vec(28.0, 28.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(14.0, 14.0); }  // Copied from SVG so no need to pre-load.
};

struct KnobSnapTny : RoundKnob
{
	KnobSnapTny()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobSnapTiny.svg")));
		box.size = Vec(28, 28);
		snap = true;
	}

	static Vec size() { return Vec(28.0, 28.0); }  // Copied from SVG so no need to pre-load.
	static Vec pos()  { return Vec(14.0, 14.0); }  // Copied from SVG so no need to pre-load.
};


//============================================================================================================
//! \brief Create output function that creates the UI component centered.

template <class TPort> Port *createInputGTX(Vec pos, Module *module, int outputId)
{
	return Port::create<TPort>(pos.minus(TPort::pos()), Port::INPUT, module, outputId);
}


//============================================================================================================
//! \brief Create output function that creates the UI component centered.

template <class TPort> Port *createOutputGTX(Vec pos, Module *module, int outputId)
{
	return Port::create<TPort>(pos.minus(TPort::pos()), Port::OUTPUT, module, outputId);
}


//============================================================================================================
//! \brief Create param function that creates the UI component centered.

template <class TParamWidget> ParamWidget *createParamGTX(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue)
{
	return ParamWidget::create<TParamWidget>(pos.minus(TParamWidget::pos()), module, paramId, minValue, maxValue, defaultValue);
}


//============================================================================================================
//! \brief ...

inline double dx(double i, std::size_t n) { return  std::sin(GTX__2PI * static_cast<double>(i) / static_cast<double>(n)); }
inline double dy(double i, std::size_t n) { return -std::cos(GTX__2PI * static_cast<double>(i) / static_cast<double>(n)); }
inline double dx(double i               ) { return dx(i, GTX__N); }
inline double dy(double i               ) { return dy(i, GTX__N); }

inline int    gx(double i) { return static_cast<int>(std::floor(0.5 + ((i+0.5) *  90))); }
inline int    gy(double i) { return static_cast<int>(std::floor(8.5 + ((i+1.0) * 102))); }
inline int    fx(double i) { return gx(i); }
inline int    fy(double i) { return gy(i - 0.1); }

inline double rad_n_b() { return 27;    }
inline double rad_n_m() { return 18;    }
inline double rad_n_s() { return 14;    }
inline double rad_l_m() { return  4.75; }
inline double rad_l_s() { return  3.25; }
inline double rad_but() { return  9;    }
inline double rad_scr() { return  8;    }
inline double rad_prt() { return 12.5;  }

inline Vec    tog(double x, double y) { return Vec(x - 7, y - 10); }
inline Vec    n_b(double x, double y) { return Vec(x - rad_n_b(), y - rad_n_b()); }
inline Vec    n_m(double x, double y) { return Vec(x - rad_n_m(), y - rad_n_m()); }
inline Vec    n_s(double x, double y) { return Vec(x - rad_n_s(), y - rad_n_s()); }
inline Vec    l_m(double x, double y) { return Vec(x - rad_l_m(), y - rad_l_m()); }
inline Vec    l_s(double x, double y) { return Vec(x - rad_l_s(), y - rad_l_s()); }
inline Vec    but(double x, double y) { return Vec(x - rad_but(), y - rad_but()); }
inline Vec    scr(double x, double y) { return Vec(x - rad_scr(), y - rad_scr()); }
inline Vec    prt(double x, double y) { return Vec(x - rad_prt(), y - rad_prt()); }

inline Vec    tog(const Vec &a)       { return tog(a.x, a.y); }
inline Vec    n_b(const Vec &a)       { return n_b(a.x, a.y); }
inline Vec    n_m(const Vec &a)       { return n_m(a.x, a.y); }
inline Vec    n_s(const Vec &a)       { return n_s(a.x, a.y); }
inline Vec    l_m(const Vec &a)       { return l_m(a.x, a.y); }
inline Vec    l_s(const Vec &a)       { return l_s(a.x, a.y); }
inline Vec    but(const Vec &a)       { return but(a.x, a.y); }
inline Vec    scr(const Vec &a)       { return scr(a.x, a.y); }
inline Vec    prt(const Vec &a)       { return prt(a.x, a.y); }

inline double px(          std::size_t i) { return GTX__IO_RADIUS * dx(i); }
inline double py(          std::size_t i) { return GTX__IO_RADIUS * dy(i); }
inline double px(double j, std::size_t i) { return gx(j) + px(i); }
inline double py(double j, std::size_t i) { return gy(j) + py(i); }


#if GTX__SAVE_SVG

//============================================================================================================
//! \brief Generates SVG file for use as module background.

class PanelGen
{
	std::ofstream os;
	Vec box;

public:
	PanelGen(const std::string &fn, const Vec &box, const std::string &title = "")
	:
	box(box)
	{
		os.open(fn);

		os << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"";
		os << " viewBox=\"0 0 " << box.x << " " << box.y << "\" version=\"1.1\">" << std::endl;

		if (!title.empty())
		{
			os << "<defs>" << std::endl;
			font();
			os << "</defs>" << std::endl;
		}

		rect(Vec(-5,       -5), Vec(box.x+10, box.y+10), "fill:#CEE1FD;stroke:black;stroke-width:1");
		rect(Vec(-5,       -5), Vec(box.x+10, 30   + 5), "fill:#BED7FC;stroke:black;stroke-width:1");
		rect(Vec(-5, box.y-20), Vec(box.x+10, 20   + 5), "fill:#BED7FC;stroke:black;stroke-width:1");

		if (!title.empty())
		{
			line(Vec(1.5*15, 14), Vec(box.x-1.5*15, 14), "stroke:#7092BE;stroke-width:2");
			line(Vec(1.5*15, 18), Vec(box.x-1.5*15, 18), "stroke:#7092BE;stroke-width:2");
			line(Vec(1.5*15, 22), Vec(box.x-1.5*15, 22), "stroke:#7092BE;stroke-width:2");

			circle(Vec(      1.5*15, 7), 17, "fill:#BED7FC;stroke:none");
			circle(Vec(box.x-1.5*15, 7), 17, "fill:#BED7FC;stroke:none");

			text(Vec(box.x / 2, 22),     title    , "font-family: '01 DigitGraphics'; font-weight: bold; font-style: normal; font-size:13; text-anchor:middle; fill:#BED7FC;stroke:#BED7FC;stroke-width:9");
			text(Vec(box.x / 2, 22), "|"+title+"|", "font-family: '01 DigitGraphics'; font-weight: bold; font-style: normal; font-size:13; text-anchor:middle; fill:#BED7FC;stroke:#BED7FC;stroke-width:3");
			text(Vec(box.x / 2, 22),     title    , "font-family: '01 DigitGraphics'; font-weight: bold; font-style: normal; font-size:13; text-anchor:middle; fill:black;");

			text(Vec(box.x / 2, box.y - 6), (box.x <= 6*15 ? "GTX" : "GRATRIX"), "font-family: '01 DigitGraphics'; font-weight: bold; font-style: normal; font-size:10; text-anchor:middle; fill:#777777;");
		}

	//	grid();
	}

	void tog(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		tog_raw(fx(x), fy(y), title, subtitle);
	}
	void tog_raw(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		text(Vec(x, y - 15), title, 10);
		text(Vec(x, y + 23), subtitle, 10);
	}
	void tog_raw2(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		text(Vec(x, y - 14), title, 8);
		text(Vec(x, y + 20), subtitle, 8);
	}

	void nob_big(double x, double y, const std::string &title = "")
	{
		return nob_big_raw(fx(x), fy(y), title);
	}
	void nob_big_raw(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		text(Vec(x, y - rad_n_b() - 10), title, 11);
		text(Vec(x, y + rad_n_b() + 15), subtitle, 10);
	}

	void nob_med(double x, double y, const std::string &title = "")
	{
		return nob_med_raw(fx(x), fy(y), title);
	}
	void nob_med_raw(double x, double y, const std::string &title = "")
	{
		if (!title.empty())
		{
			text(Vec(x, y - rad_n_m() - 5), title, 10);
		}
	}

	void nob_sml(double x, double y, const std::string &title = "")
	{
		return nob_sml_raw(fx(x), fy(y), title);
	}
	void nob_sml_raw(double x, double y, const std::string &title = "")
	{
		text(Vec(x, y - rad_n_s() - 5), title, 10);
	}

	void but2(double x, double y, const std::string &title = "")
	{
		int xx = fx(x);
		int yy = fy(y);
		text(Vec(xx + rad_prt() + 4, yy + 2), title, 10, "start");
	}

	void prt_in(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		return prt_in_raw(gx(x), gy(y), title, subtitle);
	}

	void prt_in_raw(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		text(Vec(x, y - rad_prt() -  7), title, 10);
		text(Vec(x, y + rad_prt() + 15), subtitle, 10);
	}

	void prt_in2(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		int xx = fx(x);
		int yy = fy(y);
		text(Vec(xx, yy - rad_prt() - 7), title, 10);
		text(Vec(xx, yy + rad_prt() + 7 + 8), subtitle, 10);
	}

	void prt_out(double x, double y, const std::string &title = "", const std::string &subtitle = "")
	{
		int xx = (y == 0.0) ? fx(x) : gx(x);
		int yy = (y == 0.0) ? fy(y) : gy(y);
		text(Vec(xx, yy - rad_prt() -  7), title, 10);
		text(Vec(xx, yy + rad_prt() + 15), subtitle, 10);
	}

	void bus_inx(double x, double y, const std::string &title = "")
	{
		text(Vec(gx(x), gy(y) - 44), title, 10);
	}

	void bus_in(double x, double y, const std::string &title = "")
	{
		circle(Vec(gx(x), gy(y)), 40, "fill:#BED7FC;stroke:#7092BE;stroke-width:1");
		text(Vec(gx(x), gy(y) - 44), title, 10);
	}

	void bus_out(double x, double y, const std::string &title = "")
	{
		circle(Vec(gx(x), gy(y)), 40, "fill:#7092BE;stroke:#7092BE;stroke-width:1");
		text(Vec(gx(x), gy(y) - 44), title, 10);
	}

	void text(const Vec &p1, const std::string &t, double size, const std::string &anchor = "middle")
	{
		std::ostringstream oss;
		oss << "font-family: '01 DigitGraphics'; font-weight: bold; font-style: normal; font-size:" << size << "; text-anchor:" << anchor << "; fill:black;";
		text(p1, t, oss.str());
	}

	void text(const Vec &p1, const std::string &t, const std::string &style)
	{
		if (!t.empty())
		{
			os << "<text x=\"" << p1.x << "\" y=\"" << p1.y << "\" style=\"" << style << "\">" << t << "</text>" << std::endl;
		}
	}

	void line(const Vec &p1, const Vec &p2, const std::string &style)
	{
		os << "<line x1=\"" << p1.x << "\" y1=\"" << p1.y << "\" x2=\"" << p2.x << "\" y2=\"" << p2.y << "\" style=\"" << style << "\" />\"" << std::endl;
	}

	void rect(const Vec &p1, const Vec &p2, const std::string &style)
	{
		os << "<rect x=\"" << p1.x << "\" y=\"" << p1.y << "\" width=\"" << p2.x << "\" height=\"" << p2.y << "\" style=\"" << style << "\" />" << std::endl;
	}

	void circle(const Vec &p1, float r, const std::string &style)
	{
		os << "<circle cx=\"" << p1.x << "\" cy=\"" << p1.y << "\" r=\"" << r << "\" style=\"" << style << "\" />" << std::endl;
	}

	void grid()
	{
		for (std::size_t i=0; i<box.x+0.5; i += 3*15)
		{
			line(Vec(i, 0), Vec(i, box.y), "stroke:rgb(255,0,0);stroke-width:2");
		}

		for (std::size_t i=0; i<box.y+0.5; i += 3*15)
		{
			line(Vec(0, i), Vec(box.x, i), "stroke:rgb(255,0,0);stroke-width:2");
		}
	}

	~PanelGen()
	{
		os << "</svg>" << std::endl;

		os.close();
	}

	void font()
	{
os << "<font id=\"01DigitGraphics\" horiz-adv-x=\"729\" >" << std::endl;
os << "<font-face" << std::endl;
os << "  font-family=\"01 DigitGraphics\"" << std::endl;
os << "  font-weight=\"400\"" << std::endl;
os << "  font-stretch=\"normal\"" << std::endl;
os << "  units-per-em=\"1000\"" << std::endl;
os << "  panose-1=\"0 0 4 9 0 0 0 0 0 0\"" << std::endl;
os << "  ascent=\"833\"" << std::endl;
os << "  descent=\"-167\"" << std::endl;
os << "  x-height=\"451\"" << std::endl;
os << "  cap-height=\"701\"" << std::endl;
os << "  bbox=\"80 -154 934 833\"" << std::endl;
os << "  underline-thickness=\"20\"" << std::endl;
os << "  underline-position=\"-143\"" << std::endl;
os << "  unicode-range=\"U+0020-00A0\"" << std::endl;
os << "/>" << std::endl;
os << "<missing-glyph horiz-adv-x=\"638\" d=\"M80 0v833h518v-833h-518zM167 86h345v661h-345v-661z\" />" << std::endl;
os << "<glyph glyph-name=\".notdef\" horiz-adv-x=\"638\" d=\"M80 0v833h518v-833h-518zM167 86h345v661h-345v-661z\" />" << std::endl;
os << "<glyph glyph-name=\".null\" horiz-adv-x=\"120\" />" << std::endl;
os << "<glyph glyph-name=\"nonmarkingreturn\" horiz-adv-x=\"120\" />" << std::endl;
os << "<glyph glyph-name=\"space\" unicode=\" \" horiz-adv-x=\"120\" />" << std::endl;
os << "<glyph glyph-name=\"exclam\" unicode=\"!\" horiz-adv-x=\"240\" d=\"M80 632q0 25 17.5 42.5t42.5 17.5t42.5 -17.5t17.5 -42.5v-398q0 -27 -17.5 -44t-42.5 -17t-42.5 17t-17.5 44v398zM85 52q0 23 16 39t39 16t39 -16t16 -39t-16 -39t-39 -16t-39 16t-16 39zM194 52q0 23 -15.5 38.5t-38.5 15.5t-38.5 -15.5t-15.5 -38.5t15.5 -38.5t38.5 -15.5t38.5 15.5t15.5 38.5zM87 52q0 23 15 38t38 15t38 -15t15 -38t-15 -38t-38 -15t-38 15t-15 38z\" />" << std::endl;
os << "<glyph glyph-name=\"quotedbl\" unicode=\"&#x22;\" horiz-adv-x=\"458\" d=\"M297 603q0 25 17.5 42.5t43.5 17.5t43 -17.5t17 -42.5v-147q0 -26 -17 -43.5t-43 -17.5t-43.5 17.5t-17.5 43.5v147zM80 603q0 25 17 42.5t44 17.5q25 0 42.5 -17.5t17.5 -42.5v-147q0 -26 -17.5 -43.5t-42.5 -17.5q-27 0 -44 17.5t-17 43.5v147z\" />" << std::endl;
os << "<glyph glyph-name=\"numbersign\" unicode=\"#\" horiz-adv-x=\"823\" d=\"M722 283q26 0 43.5 -17t17.5 -42q0 -26 -17.5 -43.5t-43.5 -17.5h-104v-104q0 -25 -17 -42.5t-42 -17.5q-26 0 -43.5 17.5t-17.5 42.5v104h-133v-104q0 -25 -17.5 -42.5t-43.5 -17.5q-25 0 -42 17.5t-17 42.5v104h-104q-25 0 -43 17.5t-18 43.5q0 25 18 42t43 17h104v134h-104q-25 0 -43 17.5t-18 41.5q0 27 18 44t43 17h104v104q0 25 17 43t42 18q26 0 43.5 -18t17.5 -43v-104h133v104q0 25 17.5 43t43.5 18q25 0 42 -18t17 -43v-104h104q26 0 43.5 -17t17.5 -44q0 -24 -17.5 -41.5t-43.5 -17.5h-104v-134h104zM365 283h133v134h-133v-134z\" />" << std::endl;
os << "<glyph glyph-name=\"dollar\" unicode=\"$\" horiz-adv-x=\"824\" d=\"M781 242q3 -48 -22 -89.5t-71 -68.5q-32 -19 -70.5 -25t-75.5 -6h-48v-2q0 -24 -17.5 -42t-42.5 -18t-42.5 18t-17.5 42v3h-230q-25 0 -42 18t-17 43t17 42.5t42 17.5h230v116h-28q-8 0 -16 0.5t-18 1.5q-111 7 -168 54q-38 31 -51 74t3 86q9 27 28 50.5t45.5 40.5t58 26.5t64.5 9.5h82v2q0 27 17.5 44t42.5 17t42.5 -17t17.5 -44v-1h230q8 0 17 -5t17 -13t13 -19t5 -23q0 -25 -14.5 -43t-37.5 -18h-230v-98q46 2 95 -3t82 -21q49 -23 78.5 -62.5t31.5 -87.5zM221 441q16 -14 30 -17t35 -7q22 -3 45 -4.5t43 -1.5v102h-81q-24 0 -41.5 -6t-27.5 -16.5t-11 -23.5t8 -26zM627 189q16 9 25.5 21.5t8.5 25.5q-3 29 -41 47q-16 7 -50 10t-76 3v-122q37 -1 73.5 0.5t59.5 14.5z\" />" << std::endl;
os << "<glyph glyph-name=\"percent\" unicode=\"%\" horiz-adv-x=\"821\" d=\"M677 684q18 19 42 19q25 0 43 -19q19 -18 19 -43q0 -26 -19 -42l-578 -578q-16 -19 -43 -19q-26 0 -42 19q-19 16 -19 42q0 27 19 43zM324 550q0 -19 -7.5 -35.5t-20 -29t-29 -19.5t-34.5 -7q-38 0 -64 26.5t-26 64.5q0 18 7 34.5t19 29t28.5 19.5t35.5 7q18 0 34.5 -7t29 -19.5t20 -29t7.5 -34.5zM720 153q0 -19 -7 -35.5t-19.5 -29t-29 -19.5t-34.5 -7q-38 0 -64.5 26.5t-26.5 64.5q0 18 7 34.5t19.5 29t29 20t35.5 7.5q18 0 34.5 -7.5t29 -20t19.5 -29t7 -34.5z\" />" << std::endl;
os << "<glyph glyph-name=\"ampersand\" unicode=\"&#x26;\" horiz-adv-x=\"734\" d=\"M668 97q18 -12 22 -33.5t-9 -39.5q-12 -18 -33.5 -22t-39.5 9l-14 9q-35 -20 -80 -20h-211q-27 0 -55 2t-55.5 13t-53 36t-46.5 70q-5 11 -8.5 25t-3 31t7 36.5t20.5 41.5q12 18 26.5 33.5t29.5 29.5l-22 16q-31 21 -47 51t-14 65q2 42 26 78.5t63 59.5q36 21 78.5 23.5t79.5 1.5h23l62 -1q28 0 50.5 -8t40.5 -20t32 -26t25 -27l14 -16q15 -16 14.5 -38t-17.5 -37q-16 -14 -37.5 -13.5t-35.5 16.5l-16 18q-23 26 -36.5 36t-33.5 10l-62 1h-25q-27 1 -57 0t-47 -11q-15 -9 -25 -23.5t-11 -27.5q0 -5 2 -11.5t13 -14.5l375 -260q4 27 6 47.5t2 34.5q0 22 15 37t37 15t37.5 -15t15.5 -37q0 -34 -7 -75q-6 -43 -17 -68zM303 105h169l-219 152q-15 -14 -30.5 -29.5t-26.5 -30.5q-11 -16 -10.5 -23t2.5 -9q10 -23 20 -35t23.5 -17.5t30.5 -6.5t41 -1z\" />" << std::endl;
os << "<glyph glyph-name=\"quotesingle\" unicode=\"'\" horiz-adv-x=\"241\" d=\"M80 603q0 25 17 42.5t44 17.5q25 0 42.5 -17.5t17.5 -42.5v-147q0 -26 -17.5 -43.5t-42.5 -17.5q-27 0 -44 17.5t-17 43.5v147z\" />" << std::endl;
os << "<glyph glyph-name=\"parenleft\" unicode=\"(\" horiz-adv-x=\"331\" d=\"M188 685l-108 -109v-448l108 -108q18 -18 43 -18t42 18q9 8 13.5 19.5t4.5 22.5q0 25 -18 43l-73 73v348l73 73q18 18 18 43t-18 43q-17 17 -42 17t-43 -17z\" />" << std::endl;
os << "<glyph glyph-name=\"parenright\" unicode=\")\" horiz-adv-x=\"331\" d=\"M183 25l108 108v448l-108 109q-18 17 -43 17t-43 -17q-17 -19 -17 -43t17 -43l73 -73v-348l-73 -73q-17 -19 -17 -43q0 -25 17 -42q18 -18 43 -18t43 18z\" />" << std::endl;
os << "<glyph glyph-name=\"asterisk\" unicode=\"*\" horiz-adv-x=\"812\" d=\"M730 219q18 -13 22.5 -34.5t-7.5 -41.5q-13 -20 -35.5 -24t-40.5 9l-182 123v-192q0 -25 -18 -42.5t-43 -17.5t-42.5 17.5t-17.5 42.5v202l-201 -111q-19 -10 -41 -4t-32 26q-12 19 -5.5 40.5t25.5 32.5l211 118l-199 134q-20 13 -24 35t9 41q12 18 34 22.5t42 -8.5l181 -122v192q0 25 17.5 42.5t42.5 17.5t43 -17.5t18 -42.5v-204l200 112q20 11 41.5 5t33.5 -26q10 -19 4 -41t-26 -32l-209 -118z\" />" << std::endl;
os << "<glyph glyph-name=\"plus\" unicode=\"+\" horiz-adv-x=\"821\" d=\"M720 411q25 0 43 -17.5t18 -42.5t-18 -42.5t-43 -17.5h-230v-230q0 -25 -17.5 -42.5t-42.5 -17.5t-42.5 17.5t-17.5 42.5v230h-230q-25 0 -42.5 17.5t-17.5 42.5t17.5 42.5t42.5 17.5h230v230q0 25 17.5 43t42.5 18t42.5 -18t17.5 -43v-230h230z\" />" << std::endl;
os << "<glyph glyph-name=\"comma\" unicode=\",\" horiz-adv-x=\"350\" d=\"M207 188l-110 -113q-17 -17 -17 -43q0 -25 17 -42q18 -18 43 -17.5t43 17.5l110 112q17 18 17 43t-17 43q-18 17 -43 17t-43 -17z\" />" << std::endl;
os << "<glyph glyph-name=\"hyphen\" unicode=\"-\" horiz-adv-x=\"540\" d=\"M132 231h315q22 0 37.5 15t15.5 37t-15.5 37.5t-37.5 15.5h-315q-22 0 -37 -15.5t-15 -37.5t15 -37t37 -15z\" />" << std::endl;
os << "<glyph glyph-name=\"period\" unicode=\".\" horiz-adv-x=\"287\" d=\"M247 121q0 34 -24 58.5t-59 24.5q-18 0 -33 -6.5t-26.5 -18t-18 -26.5t-6.5 -32q0 -35 24.5 -59.5t59.5 -24.5q17 0 32 6.5t26.5 18t18 26.5t6.5 33z\" />" << std::endl;
os << "<glyph glyph-name=\"slash\" unicode=\"/\" horiz-adv-x=\"730\" d=\"M600 611q-129 -130 -217.5 -219.5t-146 -148.5t-89.5 -92.5t-46.5 -51t-17.5 -23.5t-3 -10q0 -20 16 -36q16 -15 37 -15q22 0 37 15l504 507q16 13 16 36q0 22 -16 37t-38 15t-36 -14z\" />" << std::endl;
os << "<glyph glyph-name=\"zero\" unicode=\"0\" horiz-adv-x=\"821\" d=\"M727 627q23 -23 38.5 -59t15.5 -70v-296q0 -34 -15.5 -70t-38.5 -59l-20 -20q-23 -23 -59 -38t-69 -15h-297q-34 0 -70 15t-59 38l-20 20q-23 23 -38 59t-15 70v296q0 34 15 70t38 59l20 20q23 23 59 38.5t70 15.5h297q33 0 69 -15.5t59 -38.5zM218 542q-5 -6 -11 -20t-6 -24v-194l452 218q-6 13 -11 20l-20 20q-6 5 -19.5 11.5t-23.5 6.5h-297q-10 0 -24 -6.5t-20 -11.5zM642 158q5 6 11.5 20t6.5 24v189l-451 -217q4 -9 9 -16l20 -20q6 -5 20 -11t24 -6h297q10 0 23.5 6t19.5 11z\" />" << std::endl;
os << "<glyph glyph-name=\"one\" unicode=\"1\" horiz-adv-x=\"622\" d=\"M141 118h121v374l-33 -33q-16 -19 -43 -19q-26 0 -42 19q-19 16 -19 41t19 44l131 131q3 4 9 9q2 2 5 3q17 11 33 11q27 0 47 -23q17 -22 13 -50v-507h139q25 0 43 -18t18 -43t-18 -42.5t-43 -17.5h-380q-26 0 -43.5 17.5t-17.5 42.5t17.5 43t43.5 18z\" />" << std::endl;
os << "<glyph glyph-name=\"two\" unicode=\"2\" horiz-adv-x=\"822\" d=\"M81 641q0 26 17.5 43t42.5 17l439 -1q34 0 69.5 -15.5t58.5 -38.5l20 -19q23 -23 38.5 -59.5t15.5 -69.5v-8q0 -33 -14.5 -69t-37.5 -59l-17 -18q-23 -23 -59.5 -38.5t-69.5 -15.5h-301q-10 0 -24 -6.5t-20 -12.5l-20 -19q-5 -6 -11 -20t-6 -23v-8q0 -9 6 -23.5t11 -20.5l20 -19q6 -6 20 -11.5t24 -5.5h438q25 0 43 -18t18 -43q0 -26 -18 -43t-43 -17h-438q-34 0 -70 15t-59 38l-20 19q-23 23 -38 59.5t-15 69.5v8q0 32 15 68.5t38 59.5l20 19q23 23 59 38t70 15h301q9 0 22.5 6.5t18.5 12.5l18 17q5 7 11.5 21t6.5 24v8q0 9 -6.5 23.5t-11.5 19.5l-20 20q-6 6 -20.5 12t-23.5 6l-438 2q-25 0 -43 17.5t-17 42.5z\" />" << std::endl;
os << "<glyph glyph-name=\"three\" unicode=\"3\" horiz-adv-x=\"821\" d=\"M727 626q23 -23 38.5 -59t15.5 -69v-297q0 -34 -15.5 -70t-38.5 -59l-20 -20q-23 -23 -59 -37.5t-68 -14.5h-439q-26 0 -43 17t-17 42q0 26 17 43.5t43 17.5h439q9 0 23 6t19 12l20 19q6 6 12 20.5t6 23.5v88h-429q-25 0 -43 17.5t-18 42.5t18 42.5t43 17.5h429v89q0 9 -6 23t-12 20l-20 20q-5 6 -19.5 12t-23.5 6h-438q-26 0 -43.5 18t-16.5 43q0 26 17 43t43 17h439q33 0 68.5 -15.5t58.5 -38.5z\" />" << std::endl;
os << "<glyph glyph-name=\"four\" unicode=\"4\" horiz-adv-x=\"821\" d=\"M720 318q25 0 43 -18t18 -43q-1 -26 -18.5 -43t-42.5 -17l-115 1v-138q0 -25 -17.5 -42.5t-42.5 -17.5t-42.5 17.5t-17.5 42.5v138l-205 1q-33 0 -68.5 15t-58.5 38l-20 20q-23 23 -38 59t-15 69v242q0 25 17.5 42.5t42.5 17.5q26 0 43.5 -17.5t17.5 -42.5v-242q0 -9 6 -23.5t11 -19.5l20 -20q6 -6 20 -11.5t24 -5.5l203 -1v321q0 25 17.5 43t42.5 18t42.5 -18t17.5 -43v-321z\" />" << std::endl;
os << "<glyph glyph-name=\"five\" unicode=\"5\" horiz-adv-x=\"821\" d=\"M191 701h529q25 0 43 -17.5t18 -42.5t-18 -43t-43 -18h-519v-169h379q31 0 66.5 -12.5t59.5 -32.5l13 -11q26 -22 44 -58.5t18 -70.5v-26q0 -33 -15.5 -69t-38.5 -59l-20 -20q-23 -23 -59 -38t-68 -15h-440q-25 0 -42.5 17.5t-17.5 42.5t17.5 43t42.5 18h440q9 0 22.5 6t19.5 11l20 20q5 6 11.5 20t6.5 23v26q0 8 -6 19.5t-11 15.5l-13 11q-8 7 -24.5 12.5t-25.5 5.5h-389q-23 0 -43 9t-35 24t-24 35t-9 43v189q0 23 9 43t24 35t35 24t43 9z\" />" << std::endl;
os << "<glyph glyph-name=\"six\" unicode=\"6\" horiz-adv-x=\"821\" d=\"M153 648q23 23 59 38.5t70 15.5l297 -1q34 0 69.5 -15.5t58.5 -38.5l20 -20q18 -18 36 -47t18 -58q0 -25 -18 -43t-43 -18q-23 0 -40.5 16.5t-19.5 39.5q-2 3 -6.5 10.5t-11.5 14.5l-20 19q-6 6 -20 12.5t-24 6.5l-297 1q-10 0 -23 -5.5t-19 -11.5l-21 -21q-5 -7 -11 -21.5t-6 -23.5v-88h378q33 0 69 -15t59 -38l20 -20q23 -23 38.5 -59t15.5 -68v-27q0 -34 -18 -70.5t-44 -58.5l-13 -10q-24 -20 -59.5 -32t-67.5 -12h-297q-34 0 -70 15t-59 38l-20 20q-23 23 -38 59t-15 70v296q0 34 14.5 70t37.5 59zM579 290h-378v-88q0 -10 6 -24t11 -20l20 -20q6 -6 20 -11.5t24 -5.5h297q11 0 27 5.5t25 11.5l12 11q6 4 11.5 15.5t5.5 18.5v27q0 9 -6.5 22.5t-11.5 19.5l-20 20q-6 5 -19.5 11.5t-23.5 6.5z\" />" << std::endl;
os << "<glyph glyph-name=\"seven\" unicode=\"7\" horiz-adv-x=\"821\" d=\"M80 640q0 25 17.5 42.5t42.5 17.5l439 -1q34 0 69.5 -15.5t58.5 -38.5l20 -20q23 -23 38.5 -58t15.5 -65q0 -33 -18 -67.5t-44 -56.5l-13 -9q-24 -20 -59.5 -32.5t-67.5 -12.5h-8q-9 0 -23.5 -6t-19.5 -13l-20 -18q-9 -9 -19 -26.5t-12 -29.5l-42 -187q-6 -24 -27.5 -37t-45.5 -7q-21 5 -34 21.5t-13 36.5q0 2 0.5 5.5t1.5 8.5l43 186q7 29 24.5 61t38.5 53l20 20q23 23 59 38t69 15h8q11 0 27 5.5t25 12.5l12 9q6 6 11 15t6 15q-1 6 -6 18.5t-12 19.5l-20 20q-6 5 -20 11.5t-24 6.5l-438 1q-25 0 -42.5 18t-17.5 43z\" />" << std::endl;
os << "<glyph glyph-name=\"eight\" unicode=\"8\" horiz-adv-x=\"821\" d=\"M781 491q0 -32 -15.5 -68t-38.5 -59l-13 -13l13 -13q23 -23 38.5 -59t15.5 -68v-8q0 -34 -15.5 -70t-38.5 -59l-20 -20q-23 -23 -59 -38t-69 -15h-297q-34 0 -70 15t-59 38l-20 20q-23 23 -38 59t-15 70v8q0 32 15 68t38 59l13 13l-13 13q-23 23 -38 59t-15 68v8q0 34 15 70t38 59l20 20q23 23 59 38.5t70 15.5h297q33 0 69 -15.5t59 -38.5l20 -20q23 -23 38.5 -59t15.5 -70v-8zM201 499v-8q0 -9 6 -23t11 -19l20 -20q6 -6 20 -12t24 -6h297q10 0 23.5 6t19.5 12l20 20q5 5 11.5 19t6.5 23v8q0 10 -6.5 24t-11.5 20l-20 20q-6 5 -19.5 11.5t-23.5 6.5h-297q-10 0 -24 -6.5t-20 -11.5l-20 -20q-5 -6 -11 -20t-6 -24zM660 203v8q0 9 -6.5 23t-11.5 19l-20 20q-6 6 -19.5 12t-23.5 6h-297q-10 0 -24 -6t-20 -12l-20 -20q-5 -5 -11 -19t-6 -23v-8q0 -10 6 -24t11 -20l20 -20q6 -5 20 -11t24 -6h297q10 0 23.5 6t19.5 11l20 20q5 6 11.5 20t6.5 24z\" />" << std::endl;
os << "<glyph glyph-name=\"nine\" unicode=\"9\" horiz-adv-x=\"822\" d=\"M782 497v-296q0 -34 -15.5 -70t-38.5 -59l-20 -20q-23 -23 -59 -38t-69 -15h-297q-34 0 -70 15t-59 38l-55 56q-19 16 -19 41t19 44q17 17 42 17q24 0 43 -17l55 -56q6 -5 20 -11t24 -6h297q10 0 23.5 6t19.5 11l20 20q5 6 11.5 20t6.5 24v88h-378q-34 0 -70 15t-59 38l-20 20q-23 23 -38 59t-15 68v8q0 34 15 70t38 59l20 20q23 23 59 38.5t70 15.5h297q33 0 69 -15.5t59 -38.5l20 -20q23 -23 38.5 -59t15.5 -70zM239 427q6 -6 20 -12t24 -6h378v88q0 10 -6.5 24t-11.5 20l-20 20q-6 5 -19.5 11.5t-23.5 6.5h-297q-10 0 -24 -6.5t-20 -11.5l-20 -20q-5 -6 -11 -20t-6 -24v-8q0 -9 6 -23t11 -19z\" />" << std::endl;
os << "<glyph glyph-name=\"colon\" unicode=\":\" horiz-adv-x=\"287\" d=\"M247 190q0 34 -24 58.5t-59 24.5q-18 0 -33 -6.5t-26.5 -18t-18 -26.5t-6.5 -32q0 -35 24.5 -59.5t59.5 -24.5q17 0 32 6.5t26.5 18t18 26.5t6.5 33zM247 474q0 34 -24 58.5t-59 24.5q-18 0 -33 -6.5t-26.5 -18t-18 -26.5t-6.5 -32q0 -35 24.5 -59.5t59.5 -24.5q17 0 32 6.5t26.5 18t18 26.5t6.5 33z\" />" << std::endl;
os << "<glyph glyph-name=\"semicolon\" unicode=\";\" horiz-adv-x=\"355\" d=\"M315 459q0 34 -24 58.5t-59 24.5q-18 0 -33 -6.5t-26.5 -18t-18 -26.5t-6.5 -32q0 -35 24.5 -59.5t59.5 -24.5q17 0 32 6.5t26.5 18t18 26.5t6.5 33zM207 246l-110 -113q-17 -17 -17 -43q0 -25 17 -42q18 -18 43 -17.5t43 17.5l110 112q17 18 17 43t-17 43q-18 17 -43 17t-43 -17z\" />" << std::endl;
os << "<glyph glyph-name=\"less\" unicode=\"&#x3c;\" horiz-adv-x=\"445\" d=\"M384 196l-136 134l134 136q20 20 20 49t-20 49t-49 20t-49 -20l-183 -185q-21 -21 -21 -49q1 -14 5.5 -26.5t15.5 -22.5v-1l185 -182q20 -20 49 -20t49 20q21 20 20.5 49t-20.5 49z\" />" << std::endl;
os << "<glyph glyph-name=\"equal\" unicode=\"=\" horiz-adv-x=\"495\" d=\"M80 500h375v-125h-375v125zM80 250h375v-125h-375v125z\" />" << std::endl;
os << "<glyph glyph-name=\"greater\" unicode=\"&#x3e;\" horiz-adv-x=\"445\" d=\"M101 466q-20 20 -20.5 49t20.5 49q20 20 49 20t49 -20l185 -182v-1q11 -10 15.5 -22.5t5.5 -26.5q0 -28 -21 -49l-183 -185q-20 -20 -49 -20t-49 20t-20 49t20 49l134 136z\" />" << std::endl;
os << "<glyph glyph-name=\"question\" unicode=\"?\" horiz-adv-x=\"821\" d=\"M522 91q0 38 -26.5 64.5t-64.5 26.5t-64 -26.5t-26 -64.5t26 -64.5t64 -26.5t64.5 26.5t26.5 64.5zM745 605q-14 14 -27.5 28.5t-28.5 27t-32 21.5t-38 12q-23 3 -46 2t-46 -1h-267q-22 0 -40 -6.5t-34 -17t-30 -24.5l-28 -28q-17 -17 -28.5 -37.5t-16.5 -44.5q-3 -17 -2.5 -35.5t0.5 -36.5q0 -23 9 -42t33 -27q14 -5 28 -2.5t25.5 11t18.5 21t7 27.5v61q0 3 5 9t11.5 13t13.5 13.5t11 10.5q7 8 11.5 11t14.5 3h49h160h119q10 0 17 -5.5t14 -11.5q9 -9 20.5 -21.5t11.5 -26.5q0 -5 0.5 -14.5t0.5 -20.5v-20t-1 -12q-3 -7 -10 -14.5t-12 -12.5l-16.5 -16.5t-18.5 -11.5q-28 0 -56 0.5t-56 0.5h-120v-25v-54v-55v-24h120v37h14t33.5 -0.5t40.5 -0.5h37q18 0 32.5 4t27.5 17q25 25 54 51.5t42 60.5q8 19 8.5 38.5t0.5 39.5v38t-5 37q-9 31 -31 53z\" />" << std::endl;
os << "<glyph glyph-name=\"at\" unicode=\"@\" horiz-adv-x=\"822\" d=\"M764 193q-19 19 -45 17q9 10 17 18l8 8q16 16 26 39.5t10 46.5l2 198v1q0 22 -9.5 45.5t-24.5 40.5l-54 57q-17 17 -41 27.5t-47 10.5h-347q-23 0 -46.5 -9.5t-40.5 -26.5l-56 -57q-16 -16 -26 -40t-10 -47v-346q0 -23 10.5 -47t27.5 -41l55 -53q16 -16 39.5 -25.5t46.5 -9.5h347q23 0 46.5 9.5t40.5 26.5l71 71q18 18 18 43t-18 43zM659 325q0 -2 -2 -6l-50 -50q-4 -2 -5 -2h-18v267h-121h-24q-23 0 -47 -9.5t-41 -26.5l-54 -53q-17 -17 -26.5 -41t-9.5 -47v-34q0 -23 9.5 -47t26.5 -41l54 -53q17 -17 41 -26.5t47 -9.5h165q15 0 33 5l-28 -28q-2 -2 -5 -2h-343q-2 0 -6 2q-1 1 -9 8.5t-17 16.5l-17 17l-9 9q-2 2 -2 4v343q1 1 1 2t1 3l54 54q2 0 4 2h343l1 -1q1 0 2 -1t9 -9.5t17 -18.5t17 -18t8 -9q1 -1 1.5 -2.5t1.5 -3.5v-16q0 -14 -0.5 -35t-0.5 -46t-0.5 -46t-0.5 -36v-15zM463 267h-23q-3 0 -5 2q-1 1 -9 8.5t-17 17t-17 17t-9 8.5q0 1 -2 5v30q2 4 2 5q1 1 9 8.5t17 17t17 17t9 8.5q2 2 5 2h23v-146z\" />" << std::endl;
os << "<glyph glyph-name=\"A\" unicode=\"A\" horiz-adv-x=\"812\" d=\"M242 643q27 36 78 56h40h6h8h6h10h96h26h14h12q13 -2 24 -6h6q5 -4 10.5 -7t13.5 -7q6 -8 18 -16l2 -4l4 -2q1 -1 1.5 -2t2.5 -2l8 -12l142 -548l2 -18v-34q-12 -21 -36 -36q-37 -6 -68 16q-4 10 -10 16l-4 12q0 13 -30 126l-10 38l-2 6h-24h-6h-34h-8h-8h-230h-8h-6h-10h-6h-28l-8 -32l-2 -4l-2 -8l-32 -114l-8 -28q-6 -9 -18 -18l-2 -4q-58 -22 -92 30v32q0 3 0.5 7t1.5 9l154 540q2 4 6 14zM378 581h-28q-13 -24 -72 -242h24h242h26h6h6q-2 8 -4 14q-23 97 -35.5 149.5t-16.5 60.5l-2 10l-4 8h-24h-6h-102h-10z\" />" << std::endl;
os << "<glyph glyph-name=\"B\" unicode=\"B\" horiz-adv-x=\"821\" d=\"M80 0v692h510h6l26 -6l6 -2l4 -2h6l4 -4h6q107 -51 124 -170q9 -90 -58 -160q60 -61 58 -154q-8 -132 -144 -186l-32 -6l-6 -3zM206 572h-8v-164h374h6h8q67 21 66 94q-15 51 -70 70h-376zM584 286h-386v-166h380l14 4q62 23 60 86q-14 56 -68 76z\" />" << std::endl;
os << "<glyph glyph-name=\"C\" unicode=\"C\" horiz-adv-x=\"827\" d=\"M95 451q-15 178 166 238h328h6h6h16l14 -2q92 -33 156 -124v-32v-6v-6q-22 -40 -72 -38q-12 3 -22 12l-6 2q-5 7 -12 14t-16 14l-10 12l-4 4q-4 2 -7.5 6.5t-8.5 9.5l-16 10l-24 6h-266h-20q-7 1 -16 -2h-6l-6 -4h-6q-25 -14 -42 -40l-4 -2q-2 -5 -4 -10.5t-4 -13.5l-2 -4v-24v-6v-260q0 -15 6 -34q23 -39 70 -56h24h6h268l14 2q34 16 64 52q4 2 7 5.5t7 8.5l12 8l4 4l4 2l4 4q2 0 4 2l14 2h16l14 -2q22 -12 36 -38v-2v2v-4v2v-4v2v-4v2v-4v2v-4v2v-4v2v-2v-16v-2v2v-4v2v-4v2v-4v2v-4v2v-2q-53 -86 -156 -126l-12 -2h-276h-6q-110 -15 -192 70q-26 28 -44 74q-2 8 -4 12l-2 12v286z\" />" << std::endl;
os << "<glyph glyph-name=\"D\" unicode=\"D\" horiz-adv-x=\"824\" d=\"M91 613q-11 51 36 80h24h6h426h6q33 0 66 -16h6q67 -34 108 -110l10 -32l2 -4q3 -18 2 -40v-320l-14 -50q-4 -5 -7.5 -11.5t-6.5 -14.5l-4 -2l-2 -4l-2 -6q-5 -4 -10 -9.5t-10 -12.5q-4 -2 -7 -5.5t-7 -8.5q-8 -3 -14 -12q-5 -2 -10 -5t-10 -7q-11 -5 -24 -12q-11 -2 -22 -8q-2 0 -6 -2l-12 -2h-436h-8h-14h-14h-10q-14 5 -20 9l-10 6q-9 7 -12 21q-9 91 88 82v458q-57 -11 -88 38zM313 573h-14v-448v-8h252q26 -3 58 6q59 35 56 108v252q0 14 -6 34q-6 10 -15.5 20.5t-22.5 23.5q-16 9 -40 12h-14h-254z\" />" << std::endl;
os << "<glyph glyph-name=\"E\" unicode=\"E\" horiz-adv-x=\"826\" d=\"M88 509v-282q-8 -182 170 -228h16h8h36h422q41 8 40 62q-2 57 -40 57h-411l-30 2q-81 16 -96 83l1 82h451q33 9 34 60t-30 60h-453v89q18 54 76 76l23 5h444q37 16 34 64q-1 24 -10 39t-26 15h-475q-147 0 -184 -184z\" />" << std::endl;
os << "<glyph glyph-name=\"F\" unicode=\"F\" horiz-adv-x=\"819\" d=\"M94 571q50 96 162 124h468q55 -11 54 -66t-67 -55l-433 1l-14 -4q-85 -34 -69 -163h232q57 -4 59 -63q1 -57 -58 -58h-230v-240q-2 -50 -57 -49q-56 1 -60 51l-1 478q2 13 5.5 23.5t8.5 20.5z\" />" << std::endl;
os << "<glyph glyph-name=\"G\" unicode=\"G\" horiz-adv-x=\"837\" d=\"M95 474q31 175 168 216h342q54 -2 102 -29q51 -27 70 -79q20 -54 16 -91q-2 -59 -54 -54q-53 5 -56 59q13 87 -146 76h-240l-24 -8q-44 -36 -51 -70q-4 -17 -5.5 -92t0.5 -210l4 -25q13 -29 48 -45q38 -9 400 -6v156h-250q-64 -3 -69 56q-3 59 51 65l338 -1q40 -1 48 -38v-274l-2 -10q-32 -77 -122 -72h-14h-358q-7 -1 -14.5 -0.5t-17.5 2.5h-8q-171 58 -156 252v222z\" />" << std::endl;
os << "<glyph glyph-name=\"H\" unicode=\"H\" horiz-adv-x=\"816\" d=\"M80 660q8 56 58 54q51 0 60 -54v-252h456v218q-3 67 60 67q62 0 58 -69v-580q-5 -48 -51 -49q-47 -2 -65 43l-2 23v227h-456l-2 -248q-7 -40 -53 -42q-44 -2 -63 48v614z\" />" << std::endl;
os << "<glyph glyph-name=\"I\" unicode=\"I\" horiz-adv-x=\"236\" d=\"M80 660q7 39 57 41t59 -41v-620q-7 -42 -57 -40q-48 1 -59 40v620z\" />" << std::endl;
os << "<glyph glyph-name=\"J\" unicode=\"J\" horiz-adv-x=\"852\" d=\"M148 574q-41 4 -46 51t43 67h612q55 -9 54 -66q-2 -56 -55 -53l-61 1l-1 -408q-7 -81 -46.5 -121.5t-120.5 -44.5h-268l-12 2q-167 33 -156 240q5 50 60 49q56 0 58 -57q-16 -118 136 -118h148q8 2 21.5 7t27 14t23 22.5t10.5 34.5v380h-427z\" />" << std::endl;
os << "<glyph glyph-name=\"K\" unicode=\"K\" horiz-adv-x=\"822\" d=\"M80 658q11 34 55 35q45 3 62 -35l1 -206l411 243l130 1q43 -12 43 -66t-40 -57l-102 1l-226 -134l240 -322h65q47 -18 44 -66q-3 -47 -45 -52h-124l-284 376l-112 -64l-2 -276q-10 -40 -59 -39t-57 43v618z\" />" << std::endl;
os << "<glyph glyph-name=\"L\" unicode=\"L\" horiz-adv-x=\"813\" d=\"M80 656q9 40 58 40q49 1 60 -39v-461q16 -64 68 -78l470 -2q35 -14 36 -61q1 -48 -36 -57h-490q-49 5 -103 44q-56 40 -63 132v482z\" />" << std::endl;
os << "<glyph glyph-name=\"M\" unicode=\"M\" horiz-adv-x=\"974\" d=\"M750 693h-442q-182 8 -228 -170v-16v-8v-36v-422q8 -43 62 -41q57 2 57 41v411l2 30q16 81 83 96l162 -1v-451q9 -33 60 -35q51 -1 60 31v453h169q54 -18 76 -76l5 -23v-444q16 -37 64 -34q49 2 53 36l1 475q0 147 -184 184z\" />" << std::endl;
os << "<glyph glyph-name=\"N\" unicode=\"N\" horiz-adv-x=\"812\" d=\"M80 52v648h516q102 -36 134 -83q33 -46 42 -95v-470q-4 -55 -58 -54q-53 2 -60 52v448q-6 26 -18.5 45.5t-46.5 37.5l-391 1l-3 -532q-5 -47 -59 -48q-54 -2 -56 50z\" />" << std::endl;
os << "<glyph glyph-name=\"O\" unicode=\"O\" horiz-adv-x=\"830\" d=\"M112 568q47 91 152 122h24h6h262h12h40h16q8 -2 17.5 -5.5t20.5 -8.5h6q58 -29 96 -90l4 -2l10 -24l2 -6l6 -14q0 -2 2 -6v-6l2 -6v-340l-2 -28q-54 -161 -222 -156h-298l-14 2q-174 60 -156 260v258l2 12l6 18q4 9 6 20zM564 572h-272q-38 -12 -66 -48q-5 -12 -10 -32v-24v-274q2 -8 5.5 -17t8.5 -19q15 -17 42 -36q9 -2 315 0q82 12 85 92v268q0 14 -6 34q-7 16 -26 30l-10 12q-14 8 -32 12l-4 2h-14h-16z\" />" << std::endl;
os << "<glyph glyph-name=\"P\" unicode=\"P\" horiz-adv-x=\"826\" d=\"M86 632q23 41 74 62h24h6h34h8h8h8h8h22h36h8h12h10h12h34h12h14h12h14h12h14h40h6h8h6h10h22h8q28 2 56 -8h6q120 -46 142 -176v-62q14 -145 -122 -222l-26 -8l-4 -2l-16 -4l-8 -2h-6h-14h-16h-34h-8h-26h-10h-22h-24h-6h-34h-8h-26h-24h-6h-34h-12h-14h-40h-6h-8h-20v-24v-6v-34v-8v-26v-36v-6v-8l-2 -26q-44 -71 -110 -12l-6 12v574q2 9 6 16v6zM212 576h-14v-24v-6v-34v-8v-26v-36v-6v-8v-8v-62v-6v-6v-16h24h6h34h8h26h10h22h24h6h34h8h26h24h6h34h12h62h6l8 2h8l12 4q55 29 56 94q20 107 -76 144h-14h-16h-34h-8h-26h-10h-22h-24h-6h-34h-8h-26h-24h-6h-34h-20q-4 1 -10 1t-14 1h-14h-8h-8z\" />" << std::endl;
os << "<glyph glyph-name=\"Q\" unicode=\"Q\" horiz-adv-x=\"859\" d=\"M101 440q-21 187 166 250h24h6h34h8h26h36h6h8h40h6h8h24h6h34h62h6h6h16l14 -2q155 -51 158 -220v-24v-6v-34v-8v-26v-10v-22v-24v-6v-34v-8v-26v-24v-6v-34l-12 -46l-4 -4l-2 -6v-6q42 -38 20 -96q-16 -19 -42 -32q-43 -7 -76 26l-8 2l-44 -16l-12 -2h-14h-6h-6h-6h-14h-6h-8h-6h-6h-22h-8h-20h-8h-40h-6h-6h-40h-6h-8h-6h-10h-6q-146 -20 -234 108l-10 24l-2 6l-6 14l-2 6l-2 12v274zM335 572h-40q-41 -13 -70 -56q-9 -32 -6 -78v-6v-16v-6v-6v-20v-6v-62v-28q-18 -122 56 -166q31 -9 78 -6h6h16h6h6h6h6h20h22h8h8h24h6h34h8h10h34q-4 5 -8.5 10t-11.5 10l-2 4l-4 2l-10 10l-4 6l-4 2l-4 6l-6 4l-6 8l-4 4l-6 6l-8 6l-12 14l-8 6q-10 13 -28 28q0 3 -4 4q-11 13 -30 30q0 3 -4 4l-2 4q-13 12 -26 30q-11 38 10 78q35 30 84 20q23 -14 50 -44q5 -6 16 -14l2 -4l4 -4l4 -4l4 -4q5 -6 16 -14l2 -4l4 -2l2 -4l4 -4l6 -6l6 -6l24 -22q0 -3 4 -4l2 -4l4 -4l4 -4l4 -4l4 -4l6 -6l26 -26l2 18v244q0 14 -6 34q-7 14 -26 30l-10 12q-14 8 -32 12l-4 2h-14h-16h-34h-8h-26h-10h-8h-34h-14h-16h-34h-8h-26h-14z\" />" << std::endl;
os << "<glyph glyph-name=\"R\" unicode=\"R\" horiz-adv-x=\"837\" d=\"M86 630q23 43 74 62h408q19 3 42 -4h6l14 -4q124 -52 142 -180q25 -202 -138 -280q3 -5 32 -33.5t84 -82.5q0 -3 4 -4q2 -6 12 -14q1 -6 6 -12q9 -51 -36 -80h-26h-6h-6h-6q-8 4 -16.5 11t-17.5 17l-186 186h-274v-152l-2 -26q-38 -62 -104 -20l-12 24v570l2 4v8l4 4v6zM558 574h-360v-242h374h6l14 4q51 28 62 94q19 106 -82 144h-14z\" />" << std::endl;
os << "<glyph glyph-name=\"S\" unicode=\"S\" horiz-adv-x=\"868\" d=\"M122 427q37 -93 118 -130q2 0 8 -2l36 -10l314 -2h14q60 -22 76 -76q-3 -65 -60 -88h-6l-16 -4l-258 2q-40 -5 -70.5 13t-70.5 49q-60 47 -92 -3q-31 -49 65 -123l6 -4q48 -40 98 -50h10l342 -2l34 8q114 21 136 150q13 187 -156 244l-40 4h-6h-6q-140 -1 -214 -0.5t-84 2.5q-62 25 -66 81q-3 56 67 87h277q15 1 27.5 -0.5t22.5 -3.5l18 -10l23 -18l43 -50q47 -27 82 -4q34 24 12 76q-61 88 -148 124l-16 4h-16h-6h-6h-298h-6h-24q-206 -69 -164 -264z\" />" << std::endl;
os << "<glyph glyph-name=\"T\" unicode=\"T\" horiz-adv-x=\"818\" d=\"M126 581q-40 10 -43 52t43 64h608q44 -14 42 -62q0 -48 -44 -56h-236v-540q-7 -37 -59 -39q-51 0 -61 42v539h-250z\" />" << std::endl;
os << "<glyph glyph-name=\"U\" unicode=\"U\" horiz-adv-x=\"813\" d=\"M80 658q9 37 55 38q48 2 61 -38l2 -464l8 -24l2 -4q12 -16 25 -28t41 -20h292l24 6q34 18 60 56q6 28 4 78v402q7 39 57 39q49 0 61 -35v-480q1 -40 -20 -72q-47 -72 -138 -108q-16 -4 -44 -4h-238q-162 -20 -242 132q-4 10 -6.5 21t-3.5 23v482z\" />" << std::endl;
os << "<glyph glyph-name=\"V\" unicode=\"V\" horiz-adv-x=\"823\" d=\"M80 630q4 49 47 60q42 10 70 -36l156 -502q10 -27 33 -36l80 1q24 1 33 36q82 250 162 513q31 42 76 24q46 -16 41 -64l-169 -542q-10 -33 -30 -53.5t-60 -30.5h-184q-32 8 -56 31t-40 71z\" />" << std::endl;
os << "<glyph glyph-name=\"W\" unicode=\"W\" horiz-adv-x=\"974\" d=\"M264 -3h442q182 -8 228 170v16v8v36v422q-7 42 -63 40t-56 -40v-411l-2 -30q-16 -81 -83 -96l-162 1v451q-9 33 -60 34t-60 -30v-453h-169q-54 18 -76 76l-5 23v444q-16 37 -65 34q-49 -2 -53 -36v-475q0 -147 184 -184z\" />" << std::endl;
os << "<glyph glyph-name=\"X\" unicode=\"X\" horiz-adv-x=\"838\" d=\"M216 566q-7 6 -38 4q-29 -2 -55 6q-24 7 -32 50q-7 42 37 68h111q42 -12 69 -48l130 -190l138 194q38 52 98 44l76 1q48 -10 46 -58q-2 -47 -29 -53q-14 -2 -31.5 -4t-39.5 -4q-18 0 -32 -8l-152 -222l158 -230l45 -4q70 1 75 -28q6 -27 1 -57q-4 -28 -31 -29h-97q-43 -7 -71 30l-152 210l-158 -210q-28 -31 -56 -28h-98q-42 12 -45 46t11 51q13 16 77 21l39 6l156 222z\" />" << std::endl;
os << "<glyph glyph-name=\"Y\" unicode=\"Y\" horiz-adv-x=\"814\" d=\"M82 660q12 18 36 32q40 7 72 -24q1 -6 8 -18q0 -177 10 -198q12 -15 34 -34l4 -4l4 -2l4 -2l31 -8h286l27 8q8 4 16 10.5t16 15.5l4 2l4 4l12 24l2 4q1 7 1.5 15t0.5 17v140q3 13 10 28q37 36 90 12q10 -11 18 -28q0 -2 2 -4v-24v-166q-4 -10 -6 -26q-71 -174 -282 -150v-228v-12q-14 -41 -60 -44q-42 1 -60 44v240q-213 -29 -284 158l-2 12v194z\" />" << std::endl;
os << "<glyph glyph-name=\"Z\" unicode=\"Z\" horiz-adv-x=\"847\" d=\"M128 698h430h40l16 -2l14 -2q179 -71 116 -204q-12 -13 -30 -26l-470 -326q16 -13 80 -19h408q55 -21 54 -63q-2 -43 -48 -56h-480q-76 3 -114 29q-36 26 -50 71q-14 47 26 88q13 20 510 368q-16 14 -70 22h-422q-37 16 -40 57q-2 42 30 63z\" />" << std::endl;
os << "<glyph glyph-name=\"bracketleft\" unicode=\"[\" horiz-adv-x=\"385\" d=\"M141 701h-61v-702h198q26 0 43 17.5t17 42.5t-17 42.5t-43 17.5h-77v461h84q25 0 42.5 17.5t17.5 43.5q0 25 -17.5 42.5t-42.5 17.5h-144z\" />" << std::endl;
os << "<glyph glyph-name=\"backslash\" unicode=\"\\\" horiz-adv-x=\"730\" d=\"M95 524q129 -129 218.5 -217.5t148.5 -146t92.5 -89.5t51 -46.5t23 -17.5t9.5 -3q22 0 37 16q15 15 15 37q0 23 -15 37l-507 504q-15 16 -36 16q-23 0 -37.5 -16t-14.5 -38t15 -36z\" />" << std::endl;
os << "<glyph glyph-name=\"bracketright\" unicode=\"]\" horiz-adv-x=\"385\" d=\"M285 0h60v702h-198q-25 0 -42.5 -17.5t-17.5 -42.5q0 -26 17.5 -43.5t42.5 -17.5h77v-461h-83q-26 0 -43.5 -17.5t-17.5 -42.5t17.5 -42.5t43.5 -17.5h144z\" />" << std::endl;
os << "<glyph glyph-name=\"asciicircum\" unicode=\"^\" horiz-adv-x=\"638\" d=\"M339 652q-27 0 -44 -20l-198 -219q-17 -19 -15.5 -44t20.5 -42t43.5 -15.5t41.5 20.5l153 170q20 -22 46.5 -51t50 -54.5t39.5 -43.5l16 -18q17 -18 42 -19t44 16q18 17 19 41.5t-16 43.5l-197 215q-17 20 -45 20z\" />" << std::endl;
os << "<glyph glyph-name=\"underscore\" unicode=\"_\" horiz-adv-x=\"820\" d=\"M140 121q-25 0 -42.5 -17.5t-17.5 -42.5t17.5 -42.5t42.5 -17.5h580q25 0 42.5 17.5t17.5 42.5t-17.5 42.5t-42.5 17.5h-580z\" />" << std::endl;
os << "<glyph glyph-name=\"grave\" unicode=\"`\" horiz-adv-x=\"353\" d=\"M97 407q46 -45 74 -70.5t44 -38t24 -15.5t14 -3q25 0 42 17q18 18 17.5 43t-17.5 43l-112 110q-19 17 -43 17t-43 -17q-17 -19 -17 -43t17 -43z\" />" << std::endl;
os << "<glyph glyph-name=\"a\" unicode=\"a\" d=\"M652 377l-59 49q-15 12 -35 19.5t-39 7.5h-275q-18 0 -37.5 -7t-33.5 -18l-45 -32q-18 -13 -21.5 -34.5t9.5 -38.5q13 -18 34 -21.5t39 9.5l46 32l2 2l4 2t4 1h273q2 0 4 -1l4 -2l8.5 -7.5t19 -16t19.5 -16t10 -8.5h1v-17h-452q-22 0 -37 -15.5t-15 -37.5v-70q0 -21 9.5 -42t25.5 -35l60 -51q14 -12 34.5 -19.5t39.5 -7.5h270q17 0 35 5.5t32 15.5v-23h103v152v131v16q0 21 -10 42.5t-27 35.5zM584 155v-1q-2 -1 -10.5 -8t-19 -15t-19 -14.5l-8.5 -6.5q-2 -2 -4.5 -2.5t-4.5 -0.5h-268q-5 0 -7 2q-1 0 -9.5 7.5t-19 16.5t-19.5 16.5t-10 8.5v2v15h399v-20z\" />" << std::endl;
os << "<glyph glyph-name=\"b\" unicode=\"b\" d=\"M132 610q-22 0 -37 -15t-15 -37v-389q0 -20 8.5 -41t22.5 -35l62 -62q14 -14 35 -22.5t41 -8.5h270q20 0 41 8.5t35 22.5l62 62q14 14 23 35t9 41v112q0 20 -9 41t-23 35l-62 62q-14 14 -35 23t-41 9h-334v107q0 22 -15.5 37t-37.5 15zM517 346q1 0 5 -2q0 -1 9.5 -10t20.5 -20l20 -20l10 -10q0 -2 2 -4v-109q-2 -4 -2 -5q-1 0 -10 -9.5t-20 -20.5t-20.5 -20t-9.5 -10q-2 0 -2.5 -0.5t-2.5 -0.5h-266q-2 0 -2.5 0.5t-2.5 0.5q0 1 -9.5 10t-20.5 20t-20 20.5t-10 9.5q0 2 -0.5 2.5t-0.5 2.5v175h332z\" />" << std::endl;
os << "<glyph glyph-name=\"c\" unicode=\"c\" d=\"M249 453q-20 0 -41 -9t-35 -23l-62 -62q-14 -14 -22.5 -35t-8.5 -41v-112q0 -20 8.5 -41t22.5 -35l62 -62q14 -14 35 -22.5t41 -8.5h270q20 0 41 8.5t35 22.5l78 78q16 16 16 37q0 22 -16 38q-15 15 -37 15t-37 -15l-12 -12t-26 -26.5t-26.5 -26.5t-12.5 -13q-2 0 -2.5 -0.5t-2.5 -0.5h-266q-2 0 -2.5 0.5t-2.5 0.5l-9 9.5t-20.5 21t-21 20.5l-9.5 9q0 2 -0.5 2.5t-0.5 2.5v109q0 1 0.5 2t0.5 2l9.5 9.5t21 20.5t20.5 20.5l9 9.5q6 2 5 2h266q-1 0 5 -2q0 -1 12.5 -13t26.5 -26.5t26 -26l12 -11.5q15 -16 37 -16t37 16q16 15 16 37t-16 37l-78 78q-14 14 -35 23t-41 9h-270z\" />" << std::endl;
os << "<glyph glyph-name=\"d\" unicode=\"d\" d=\"M584 562v-107h-335q-20 0 -41 -9t-35 -23l-62 -62q-14 -14 -22.5 -35t-8.5 -41v-112q0 -20 8.5 -41t22.5 -35l62 -62q14 -14 35 -22.5t41 -8.5h270q20 0 41 8.5t35 22.5l62 62q14 14 23 35t9 41v389q0 22 -15.5 37t-37.5 15t-37 -15t-15 -37zM584 350v-175q-2 -4 -2 -5q-1 0 -10 -9.5t-20 -20.5t-20.5 -20t-9.5 -10q-2 0 -2.5 -0.5t-2.5 -0.5h-266q-2 0 -2.5 0.5t-2.5 0.5q0 1 -9.5 10t-20.5 20t-20 20.5t-10 9.5q0 2 -0.5 2.5t-0.5 2.5v109q0 1 0.5 2t0.5 2l10 10l20 20t20.5 20t9.5 10q4 2 5 2h333z\" />" << std::endl;
os << "<glyph glyph-name=\"e\" unicode=\"e\" d=\"M249 450q-18 0 -38.5 -7t-34.5 -19l-59 -46q-17 -14 -27 -35.5t-10 -42.5v-147q0 -21 9.5 -42.5t26.5 -35.5l59 -50q15 -12 35 -19t39 -7h270q19 0 39 7t35 19q-1 -1 -2 -1l-1 -1l45 33q18 13 21.5 34t-9.5 39t-34 21.5t-39 -9.5l-46 -33l-2 -2q-4 -2 -8 -2h-268q-4 0 -8 2l-8.5 7.5t-19 16t-19.5 16t-10 8.5v1v17h451q22 0 37.5 15t15.5 37v70q0 21 -10 42.5t-26 35.5l-60 51q-14 12 -34.5 19.5t-39.5 7.5h-270zM518 345q4 0 7 -3q1 0 9.5 -7.5t19 -16.5t19.5 -16.5t10 -8.5l1 -1v-15h-399v20q1 1 10 8t19 15t18.5 15l8.5 7q2 1 4.5 2t4.5 1h268z\" />" << std::endl;
os << "<glyph glyph-name=\"f\" unicode=\"f\" horiz-adv-x=\"594\" d=\"M502 609l-119 1q-20 0 -40.5 -7t-35.5 -20l-59 -51q-17 -14 -26.5 -35t-9.5 -43v-35h-79q-22 0 -37.5 -15t-15.5 -37t15.5 -37t37.5 -15h79v-263q0 -22 15 -37t37 -15t37.5 15t15.5 37v263h89q22 0 37 15t15 37t-15 37t-37 15h-89v33v1q1 1 10 8.5t19.5 16.5t19 16.5t9.5 7.5q1 1 3 1.5t3 1.5h19q19 0 41 -0.5t40 -0.5h18q22 -1 37.5 14t16.5 37t-14 37.5t-37 16.5z\" />" << std::endl;
os << "<glyph glyph-name=\"g\" unicode=\"g\" d=\"M689 295q0 21 -10 42.5t-26 35.5l-60 51q-14 12 -34.5 19.5t-39.5 7.5h-270q-19 0 -39 -7.5t-35 -19.5l-59 -49q-17 -14 -26.5 -35.5t-9.5 -42.5v-147q0 -21 10 -42.5t27 -35.5l59 -47q14 -11 34.5 -18t38.5 -7h270q16 0 33.5 5.5t31.5 14.5v-10q-2 -4 -2 -5q-1 0 -9 -8l-17.5 -17.5l-17.5 -17.5l-9 -9t-2 -1t-2 -1h-330q-22 0 -37.5 -15t-15.5 -37t15.5 -37.5t37.5 -15.5h332q20 0 41 9t35 23l54 54q14 14 23 35t9 41v133q0 2 -0.5 3.5t-0.5 3.5t0.5 4.5t0.5 4.5v138zM525 107q-4 -2 -7 -2h-268q-2 0 -4.5 0.5t-4.5 2.5l-8.5 6.5t-18.5 14.5t-19 15t-10 8v1v142q1 1 10 8.5t19.5 16t19 16l8.5 7.5l4 2t4 1h268q4 0 7 -3q1 0 9.5 -7.5t19 -16.5t19.5 -16.5t10 -8.5l1 -1v-134q-1 0 -1 -1v0q-1 -1 -10 -9t-19.5 -17t-19.5 -17z\" />" << std::endl;
os << "<glyph glyph-name=\"h\" unicode=\"h\" d=\"M673 342l-94 95q-17 15 -37 15h-315q-21 0 -38 -15l-4 -5v127q0 22 -15.5 37t-37.5 15t-37 -15t-15 -37v-506q0 -22 15 -37t37 -15t37.5 15t15.5 37v230q14 14 33 33.5t30 30.5h272l64 -64v-230q0 -22 15 -37t37 -15t37.5 15t15.5 37v252q0 21 -16 37z\" />" << std::endl;
os << "<glyph glyph-name=\"i\" unicode=\"i\" horiz-adv-x=\"246\" d=\"M206 559q0 26 -18.5 44.5t-44.5 18.5t-44.5 -18.5t-18.5 -44.5t18.5 -44.5t44.5 -18.5t44.5 18.5t18.5 44.5zM91 401v-346q0 -22 15 -37t37 -15t37 15t15 37v346q0 23 -15 38t-37 15t-37 -15t-15 -38z\" />" << std::endl;
os << "<glyph glyph-name=\"j\" unicode=\"j\" horiz-adv-x=\"393\" d=\"M353 555q0 26 -18.5 44.5t-44.5 18.5t-44.5 -18.5t-18.5 -44.5t18.5 -44.5t44.5 -18.5t44.5 18.5t18.5 44.5zM238 397v-376q-2 -2 -2 -4l-37 -37q-1 0 -2 -0.5t-2 -0.5h-62q-23 0 -38 -15.5t-15 -37.5t15 -37t38 -15h63q20 0 41 8.5t35 22.5l39 39q14 14 22.5 35t8.5 41v377q0 23 -15 38t-37 15t-37 -15t-15 -38z\" />" << std::endl;
os << "<glyph glyph-name=\"k\" unicode=\"k\" d=\"M636 108h-74q-7 6 -28 25.5t-48 45t-56.5 53t-53.5 50.5l163 62q5 2 12.5 3.5t12.5 1.5h72q22 0 37.5 15t15.5 37q0 23 -15.5 38t-37.5 15h-72q-14 0 -31.5 -3.5t-31.5 -8.5l-316 -122v241q0 22 -15.5 37t-37.5 15t-37 -15t-15 -37v-317v-1v-188q0 -22 15 -37t37 -15t37.5 15t15.5 37v153l82 32l239 -223q14 -14 36 -14h94q22 0 37.5 15t15.5 37t-15.5 37.5t-37.5 15.5z\" />" << std::endl;
os << "<glyph glyph-name=\"l\" unicode=\"l\" horiz-adv-x=\"319\" d=\"M95 595q-15 -15 -15 -37t15 -37l11.5 -12t26 -26.5t26.5 -27t13 -13.5q2 -2 2 -5v-385q0 -22 15 -37t38 -15q22 0 37 15t15 37v387q0 20 -8.5 41t-22.5 35l-79 80q-15 15 -36.5 15.5t-37.5 -15.5z\" />" << std::endl;
os << "<glyph glyph-name=\"m\" unicode=\"m\" horiz-adv-x=\"918\" d=\"M839 392l-27 20q-9 7 -21 12t-28.5 8t-40 4t-56.5 1h-150q-15 0 -27.5 -8t-19.5 -20l-4 3q-9 7 -20.5 12t-28 8t-40 4t-56.5 1h-150q-17 15 -38 15q-22 0 -37 -15t-15 -37v-347q0 -22 15 -37t37 -15t37.5 15t15.5 37v278h135q45 0 62.5 -1.5t22.5 -2.5q2 -2 10 -8.5t11 -8.5v-257q0 -22 15.5 -37t37.5 -15t37 15t15 37v261v9t-2 8h137q46 0 63.5 -1.5t21.5 -2.5q2 -2 10.5 -8.5t11.5 -8.5v-257q0 -22 15 -37t37 -15t37.5 15t15.5 37v261q0 22 -11 43.5t-28 34.5z\" />" << std::endl;
os << "<glyph glyph-name=\"n\" unicode=\"n\" d=\"M649 390l-26 20q-9 7 -21 11.5t-28.5 7.5t-40 4t-56.5 1h-308q-14 16 -37 16q-22 0 -37 -15t-15 -38v-346q0 -22 15 -37t37 -15t37.5 15t15.5 37v278h292q45 0 62.5 -1.5t22.5 -2.5q2 -2 10 -8.5t12 -8.5v-257q0 -22 15 -37t37 -15t37.5 15t15.5 37v261q0 22 -11.5 43.5t-28.5 34.5z\" />" << std::endl;
os << "<glyph glyph-name=\"o\" unicode=\"o\" d=\"M249 454q-18 0 -38.5 -7t-34.5 -19l-59 -46q-17 -14 -27 -35.5t-10 -42.5v-149q0 -21 10 -42.5t26 -35.5l60 -48q14 -12 34 -19t39 -7h270q19 0 39 7t35 19l59 50q17 14 27 35.5t10 42.5v141q0 21 -10 42.5t-26 35.5l-60 51q-14 12 -34.5 19.5t-39.5 7.5h-270zM518 349q4 0 7 -3q1 0 9.5 -7.5t19 -16.5t19.5 -16.5t10 -8.5l1 -1v-137q-1 0 -1 -1v0q-1 -1 -10 -8.5t-19.5 -16t-19 -16l-8.5 -7.5q-4 -2 -8 -2h-268q-5 0 -9 2q0 1 -8.5 8t-18.5 15t-19 15.5t-10 8.5v144q1 1 10 8t19 15t18.5 15l8.5 7q2 1 4.5 2t4.5 1h268z\" />" << std::endl;
os << "<glyph glyph-name=\"p\" unicode=\"p\" d=\"M652 375l-60 48q-14 11 -34 18.5t-39 7.5h-270q-19 0 -39 -7.5t-35 -19.5l-59 -49q-11 -9 -19 -22t-13 -27q-4 -9 -4 -22v-7v-147v-249q0 -22 15 -37.5t37 -15.5t37.5 15.5t15.5 37.5v118q14 -9 31 -14t33 -5h270q18 0 38.5 7t34.5 18l58 45q17 14 28 35.5t11 42.5v151q0 21 -10.5 42.5t-26.5 35.5zM584 149q-2 -1 -11 -8t-18.5 -14.5t-18 -14l-8.5 -6.5q-3 -3 -10 -3h-268q-2 0 -4.5 0.5t-4.5 2.5l-8.5 6.5t-18.5 14.5t-19 15t-10 8v1v142q1 1 10 8.5t19.5 16t19 16l8.5 7.5l4 2t4 1h268q2 0 4.5 -1t4.5 -2l8.5 -7t18.5 -15.5t19 -15.5t10 -8l1 -1v-145z\" />" << std::endl;
os << "<glyph glyph-name=\"q\" unicode=\"q\" d=\"M80 301v-151q0 -21 10.5 -42.5t27.5 -35.5l58 -45q14 -11 34.5 -18t38.5 -7h270q16 0 33.5 5t31.5 14v-117q0 -22 15 -37.5t37 -15.5t37.5 15.5t15.5 37.5v248v147v7q0 10 -5 22q-5 14 -13 27t-19 22l-59 49q-15 12 -35 19.5t-39 7.5h-270q-19 0 -39 -7.5t-34 -18.5l-60 -48q-16 -14 -26 -35.5t-10 -42.5zM185 298v1q1 1 10 8t19 15.5t18.5 15.5l8.5 7q2 1 4.5 2t4.5 1h268q2 0 4 -1l4 -2l8.5 -7.5t19 -16t19.5 -16t10 -8.5h1v-142v-1q-2 -1 -10.5 -8t-19 -15t-19 -14.5l-8.5 -6.5q-2 -2 -4.5 -2.5t-4.5 -0.5h-268q-2 0 -5 0.5t-5 2.5l-8.5 6.5t-18 14t-18.5 14.5t-10 8v145z\" />" << std::endl;
os << "<glyph glyph-name=\"r\" unicode=\"r\" d=\"M649 400l-26 20q-14 11 -34.5 17.5t-38.5 6.5h-371q-10 0 -15 -2q-14 11 -32 11q-22 0 -37 -15t-15 -38v-346q0 -22 15 -37t37 -15t37.5 15t15.5 37v285h365q2 0 5 -1t5 -2q1 -1 10.5 -8.5t13.5 -9.5q2 -20 16.5 -34.5t35.5 -14.5q22 0 37.5 15.5t15.5 37.5t-11.5 43.5t-28.5 34.5z\" />" << std::endl;
os << "<glyph glyph-name=\"s\" unicode=\"s\" horiz-adv-x=\"732\" d=\"M249 453q-20 0 -40 -7.5t-35 -19.5l-57 -50q-16 -14 -25.5 -35t-9.5 -42v-16q0 -26 15.5 -49t40.5 -31l49 -18q13 -5 30.5 -7.5t31.5 -2.5h274q5 0 13.5 -1.5t13.5 -3.5l36 -12q-1 -1 -1 -2q-1 -1 -9.5 -8t-18.5 -15.5t-18.5 -16l-8.5 -7.5q-4 -2 -7 -2h-271q-4 0 -8 2q0 1 -12 11t-26.5 22t-26 21.5l-11.5 9.5q-17 14 -38.5 12t-35.5 -19t-12.5 -38.5t18.5 -35.5l78 -64q14 -12 34.5 -19t39.5 -7h271q20 0 40.5 7.5t34.5 19.5l57 49q16 14 26 35.5t10 42.5v16q0 26 -16 48.5t-41 31.5l-49 17q-13 5 -30.5 8t-31.5 3h-274q-5 0 -13.5 1t-13.5 3l-12 4.5t-23 8.5v1q1 1 9.5 8.5t18.5 16t18.5 15.5l8.5 7l4 2t4 1h270q2 0 4 -1l4 -2l12 -10t26.5 -22t26 -22l11.5 -10q17 -14 38.5 -12t35.5 19t12.5 38.5t-18.5 35.5l-78 64q-14 12 -34.5 19.5t-39.5 7.5h-271z\" />" << std::endl;
os << "<glyph glyph-name=\"t\" unicode=\"t\" horiz-adv-x=\"593\" d=\"M501 -2q22 0 37 15.5t14 37.5q0 22 -15.5 37t-37.5 15h-18t-40 -0.5t-41 -0.5h-19q-5 0 -7 2l-8.5 7.5t-19 16.5t-19.5 16.5t-10 8.5q-1 0 -1 1v147h89q22 0 37.5 15t15.5 37q0 23 -15.5 38t-37.5 15h-89v148q0 22 -15 37.5t-38 15.5q-22 0 -37 -15.5t-15 -37.5v-148h-79q-22 0 -37 -15t-15 -38q0 -22 15 -37t37 -15h79v-149q0 -21 9.5 -42.5t26.5 -35.5l60 -51q14 -12 34.5 -19.5t40.5 -6.5z\" />" << std::endl;
os << "<glyph glyph-name=\"u\" unicode=\"u\" d=\"M584 400v-227q-2 -4 -2 -5q-1 0 -10 -9.5t-20 -20.5t-20.5 -20t-9.5 -10q-2 0 -2.5 -0.5t-2.5 -0.5h-266q-2 0 -2.5 0.5t-2.5 0.5q0 1 -9.5 10t-20.5 20t-20 20.5t-10 9.5q0 2 -0.5 2.5t-0.5 2.5v227q0 23 -15.5 38t-37.5 15t-37 -15t-15 -38v-229q0 -20 8.5 -41t22.5 -35l62 -62q14 -14 35 -22.5t41 -8.5h270q20 0 41 8.5t35 22.5l62 62q14 14 23 35t9 41v229q0 23 -15.5 38t-37.5 15t-37 -15t-15 -38z\" />" << std::endl;
os << "<glyph glyph-name=\"v\" unicode=\"v\" d=\"M584 398v-149q-2 -2 -2 -4l-11 -11t-25 -25.5t-32.5 -33t-32.5 -33t-25.5 -25.5t-10.5 -11q-1 0 -2 -0.5t-2 -0.5h-114q-1 0 -2 0.5t-2 0.5t-11 10.5t-25 26t-32.5 33t-32.5 33t-25 26t-11 10.5q0 1 -0.5 2t-0.5 2v149q0 23 -15.5 38t-37.5 15t-37 -15t-15 -38v-150q0 -20 8.5 -41t22.5 -35l138 -141q14 -14 35.5 -22.5t41.5 -8.5h116q20 0 41.5 8.5t35.5 22.5l138 141q14 14 23 35t9 41v150q0 23 -15.5 38t-37.5 15t-37 -15t-15 -38z\" />" << std::endl;
os << "<glyph glyph-name=\"w\" unicode=\"w\" horiz-adv-x=\"875\" d=\"M782 451q-22 0 -37 -15t-15 -38v-227q-1 -2 -1 -3l-1 -2q-1 0 -10 -9.5t-20 -20.5l-20 -20l-10 -10q-2 0 -2.5 -0.5t-2.5 -0.5h-153v293q0 22 -15.5 37t-37.5 15t-37 -15t-15 -37v-293h-154q-1 0 -2 0.5t-2 0.5l-10 10t-20.5 20t-20.5 20.5t-10 9.5q0 2 -0.5 2.5t-0.5 2.5v227q0 23 -15.5 38t-37.5 15t-37 -15t-15 -38v-229q0 -20 8.5 -41t22.5 -35l62 -62q14 -14 35 -22.5t41 -8.5h416q20 0 41 8.5t35 22.5l62 62q14 14 23 35t9 41v229q0 23 -15.5 38t-37.5 15z\" />" << std::endl;
os << "<glyph glyph-name=\"x\" unicode=\"x\" d=\"M636 108h-55q-1 0 -2 0.5t-2 0.5l-10 10t-26 26.5t-37.5 37.5t-45.5 45l119 119q2 2 4 2h55q22 0 37.5 15t15.5 37q0 23 -15.5 38t-37.5 15h-56q-20 0 -41 -9t-35 -23l-10 -9.5t-26.5 -26t-38 -38.5t-45.5 -46l-46 46l-38.5 38.5l-26 26l-9.5 9.5q-14 14 -35 23t-41 9h-56q-22 0 -37 -15t-15 -38q0 -22 15 -37t37 -15h55q2 0 4 -2l119 -119q-24 -24 -45.5 -45t-37.5 -37.5t-26 -26.5l-10 -10q-1 0 -2 -0.5t-2 -0.5h-55q-22 0 -37 -15.5t-15 -37.5t15 -37t37 -15h56q20 0 41 8.5t35 22.5l120 120l120 -120q14 -14 35 -22.5t41 -8.5h56q22 0 37.5 15t15.5 37t-15.5 37.5t-37.5 15.5z\" />" << std::endl;
os << "<glyph glyph-name=\"y\" unicode=\"y\" d=\"M636 451q-22 0 -37 -15t-15 -38v-25q0 -22 -0.5 -58.5t-0.5 -71.5v-60v-27v-1q-2 -1 -10.5 -8.5t-19 -16t-19 -16l-8.5 -7.5q-4 -2 -8 -2h-268q-4 0 -8 2l-8.5 7.5t-19 16t-19.5 16t-10 8.5v1v242q0 23 -15.5 38t-37.5 15t-37 -15t-15 -38v-244q0 -21 9.5 -42.5t26.5 -35.5l59 -50q15 -12 35 -19t39 -7h83v-101q0 -22 15 -37.5t37 -15.5t37 15.5t15 37.5v101h83q19 0 39.5 7t34.5 19l59 50q17 14 26.5 35t9.5 43l1 244q0 22 -15.5 37.5t-37.5 15.5z\" />" << std::endl;
os << "<glyph glyph-name=\"z\" unicode=\"z\" d=\"M132 453q-22 0 -37 -15t-15 -38q0 -22 15 -37t37 -15h385q1 0 5 -2l5 -5.5t10 -10.5q-23 -7 -61.5 -19t-82.5 -26t-89.5 -28.5t-82.5 -26t-60 -18.5l-23 -7q-37 -11 -46 -47q-2 -10 -2 -15q0 -25 21 -48l62 -62q14 -14 35 -22.5t41 -8.5h387q22 0 37.5 15t15.5 37t-15.5 37.5t-37.5 15.5h-385q-2 0 -2.5 0.5t-2.5 0.5q0 1 -5 6l-10 10q23 7 61.5 19t82.5 26t89.5 28.5t82.5 26t60 18.5l23 7q17 5 29 17.5t17 29.5q2 10 2 15q0 27 -21 48l-62 62q-14 14 -35 23t-41 9h-387z\" />" << std::endl;
os << "<glyph glyph-name=\"asciitilde\" unicode=\"~\" horiz-adv-x=\"747\" d=\"M602 568l-10.5 -11t-23.5 -24.5t-23.5 -25t-11.5 -12.5q-2 0 -4 -2h-62q-2 0 -3.5 1t-2.5 1q-1 1 -11 10t-21.5 19.5t-21 19.5l-9.5 9q-17 16 -40.5 25t-46.5 9h-52q-23 -1 -46.5 -10.5t-40.5 -25.5l-74 -74q-17 -17 -17.5 -42t17.5 -43q17 -18 42 -18q26 0 43 17l11 11l24.5 24.5l25 25l12.5 12.5q2 0 6 2h49q4 0 6 -2q1 -1 11 -10t21.5 -19.5t21 -19.5l9.5 -9q17 -16 40.5 -25t46.5 -9l65 1q23 0 47 10.5t40 27.5l71 74q17 19 16.5 44t-19.5 42q-18 17 -43 16.5t-42 -19.5z\" />" << std::endl;
os << "<glyph glyph-name=\"nonbreakingspace\" unicode=\"&#xa0;\" horiz-adv-x=\"120\" />" << std::endl;
os << "<glyph glyph-name=\".null\" horiz-adv-x=\"120\" />" << std::endl;
os << "<hkern g1=\".notdef\" u2=\"j\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x21;\" u2=\"j\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"&#x21;\" u2=\"&#x3e;\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x7e;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"z\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"y\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"x\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"w\" k=\"100\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"v\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"u\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"t\" k=\"148\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"s\" k=\"164\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"r\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"q\" k=\"155\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"p\" k=\"161\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"o\" k=\"169\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"n\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"m\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"l\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"k\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"j\" k=\"240\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"i\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"h\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"g\" k=\"163\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"f\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"e\" k=\"171\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"d\" k=\"179\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"c\" k=\"190\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"b\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"a\" k=\"150\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"`\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"^\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"]\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"\\\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"[\" k=\"135\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"Z\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"Y\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"X\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"W\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"V\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"U\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"T\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"S\" k=\"110\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"R\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"Q\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"P\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"O\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"N\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"M\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"L\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"K\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"J\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"I\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"H\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"G\" k=\"93\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"F\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"E\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"D\" k=\"97\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"C\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"B\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"A\" k=\"162\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x40;\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x3f;\" k=\"88\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x3e;\" k=\"169\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x3d;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x3c;\" k=\"198\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x3b;\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x3a;\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x39;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x38;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x37;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x36;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x35;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x34;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x33;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x32;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x31;\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x30;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x2f;\" k=\"445\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x2d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x2b;\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x2a;\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x29;\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x28;\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x27;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x26;\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x25;\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x24;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x23;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x22;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" u2=\"&#x21;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x22;\" g2=\".notdef\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x7e;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"z\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"y\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"x\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"w\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"v\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"u\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"t\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"s\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"r\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"q\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"p\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"o\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"n\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"m\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"l\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"k\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"j\" k=\"188\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"i\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"h\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"g\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"f\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"e\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"d\" k=\"176\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"c\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"b\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"a\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"`\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"_\" k=\"167\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"^\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"]\" k=\"192\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"\\\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"[\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"Z\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"Y\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"X\" k=\"222\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"W\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"V\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"U\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"T\" k=\"222\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"S\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"R\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"Q\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"P\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"O\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"N\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"M\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"L\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"K\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"J\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"I\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"H\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"G\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"F\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"E\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"D\" k=\"121\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"C\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"B\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"A\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x40;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x3f;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x3e;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x3d;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x3c;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x3b;\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x3a;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x39;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x38;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x37;\" k=\"222\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x36;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x35;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x34;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x33;\" k=\"192\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x32;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x31;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x30;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x2f;\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x2e;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x2d;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x2c;\" k=\"148\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x2b;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x2a;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x29;\" k=\"170\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x28;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x27;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x26;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x25;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x24;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x23;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x22;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" u2=\"&#x21;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x23;\" g2=\".notdef\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"&#x7e;\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"j\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"`\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"X\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"T\" k=\"190\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"D\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"&#x3e;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"&#x2f;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x24;\" u2=\"&#x2c;\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x7e;\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"x\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"t\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"l\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"j\" k=\"169\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"f\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"a\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"`\" k=\"151\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"_\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"^\" k=\"155\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"\\\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"T\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"A\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x3f;\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x3e;\" k=\"96\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x3c;\" k=\"172\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x37;\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x2f;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x2e;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x2d;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x2c;\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x2b;\" k=\"202\" />" << std::endl;
os << "<hkern u1=\"&#x25;\" u2=\"&#x25;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x7e;\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"t\" k=\"93\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"l\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"j\" k=\"170\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"f\" k=\"116\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"`\" k=\"169\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"^\" k=\"173\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"\\\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"Y\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"V\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"T\" k=\"219\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x3f;\" k=\"110\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x3e;\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x3c;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x37;\" k=\"205\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x34;\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x2b;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x29;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x27;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" u2=\"&#x22;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"&#x26;\" g2=\".notdef\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"t\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"s\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"q\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"p\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"o\" k=\"84\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"j\" k=\"156\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"g\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"e\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"d\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"c\" k=\"106\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"a\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"`\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"A\" k=\"114\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x3e;\" k=\"84\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x3c;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x3b;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x31;\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x2f;\" k=\"360\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x2b;\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x29;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" u2=\"&#x25;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"&#x27;\" g2=\".notdef\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x7e;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"v\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"t\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"j\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"f\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"`\" k=\"121\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"^\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x3e;\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x3d;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x3c;\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x3a;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x2d;\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x2b;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x2a;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x28;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x28;\" u2=\"&#x23;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x29;\" u2=\"j\" k=\"144\" />" << std::endl;
os << "<hkern u1=\"&#x29;\" u2=\"`\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x29;\" u2=\"]\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x29;\" u2=\"&#x3e;\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"&#x29;\" u2=\"&#x2f;\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x29;\" u2=\"&#x2c;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x29;\" u2=\"&#x29;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"t\" k=\"88\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"j\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"f\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"_\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"]\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"D\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x3c;\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x37;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x33;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x2f;\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x2e;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x2d;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x2c;\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x2b;\" k=\"123\" />" << std::endl;
os << "<hkern u1=\"&#x2a;\" u2=\"&#x29;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x7e;\" k=\"184\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"z\" k=\"153\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"y\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"x\" k=\"168\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"v\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"t\" k=\"215\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"s\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"r\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"q\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"p\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"o\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"n\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"l\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"k\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"j\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"i\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"h\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"g\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"f\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"e\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"d\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"c\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"b\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"a\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"`\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"_\" k=\"234\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"]\" k=\"141\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"\\\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"[\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"Z\" k=\"168\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"Y\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"X\" k=\"276\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"V\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"T\" k=\"338\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"S\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"Q\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"P\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"O\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"J\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"I\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"G\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"F\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"E\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"D\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"C\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"A\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x3f;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x3e;\" k=\"218\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x3d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x3c;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x3b;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x3a;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x37;\" k=\"264\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x33;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x31;\" k=\"156\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x2f;\" k=\"249\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x2e;\" k=\"234\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x2d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x2c;\" k=\"265\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x2b;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x2a;\" k=\"168\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x29;\" k=\"119\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x28;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x27;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x25;\" k=\"233\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x24;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x23;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x22;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2b;\" u2=\"&#x21;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"y\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"w\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"v\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"t\" k=\"101\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"l\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"j\" k=\"131\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"f\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"\\\" k=\"403\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"Y\" k=\"311\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"V\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"T\" k=\"311\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x3f;\" k=\"280\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x3d;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x3c;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x3a;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x37;\" k=\"212\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x34;\" k=\"135\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x2b;\" k=\"259\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x2a;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x28;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x25;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x2c;\" u2=\"&#x23;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"z\" k=\"156\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"x\" k=\"127\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"l\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"j\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"`\" k=\"161\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"]\" k=\"151\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"\\\" k=\"200\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"Z\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"Y\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"X\" k=\"196\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"V\" k=\"74\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"T\" k=\"273\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"S\" k=\"93\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"D\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x3f;\" k=\"271\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x3e;\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x3b;\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x39;\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x37;\" k=\"274\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x33;\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x31;\" k=\"166\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x2f;\" k=\"166\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x29;\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x25;\" k=\"150\" />" << std::endl;
os << "<hkern u1=\"&#x2d;\" u2=\"&#x24;\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"t\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"j\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"f\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"\\\" k=\"340\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"Y\" k=\"245\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"V\" k=\"106\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"T\" k=\"244\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"&#x3f;\" k=\"211\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"&#x3c;\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"&#x37;\" k=\"199\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"&#x34;\" k=\"168\" />" << std::endl;
os << "<hkern u1=\"&#x2e;\" u2=\"&#x2b;\" k=\"245\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x7e;\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"z\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"y\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"x\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"w\" k=\"100\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"v\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"u\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"t\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"s\" k=\"179\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"r\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"q\" k=\"170\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"p\" k=\"176\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"o\" k=\"184\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"n\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"m\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"j\" k=\"132\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"g\" k=\"178\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"f\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"e\" k=\"186\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"d\" k=\"194\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"c\" k=\"205\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"a\" k=\"165\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"`\" k=\"101\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"_\" k=\"420\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"^\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"A\" k=\"121\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x3e;\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x3c;\" k=\"213\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x3b;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x2f;\" k=\"460\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x2e;\" k=\"376\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x2d;\" k=\"247\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x2c;\" k=\"462\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x2b;\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"&#x2f;\" u2=\"&#x29;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x30;\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"&#x30;\" u2=\"&#x2c;\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"&#x30;\" u2=\"&#x29;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x7e;\" k=\"135\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"y\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"v\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"t\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"l\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"j\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"f\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"`\" k=\"230\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"^\" k=\"172\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"\\\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"Y\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"V\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"T\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x3f;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x3e;\" k=\"97\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x3d;\" k=\"173\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x3c;\" k=\"219\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x3a;\" k=\"135\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x37;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x34;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x2d;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x2b;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x2a;\" k=\"186\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x28;\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x27;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x25;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x23;\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"&#x31;\" u2=\"&#x22;\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"&#x32;\" u2=\"j\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x32;\" u2=\"&#x3e;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x32;\" u2=\"&#x3c;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"&#x32;\" u2=\"&#x2d;\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x33;\" u2=\"j\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x33;\" u2=\"&#x3e;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x33;\" u2=\"&#x2c;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x33;\" u2=\"&#x29;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x7e;\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"x\" k=\"149\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"l\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"a\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"`\" k=\"183\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"_\" k=\"132\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"^\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"]\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"\\\" k=\"145\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"Z\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"Y\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"X\" k=\"156\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"V\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"T\" k=\"156\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"S\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"D\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"A\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x3f;\" k=\"154\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x3e;\" k=\"127\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x3c;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x3b;\" k=\"83\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x37;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x33;\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x31;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x2f;\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x2e;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x2d;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x2c;\" k=\"144\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x2b;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x29;\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x27;\" k=\"158\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x25;\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x24;\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"&#x34;\" u2=\"&#x22;\" k=\"158\" />" << std::endl;
os << "<hkern u1=\"&#x35;\" u2=\"&#x7e;\" k=\"100\" />" << std::endl;
os << "<hkern u1=\"&#x35;\" u2=\"t\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x35;\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"&#x35;\" u2=\"f\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x35;\" u2=\"`\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"&#x35;\" u2=\"&#x3e;\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"&#x35;\" u2=\"&#x2c;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x36;\" u2=\"t\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x36;\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"&#x36;\" u2=\"f\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x36;\" u2=\"`\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x36;\" u2=\"&#x2c;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x36;\" u2=\"&#x29;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"s\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"q\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"p\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"o\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"j\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"g\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"e\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"d\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"c\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"a\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"_\" k=\"286\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"A\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x3c;\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x2f;\" k=\"327\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x2e;\" k=\"304\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x2d;\" k=\"160\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x2c;\" k=\"313\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x2b;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x29;\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"&#x37;\" u2=\"&#x25;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x38;\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"&#x38;\" u2=\"&#x2c;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x38;\" u2=\"&#x29;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x39;\" u2=\"j\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x39;\" u2=\"&#x3e;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x39;\" u2=\"&#x2c;\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"&#x39;\" u2=\"&#x29;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"t\" k=\"161\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"l\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"j\" k=\"161\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"d\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"_\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"]\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"X\" k=\"115\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"T\" k=\"284\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"D\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x3f;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x3e;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x3c;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x37;\" k=\"299\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x33;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x2e;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x2d;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x2c;\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x2b;\" k=\"100\" />" << std::endl;
os << "<hkern u1=\"&#x3a;\" u2=\"&#x29;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"t\" k=\"161\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"l\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"j\" k=\"149\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"_\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"]\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"T\" k=\"284\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x3f;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x3e;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x3c;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x37;\" k=\"216\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x33;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x2f;\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x2e;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x2d;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x2c;\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x2b;\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"&#x3b;\" u2=\"&#x29;\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x7e;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"z\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"y\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"x\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"w\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"v\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"t\" k=\"180\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"s\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"r\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"p\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"n\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"m\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"l\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"j\" k=\"211\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"i\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"h\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"g\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"f\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"d\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"c\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"`\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"_\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"^\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"]\" k=\"108\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"\\\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"[\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"Z\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"Y\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"X\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"W\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"V\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"U\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"T\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"S\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"R\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"Q\" k=\"74\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"P\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"O\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"N\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"M\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"L\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"K\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"J\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"I\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"H\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"G\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"F\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"E\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"D\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"C\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"B\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"A\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x3f;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x3e;\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x3d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x3c;\" k=\"230\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x39;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x38;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x37;\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x36;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x35;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x34;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x33;\" k=\"93\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x32;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x31;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x30;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x2f;\" k=\"108\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x2e;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x2d;\" k=\"156\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x2c;\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x2b;\" k=\"169\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x2a;\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x29;\" k=\"132\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x28;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x27;\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x26;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x25;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x24;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x23;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x22;\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" u2=\"&#x21;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3c;\" g2=\".notdef\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x7e;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"y\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"x\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"w\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"v\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"t\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"s\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"r\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"q\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"p\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"o\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"n\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"m\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"k\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"j\" k=\"226\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"i\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"h\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"g\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"f\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"e\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"d\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"c\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"b\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"a\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"`\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"_\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"^\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"]\" k=\"231\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"\\\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"[\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"Z\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"Y\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"X\" k=\"168\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"W\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"V\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"U\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"T\" k=\"338\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"S\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"R\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"Q\" k=\"74\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"P\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"O\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"N\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"M\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"L\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"K\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"J\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"I\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"H\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"G\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"F\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"E\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"D\" k=\"153\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"C\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"B\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"A\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x3f;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x3e;\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x3d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x3c;\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x3b;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x3a;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x39;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x38;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x37;\" k=\"353\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x36;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x35;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x34;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x33;\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x32;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x31;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x30;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x2f;\" k=\"123\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x2e;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x2d;\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x2c;\" k=\"125\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x2b;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x2a;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x29;\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x28;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x27;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x26;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x25;\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x24;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x23;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x22;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" u2=\"&#x21;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3d;\" g2=\".notdef\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x7e;\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"z\" k=\"153\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"y\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"x\" k=\"168\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"v\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"t\" k=\"215\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"s\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"r\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"q\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"p\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"o\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"n\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"l\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"k\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"j\" k=\"234\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"i\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"h\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"g\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"f\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"e\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"d\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"c\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"b\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"a\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"`\" k=\"184\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"_\" k=\"234\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"]\" k=\"218\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"\\\" k=\"184\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"[\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"Z\" k=\"168\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"Y\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"X\" k=\"276\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"V\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"T\" k=\"292\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"S\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"Q\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"P\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"O\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"J\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"I\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"G\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"F\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"E\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"D\" k=\"153\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"C\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"A\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x3f;\" k=\"88\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x3e;\" k=\"218\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x3d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x3c;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x3b;\" k=\"124\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x3a;\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x39;\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x38;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x37;\" k=\"292\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x33;\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x32;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x31;\" k=\"215\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x2f;\" k=\"260\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x2e;\" k=\"234\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x2d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x2c;\" k=\"265\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x2b;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x2a;\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x29;\" k=\"218\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x28;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x27;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x26;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x25;\" k=\"244\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x24;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x23;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x22;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" u2=\"&#x21;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"&#x3e;\" g2=\".notdef\" k=\"75\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"j\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"_\" k=\"201\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"A\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x3e;\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x3c;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x2f;\" k=\"241\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x2e;\" k=\"234\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x2d;\" k=\"74\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x2c;\" k=\"228\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x2b;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x29;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"&#x3f;\" u2=\"&#x25;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"&#x40;\" u2=\"j\" k=\"145\" />" << std::endl;
os << "<hkern u1=\"&#x40;\" u2=\"`\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"&#x40;\" u2=\"&#x3e;\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"&#x40;\" u2=\"&#x29;\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x7e;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"y\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"t\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"l\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"j\" k=\"145\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"`\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"^\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"\\\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"Y\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"V\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"T\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x3f;\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x3e;\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x3c;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x37;\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x2c;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x27;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"A\" u2=\"&#x22;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"B\" u2=\"j\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"B\" u2=\"&#x3e;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"B\" u2=\"&#x2c;\" k=\"135\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"t\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"j\" k=\"128\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"f\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"`\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"^\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"&#x3e;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"&#x3c;\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"&#x2d;\" k=\"566\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"&#x2c;\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"C\" u2=\"&#x2b;\" k=\"271\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"l\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"j\" k=\"148\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"_\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"]\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"\\\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"T\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x3f;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x3e;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x37;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x2e;\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x2c;\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x29;\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x27;\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"D\" u2=\"&#x22;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x7e;\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"v\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"t\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"j\" k=\"141\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"`\" k=\"155\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"^\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x3e;\" k=\"84\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x3d;\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x3c;\" k=\"114\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x3a;\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x2d;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x2b;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x28;\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"E\" u2=\"&#x23;\" k=\"114\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x7e;\" k=\"144\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"z\" k=\"268\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"y\" k=\"276\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"x\" k=\"270\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"w\" k=\"272\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"v\" k=\"271\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"u\" k=\"264\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"t\" k=\"151\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"s\" k=\"274\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"r\" k=\"276\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"q\" k=\"250\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"p\" k=\"271\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"o\" k=\"264\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"n\" k=\"274\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"m\" k=\"270\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"j\" k=\"163\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"g\" k=\"273\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"f\" k=\"220\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"e\" k=\"266\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"d\" k=\"274\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"c\" k=\"285\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"a\" k=\"291\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"`\" k=\"381\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"_\" k=\"570\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"^\" k=\"155\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"A\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x3e;\" k=\"141\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x3d;\" k=\"278\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x3c;\" k=\"232\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x3b;\" k=\"358\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x3a;\" k=\"278\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x31;\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x2f;\" k=\"478\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x2e;\" k=\"570\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x2d;\" k=\"278\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x2c;\" k=\"616\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x2b;\" k=\"278\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x2a;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x28;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x26;\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x25;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x24;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"F\" u2=\"&#x23;\" k=\"155\" />" << std::endl;
os << "<hkern u1=\"G\" u2=\"j\" k=\"135\" />" << std::endl;
os << "<hkern u1=\"G\" u2=\"&#x2c;\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"H\" u2=\"j\" k=\"130\" />" << std::endl;
os << "<hkern u1=\"H\" u2=\"&#x3e;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"I\" u2=\"j\" k=\"123\" />" << std::endl;
os << "<hkern u1=\"I\" u2=\"&#x3e;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x7e;\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"z\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"y\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"x\" k=\"88\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"w\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"v\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"u\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"t\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"s\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"r\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"q\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"p\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"o\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"n\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"m\" k=\"88\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"j\" k=\"190\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"g\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"f\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"e\" k=\"84\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"d\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"c\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"a\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"`\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"_\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"^\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"A\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x3e;\" k=\"127\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x3d;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x3c;\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x3b;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x3a;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x31;\" k=\"96\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x2f;\" k=\"144\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x2e;\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x2d;\" k=\"114\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x2c;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x2b;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x28;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x26;\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x25;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x24;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"J\" u2=\"&#x23;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x7e;\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"v\" k=\"106\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"t\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"l\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"j\" k=\"170\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"f\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"`\" k=\"308\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"^\" k=\"159\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"\\\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"Y\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"T\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x3f;\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x3e;\" k=\"114\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x3d;\" k=\"128\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x3c;\" k=\"220\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x3a;\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x37;\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x34;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x2d;\" k=\"220\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x2c;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x2b;\" k=\"220\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x28;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x27;\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x23;\" k=\"144\" />" << std::endl;
os << "<hkern u1=\"K\" u2=\"&#x22;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x7e;\" k=\"564\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"y\" k=\"164\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"v\" k=\"96\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"t\" k=\"159\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"l\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"j\" k=\"145\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"f\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"`\" k=\"564\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"^\" k=\"549\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"\\\" k=\"487\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"Y\" k=\"272\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"V\" k=\"149\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"T\" k=\"272\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x3f;\" k=\"241\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x3e;\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x3d;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x3c;\" k=\"236\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x3a;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x37;\" k=\"210\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x34;\" k=\"313\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x2d;\" k=\"564\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x2b;\" k=\"272\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x2a;\" k=\"269\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x28;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x27;\" k=\"564\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x25;\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x23;\" k=\"149\" />" << std::endl;
os << "<hkern u1=\"L\" u2=\"&#x22;\" k=\"564\" />" << std::endl;
os << "<hkern u1=\"M\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"N\" u2=\"j\" k=\"130\" />" << std::endl;
os << "<hkern u1=\"N\" u2=\"&#x3e;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"O\" u2=\"j\" k=\"132\" />" << std::endl;
os << "<hkern u1=\"O\" u2=\"&#x3e;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"O\" u2=\"&#x2c;\" k=\"120\" />" << std::endl;
os << "<hkern u1=\"O\" u2=\"&#x29;\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"j\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"_\" k=\"565\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"]\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"A\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"&#x3e;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"&#x2f;\" k=\"243\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"&#x2e;\" k=\"220\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"&#x2c;\" k=\"291\" />" << std::endl;
os << "<hkern u1=\"P\" u2=\"&#x29;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"Q\" u2=\"j\" k=\"131\" />" << std::endl;
os << "<hkern u1=\"Q\" u2=\"&#x3e;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"R\" u2=\"j\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"R\" u2=\"&#x3e;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"S\" u2=\"j\" k=\"125\" />" << std::endl;
os << "<hkern u1=\"S\" u2=\"&#x3e;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"S\" u2=\"&#x3c;\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"S\" u2=\"&#x2c;\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"S\" u2=\"&#x29;\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x7e;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"z\" k=\"236\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"y\" k=\"259\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"x\" k=\"253\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"w\" k=\"255\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"v\" k=\"254\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"u\" k=\"247\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"t\" k=\"134\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"s\" k=\"257\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"r\" k=\"259\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"q\" k=\"233\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"p\" k=\"254\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"o\" k=\"247\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"n\" k=\"257\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"m\" k=\"253\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"j\" k=\"184\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"g\" k=\"256\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"f\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"e\" k=\"249\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"d\" k=\"257\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"c\" k=\"268\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"a\" k=\"229\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"`\" k=\"317\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"_\" k=\"275\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"^\" k=\"167\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"A\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x3e;\" k=\"154\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x3d;\" k=\"275\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x3c;\" k=\"244\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x3b;\" k=\"275\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x3a;\" k=\"275\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x31;\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x2f;\" k=\"275\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x2e;\" k=\"275\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x2d;\" k=\"291\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x2c;\" k=\"248\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x2b;\" k=\"275\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x28;\" k=\"121\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x26;\" k=\"75\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x25;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x24;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"T\" u2=\"&#x23;\" k=\"153\" />" << std::endl;
os << "<hkern u1=\"U\" u2=\"j\" k=\"119\" />" << std::endl;
os << "<hkern u1=\"U\" u2=\"&#x3e;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"U\" u2=\"&#x2c;\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"t\" k=\"74\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"s\" k=\"74\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"p\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"o\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"j\" k=\"164\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"g\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"e\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"d\" k=\"74\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"c\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"a\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"`\" k=\"88\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"_\" k=\"140\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"^\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"A\" k=\"140\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x3e;\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x3c;\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x3b;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x31;\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x2f;\" k=\"140\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x2e;\" k=\"140\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x2d;\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x2c;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x2b;\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"V\" u2=\"&#x25;\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"W\" u2=\"j\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"W\" u2=\"&#x2c;\" k=\"137\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x7e;\" k=\"131\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"v\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"t\" k=\"154\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"f\" k=\"100\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"`\" k=\"229\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"^\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x3e;\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x3d;\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x3c;\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x3a;\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x2d;\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x2b;\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x28;\" k=\"110\" />" << std::endl;
os << "<hkern u1=\"X\" u2=\"&#x23;\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"j\" k=\"159\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"d\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"c\" k=\"66\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"_\" k=\"274\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"A\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"&#x3e;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"&#x3c;\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"&#x2f;\" k=\"274\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"&#x2e;\" k=\"274\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"&#x2d;\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"&#x2c;\" k=\"336\" />" << std::endl;
os << "<hkern u1=\"Y\" u2=\"&#x2b;\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"v\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"t\" k=\"127\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"j\" k=\"143\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"f\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"`\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"^\" k=\"84\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"&#x3e;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"&#x3c;\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"&#x2d;\" k=\"272\" />" << std::endl;
os << "<hkern u1=\"Z\" u2=\"&#x2b;\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"&#x7e;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"v\" k=\"91\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"t\" k=\"124\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"j\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"f\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"`\" k=\"169\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"^\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"&#x3e;\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"&#x3d;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"&#x3c;\" k=\"159\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"&#x2d;\" k=\"131\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"&#x2b;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"[\" u2=\"&#x23;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x7e;\" k=\"303\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"v\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"t\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"l\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"j\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"f\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"`\" k=\"355\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"^\" k=\"221\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"\\\" k=\"395\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"Y\" k=\"253\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"V\" k=\"130\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"T\" k=\"252\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x3f;\" k=\"220\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x3e;\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x3c;\" k=\"222\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x37;\" k=\"191\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x34;\" k=\"207\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x2d;\" k=\"148\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x2b;\" k=\"207\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x2a;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x27;\" k=\"315\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x23;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"\\\" u2=\"&#x22;\" k=\"315\" />" << std::endl;
os << "<hkern u1=\"]\" u2=\"j\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"]\" u2=\"`\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"]\" u2=\"&#x3e;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"]\" u2=\"&#x29;\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x7e;\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"z\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"t\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"l\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"j\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"d\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"`\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"]\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"\\\" k=\"108\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"Z\" k=\"166\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"X\" k=\"166\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"V\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"T\" k=\"150\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"J\" k=\"166\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"D\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"A\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x3e;\" k=\"152\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x37;\" k=\"167\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x33;\" k=\"75\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x32;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x31;\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x2f;\" k=\"242\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x2d;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x2a;\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x29;\" k=\"130\" />" << std::endl;
os << "<hkern u1=\"^\" u2=\"&#x25;\" k=\"151\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"y\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"t\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"l\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"f\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"\\\" k=\"357\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"Y\" k=\"265\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"V\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"T\" k=\"265\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x3f;\" k=\"234\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x3e;\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x3d;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x3c;\" k=\"195\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x3a;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x37;\" k=\"164\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x34;\" k=\"311\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x2b;\" k=\"226\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x2a;\" k=\"223\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x28;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x25;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"_\" u2=\"&#x23;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"l\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"j\" k=\"153\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"`\" k=\"106\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"]\" k=\"158\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"\\\" k=\"145\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"Z\" k=\"193\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"X\" k=\"248\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"V\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"T\" k=\"279\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"D\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x3e;\" k=\"189\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x3b;\" k=\"83\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x3a;\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x37;\" k=\"296\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x33;\" k=\"81\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x31;\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x2f;\" k=\"235\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x2a;\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x29;\" k=\"136\" />" << std::endl;
os << "<hkern u1=\"`\" u2=\"&#x25;\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"a\" u2=\"j\" k=\"101\" />" << std::endl;
os << "<hkern u1=\"a\" u2=\"`\" k=\"100\" />" << std::endl;
os << "<hkern u1=\"a\" u2=\"\\\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"a\" u2=\"T\" k=\"243\" />" << std::endl;
os << "<hkern u1=\"a\" u2=\"&#x37;\" k=\"182\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"`\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"\\\" k=\"134\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"T\" k=\"237\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"&#x37;\" k=\"207\" />" << std::endl;
os << "<hkern u1=\"b\" u2=\"&#x2c;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"`\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"\\\" k=\"130\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"T\" k=\"233\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"&#x3c;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"&#x37;\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"&#x2d;\" k=\"441\" />" << std::endl;
os << "<hkern u1=\"c\" u2=\"&#x2b;\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"d\" u2=\"j\" k=\"101\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"`\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"^\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"\\\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"T\" k=\"236\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x3e;\" k=\"69\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x3d;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x3c;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x3a;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x37;\" k=\"206\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x2d;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x2b;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x28;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"e\" u2=\"&#x23;\" k=\"86\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"z\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"y\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"x\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"w\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"v\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"u\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"t\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"s\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"r\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"q\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"p\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"o\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"n\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"m\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"j\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"i\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"g\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"f\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"e\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"d\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"c\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"a\" k=\"58\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"`\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"_\" k=\"158\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"^\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"A\" k=\"75\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x3d;\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x3c;\" k=\"90\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x3b;\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x3a;\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x31;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x2f;\" k=\"256\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x2e;\" k=\"191\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x2d;\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x2c;\" k=\"185\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x2b;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x2a;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x29;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x28;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"f\" u2=\"&#x23;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"g\" u2=\"`\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"g\" u2=\"\\\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"g\" u2=\"T\" k=\"245\" />" << std::endl;
os << "<hkern u1=\"g\" u2=\"&#x37;\" k=\"184\" />" << std::endl;
os << "<hkern u1=\"h\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"h\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"h\" u2=\"`\" k=\"101\" />" << std::endl;
os << "<hkern u1=\"h\" u2=\"\\\" k=\"141\" />" << std::endl;
os << "<hkern u1=\"h\" u2=\"T\" k=\"244\" />" << std::endl;
os << "<hkern u1=\"h\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"h\" u2=\"&#x37;\" k=\"183\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"t\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"l\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"f\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"\\\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"Y\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"T\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x3f;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x3d;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x3c;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x3a;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x37;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x34;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x2d;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x2b;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x2a;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x28;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x27;\" k=\"172\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x25;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x23;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"i\" u2=\"&#x22;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"t\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"f\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"c\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"a\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"`\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"_\" k=\"175\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"^\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"]\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"[\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"Z\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"T\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"S\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"Q\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"J\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"G\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"E\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"D\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"C\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"A\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x3e;\" k=\"127\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x3d;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x3c;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x3b;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x3a;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x37;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x2f;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x2e;\" k=\"57\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x2d;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x2c;\" k=\"267\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x2b;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x2a;\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x29;\" k=\"104\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x28;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x25;\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x24;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" u2=\"&#x23;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"j\" g2=\".notdef\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x7e;\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"y\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"t\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"l\" k=\"63\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"j\" k=\"138\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"`\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"^\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"\\\" k=\"84\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"Y\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"V\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"T\" k=\"264\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x3f;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x3e;\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x3c;\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x37;\" k=\"203\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x34;\" k=\"52\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x2d;\" k=\"206\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x2b;\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x2a;\" k=\"123\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x27;\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"k\" u2=\"&#x22;\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x7e;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"y\" k=\"159\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"t\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"l\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"j\" k=\"164\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"f\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"`\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"^\" k=\"421\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"\\\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"Y\" k=\"267\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"V\" k=\"144\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"T\" k=\"121\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x3f;\" k=\"236\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x3e;\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x3d;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x3c;\" k=\"236\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x3a;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x37;\" k=\"122\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x34;\" k=\"375\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x2d;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x2b;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x2a;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x29;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x28;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x27;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x25;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x23;\" k=\"113\" />" << std::endl;
os << "<hkern u1=\"l\" u2=\"&#x22;\" k=\"436\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"l\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"j\" k=\"108\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"`\" k=\"88\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"\\\" k=\"127\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"T\" k=\"261\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"&#x3f;\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"&#x37;\" k=\"201\" />" << std::endl;
os << "<hkern u1=\"m\" u2=\"&#x2c;\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"n\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"n\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"n\" u2=\"`\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"n\" u2=\"\\\" k=\"110\" />" << std::endl;
os << "<hkern u1=\"n\" u2=\"T\" k=\"244\" />" << std::endl;
os << "<hkern u1=\"n\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"n\" u2=\"&#x37;\" k=\"183\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"`\" k=\"96\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"\\\" k=\"136\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"T\" k=\"239\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"&#x37;\" k=\"209\" />" << std::endl;
os << "<hkern u1=\"o\" u2=\"&#x2c;\" k=\"72\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"j\" k=\"119\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"`\" k=\"87\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"_\" k=\"439\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"\\\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"T\" k=\"245\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"A\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"&#x37;\" k=\"200\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"&#x2f;\" k=\"209\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"&#x2e;\" k=\"439\" />" << std::endl;
os << "<hkern u1=\"p\" u2=\"&#x2c;\" k=\"439\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"l\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"`\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"\\\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"V\" k=\"53\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"T\" k=\"252\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"&#x37;\" k=\"191\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"&#x27;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"q\" u2=\"&#x22;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"z\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"t\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"l\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"j\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"i\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"f\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"`\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"_\" k=\"429\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"]\" k=\"116\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"\\\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"Z\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"Y\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"X\" k=\"207\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"T\" k=\"238\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"J\" k=\"188\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x3f;\" k=\"157\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x3e;\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x3d;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x3b;\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x3a;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x37;\" k=\"254\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x31;\" k=\"131\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x2f;\" k=\"178\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x2e;\" k=\"462\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x2c;\" k=\"441\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x2a;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x29;\" k=\"94\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x27;\" k=\"126\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x25;\" k=\"162\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x23;\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"r\" u2=\"&#x22;\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"`\" k=\"89\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"\\\" k=\"128\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"T\" k=\"232\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"&#x37;\" k=\"202\" />" << std::endl;
os << "<hkern u1=\"s\" u2=\"&#x2c;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x7e;\" k=\"134\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"w\" k=\"62\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"v\" k=\"92\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"u\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"t\" k=\"110\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"j\" k=\"156\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"f\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"d\" k=\"64\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"c\" k=\"75\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"b\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"a\" k=\"153\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"`\" k=\"201\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"_\" k=\"199\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"^\" k=\"97\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"\\\" k=\"225\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"Y\" k=\"129\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"V\" k=\"175\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"U\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"T\" k=\"267\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"S\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"Q\" k=\"65\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"O\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"J\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"G\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"E\" k=\"61\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"C\" k=\"71\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"A\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x3f;\" k=\"142\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x3e;\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x3d;\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x3c;\" k=\"144\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x3b;\" k=\"199\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x3a;\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x39;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x38;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x37;\" k=\"221\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x36;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x34;\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x32;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x30;\" k=\"68\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x2f;\" k=\"199\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x2e;\" k=\"199\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x2d;\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x2c;\" k=\"199\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x2b;\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x2a;\" k=\"249\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x28;\" k=\"107\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x27;\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x26;\" k=\"51\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x24;\" k=\"76\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x23;\" k=\"160\" />" << std::endl;
os << "<hkern u1=\"t\" u2=\"&#x22;\" k=\"146\" />" << std::endl;
os << "<hkern u1=\"u\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"u\" u2=\"j\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"u\" u2=\"\\\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"u\" u2=\"T\" k=\"240\" />" << std::endl;
os << "<hkern u1=\"u\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"u\" u2=\"&#x37;\" k=\"210\" />" << std::endl;
os << "<hkern u1=\"u\" u2=\"&#x2c;\" k=\"119\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"l\" k=\"97\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"j\" k=\"159\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"_\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"]\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"\\\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"X\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"T\" k=\"247\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"D\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x3f;\" k=\"97\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x3e;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x37;\" k=\"217\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x2f;\" k=\"118\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x2e;\" k=\"195\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x2c;\" k=\"96\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x29;\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"v\" u2=\"&#x25;\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"l\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"j\" k=\"108\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"\\\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"T\" k=\"259\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"&#x3f;\" k=\"77\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"&#x37;\" k=\"229\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"&#x2c;\" k=\"60\" />" << std::endl;
os << "<hkern u1=\"w\" u2=\"&#x29;\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"l\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"j\" k=\"97\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"\\\" k=\"67\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"T\" k=\"247\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x3f;\" k=\"82\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x3e;\" k=\"95\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x3c;\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x37;\" k=\"186\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x2d;\" k=\"133\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x2c;\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x2b;\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"x\" u2=\"&#x2a;\" k=\"106\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"l\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"j\" k=\"99\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"a\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"_\" k=\"163\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"T\" k=\"226\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"A\" k=\"102\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x3f;\" k=\"80\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x3c;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x37;\" k=\"196\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x2f;\" k=\"163\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x2e;\" k=\"163\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x2d;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x2c;\" k=\"163\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x2b;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"y\" u2=\"&#x25;\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"l\" k=\"56\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"j\" k=\"131\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"`\" k=\"114\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"\\\" k=\"154\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"V\" k=\"73\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"T\" k=\"257\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"&#x3f;\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"&#x3e;\" k=\"105\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"&#x37;\" k=\"196\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"&#x2a;\" k=\"70\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"&#x27;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"z\" u2=\"&#x22;\" k=\"59\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"t\" k=\"97\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"s\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"q\" k=\"103\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"p\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"o\" k=\"117\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"j\" k=\"112\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"g\" k=\"111\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"e\" k=\"119\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"d\" k=\"128\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"c\" k=\"139\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"a\" k=\"98\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"^\" k=\"54\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"]\" k=\"55\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"A\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x3d;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x3c;\" k=\"147\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x3b;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x3a;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x2f;\" k=\"448\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x2b;\" k=\"85\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x2a;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x29;\" k=\"79\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x28;\" k=\"78\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x25;\" k=\"140\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x24;\" k=\"109\" />" << std::endl;
os << "<hkern u1=\"&#x7e;\" u2=\"&#x23;\" k=\"78\" />" << std::endl;
os << "</font>" << std::endl;
	}
};

#endif

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;


//============================================================================================================
//! \name Module Widgets


#endif
