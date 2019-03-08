#pragma once

#include <iostream>

#include "AH.hpp"
#include "componentlibrary.hpp"

namespace rack_plugin_AmalgamatedHarmonics {

struct ParamEvent {
	
	ParamEvent(int t, int i, float v) : pType(t), pId(i), value(v) {}
	
	int pType;
	int pId;
	float value;

};

struct AHModule : Module {

	float delta;
	float rho;

	AHModule(int numParams, int numInputs, int numOutputs, int numLights = 0) : Module(numParams, numInputs, numOutputs, numLights) {
		delta = engineGetSampleTime();
		rho = engineGetSampleRate();
	}

	void onSampleRateChange() override { 
		delta = engineGetSampleTime();
		rho = engineGetSampleRate();
	}

	int stepX = 0;
	
	bool debugFlag = false;
	
	inline bool debugEnabled() {
		return debugFlag;
	}
	
	bool receiveEvents = false;
	int keepStateDisplay = 0;
	std::string paramState = ">";
	
	virtual void receiveEvent(ParamEvent e) {
		paramState = ">";
		keepStateDisplay = 0;
	}
	
	void step() override {
		
		stepX++;
	
		// Once we start stepping, we can process events
		receiveEvents = true;
		// Timeout for display
		keepStateDisplay++;
		if (keepStateDisplay > 50000) {
			paramState = ">"; 
		}
		
	}
	
};

struct StateDisplay : TransparentWidget {
	
	AHModule *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	StateDisplay() {
		font = Font::load(assetPlugin(plugin, "res/EurostileBold.ttf"));
	}

	void draw(NVGcontext *vg) override {
	
		Vec pos = Vec(0, 15);

		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);

		nvgFillColor(vg, nvgRGBA(255, 0, 0, 0xff));
	
		char text[128];
		snprintf(text, sizeof(text), "%s", module->paramState.c_str());
		nvgText(vg, pos.x + 10, pos.y + 5, text, NULL);			

	}
	
};

struct AHParamWidget { // it's a mix-in

	int pType = -1; // Should be set by ste<>(), but if not this allows us to catch pwidgers we are not interested in
	int pId;
	AHModule *mod = NULL;
	
	virtual ParamEvent generateEvent(float value) {
		return ParamEvent(pType,pId,value);
	};
		
	template <typename T = AHParamWidget>
	static void set(T *param, int pType, int pId) {
		param->pType = pType;
		param->pId = pId;
	}

};

// Not going to monitor buttons
struct AHButton : SVGSwitch, MomentarySwitch {
	AHButton() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/AHButton.svg")));
	}	
};

struct AHKnob : RoundKnob, AHParamWidget {
	void onChange(EventChange &e) override { 
		// One off cast, don't want to subclass from ParamWidget, so have to grab it here
		if (!mod) {
			mod = static_cast<AHModule *>(this->module);
		}
		mod->receiveEvent(generateEvent(value));
		RoundKnob::onChange(e);
	}
};

struct AHKnobSnap : AHKnob {
	AHKnobSnap() {
		snap = true;
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/AHKnob.svg")));
	}
};

struct AHKnobNoSnap : AHKnob {
	AHKnobNoSnap() {
		snap = false;
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/AHKnob.svg")));
	}
};

struct AHBigKnobNoSnap : AHKnob {
	AHBigKnobNoSnap() {
		snap = false;
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/AHBigKnob.svg")));
	}
};

struct AHBigKnobSnap : AHKnob {
	AHBigKnobSnap() {
		snap = true;
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/AHBigKnob.svg")));
	}
};

struct AHTrimpotSnap : AHKnob {
	AHTrimpotSnap() {
		snap = true;
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/AHTrimpot.svg")));
	}
};

struct AHTrimpotNoSnap : AHKnob {
	AHTrimpotNoSnap() {
		snap = false;
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/AHTrimpot.svg")));
	}
};


struct UI {

	enum UIElement {
		KNOB = 0,
		PORT,
		BUTTON,
		LIGHT,
		TRIMPOT
	};
	
	float Y_KNOB[2]    = {50.8f, 56.0f}; // w.r.t 22 = 28.8 from bottom
	float Y_PORT[2]    = {49.7f, 56.0f}; // 27.7
	float Y_BUTTON[2]  = {53.3f, 56.0f}; // 31.3 
	float Y_LIGHT[2]   = {57.7f, 56.0f}; // 35.7
	float Y_TRIMPOT[2] = {52.8f, 56.0f}; // 30.8

	float Y_KNOB_COMPACT[2]     = {30.1f, 35.0f}; // Calculated relative to  PORT=29 and the deltas above
	float Y_PORT_COMPACT[2]     = {29.0f, 35.0f};
	float Y_BUTTON_COMPACT[2]   = {32.6f, 35.0f};
	float Y_LIGHT_COMPACT[2]    = {37.0f, 35.0f};
	float Y_TRIMPOT_COMPACT[2]  = {32.1f, 35.0f};
	
	float X_KNOB[2]     = {12.5f, 48.0f}; // w.r.t 6.5 = 6 from left
	float X_PORT[2]     = {11.5f, 48.0f}; // 5
	float X_BUTTON[2]   = {14.7f, 48.0f}; // 8.2
	float X_LIGHT[2]    = {19.1f, 48.0f}; // 12.6
	float X_TRIMPOT[2]  = {14.7f, 48.0f}; // 8.2

	float X_KNOB_COMPACT[2]     = {21.0f, 35.0f}; // 15 + 6, see calc above
	float X_PORT_COMPACT[2]     = {20.0f, 35.0f}; // 15 + 5
	float X_BUTTON_COMPACT[2]   = {23.2f, 35.0f}; // 15 + 8.2
	float X_LIGHT_COMPACT[2]    = {27.6f, 35.0f}; // 15 + 12.6
	float X_TRIMPOT_COMPACT[2]  = {23.2f, 35.0f}; // 15 + 8.2
	

	Vec getPosition(int type, int xSlot, int ySlot, bool xDense, bool yDense);
	
	/* From the numerical key on a keyboard (0 = C, 11 = B), spacing in px between white keys and a starting x and Y coordinate for the C key (in px)
	* calculate the actual X and Y coordinate for a key, and the scale note to which that key belongs (see Midi note mapping)
	* http://www.grantmuller.com/MidiReference/doc/midiReference/ScaleReference.html */
	void calculateKeyboard(int inKey, float spacing, float xOff, float yOff, float *x, float *y, int *scale);
	
};

} // namespace rack_plugin_AmalgamatedHarmonics
