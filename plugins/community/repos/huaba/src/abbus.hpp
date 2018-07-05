
// Exports 
struct dh_switch3 : SVGSwitch, ToggleSwitch {
	dh_switch3() {
		addFrame(SVG::load(assetPlugin(plugin,"res/Switch3_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/Switch3_1.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/Switch3_2.svg")));
	}
};
