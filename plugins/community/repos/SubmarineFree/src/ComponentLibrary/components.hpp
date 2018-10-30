//////////////////
// Ports
//////////////////

struct SilverPort : Port {
	NVGcolor col = nvgRGB(0xf0, 0xf0, 0xf0);
	SilverPort() {
		box.size.x = 25;
		box.size.y = 25;
	}
	void draw(NVGcontext *vg) override;
};

struct RedPort : SilverPort {
	RedPort() { col = nvgRGB(0xff, 0x20, 0x20); }
};

struct BluePort : SilverPort {
	BluePort() { col = nvgRGB(0x29, 0xb2, 0xef); }
};

struct BlackPort : SilverPort {
	BlackPort() { col = nvgRGB(0x40, 0x40, 0x40); }
};
/*
struct sub_port : SVGPort {
	sub_port() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Components/sub_port.svg")));
	}
};

struct sub_port_red : SVGPort {
	sub_port_red() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Components/sub_port_red.svg")));
	}
};

struct sub_port_blue : SVGPort {
	sub_port_blue() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Components/sub_port_blue.svg")));
	}
};

struct sub_port_black : SVGPort {
	sub_port_black() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Components/sub_port_black.svg")));
	}
};

*/
//////////////////
// Switches
//////////////////

struct sub_sw_2 : SVGSwitch, ToggleSwitch {
	sub_sw_2() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_2a.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_2b.svg")));
	}
};

struct sub_sw_3 : SVGSwitch, ToggleSwitch {
	sub_sw_3() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_3a.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_3b.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_3c.svg")));
	}
};

struct sub_sw_4 : SVGSwitch, ToggleSwitch {
	sub_sw_4() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4a.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4b.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4c.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4d.svg")));
	}
};

struct sub_sw_2h : SVGSwitch, ToggleSwitch {
	sub_sw_2h() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_2ha.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_2hb.svg")));
	}
};

struct sub_sw_3h : SVGSwitch, ToggleSwitch {
	sub_sw_3h() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_3ha.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_3hb.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_3hc.svg")));
	}
};

struct sub_sw_4h : SVGSwitch, ToggleSwitch {
	sub_sw_4h() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4ha.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4hb.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4hc.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_sw_4hd.svg")));
	}
};

struct sub_btn : SVGSwitch, ToggleSwitch {
	sub_btn() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_btn.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Components/sub_btn_a.svg")));
	}
	void step() override {
		setValue(module->params[paramId].value);
	}
};

//////////////////
// Knobs
//////////////////

struct LightKnob : Knob {
	/** Angles in radians */
	float minAngle = -0.83*M_PI;
	float maxAngle = 0.83*M_PI;
	/** Radii in standard units */
	float radius = 19.0;
	int enabled = 1;
	LightKnob() {smooth = false;}
	void draw(NVGcontext *vg) override;
	void setEnabled(int val);
	void setRadius(int r);
};

template <class K>
struct TinyKnob : K {
	TinyKnob() {
		K::setRadius(9.0f);
	}
};

template <class K>
struct SmallKnob : K {
	SmallKnob() {
		K::setRadius(12.0f);
	}
};

template <class K>
struct MedKnob : K {
	MedKnob() {
		K::setRadius(19.0f);
	}
};

template <class K>
struct LargeKnob : K {
	LargeKnob() {
		K::setRadius(27.0f);
	}
};

template <class K>
struct SnapKnob : K {
	SnapKnob() {
		K::snap = true;
	}
};

template <class K>
struct NarrowKnob : K {
	NarrowKnob() {
		K::minAngle = -0.75*M_PI;
		K::maxAngle = 0.75*M_PI;	
	}
};

//////////////////
// Lights
//////////////////

struct BlueRedLight : GrayModuleLightWidget {
	BlueRedLight() {
		addBaseColor(COLOR_BLUE);
		addBaseColor(COLOR_RED);
	}
};
