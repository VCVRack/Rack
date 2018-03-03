#pragma once
#include <vector>
#include "util/common.hpp"
#include <jansson.h>


namespace rack {


struct Param {
	float value = 0.0;
};

struct Light {
	/** The square of the brightness value */
	float value = 0.0;
	float getBrightness();
	void setBrightness(float brightness) {
		value = (brightness > 0.f) ? brightness * brightness : 0.f;
	}
	void setBrightnessSmooth(float brightness);
};

struct DualLight {
	/** The square of the brightness value */
	float value = 0.0;

	float getPositiveBrightness() {
		return sqrtf(fmaxf(0.f, value));
	}
	float getNegativeBrightness() {
		return sqrtf(fmaxf(0.f, -value));
	}
	void setSignedBrightness(float brightness) {
		value = ((brightness < 0.f) ? -1.f : 1.f) * (brightness * brightness);
	}
	void setSignedBrightnessSmooth(float brightness, float dt) {
		float v = brightness * brightness;
		float abs_value = fabsf(value);
		if (v < abs_value) {
			// Fade out light with lambda = framerate
			abs_value += (v - abs_value) * dt;

		}
		else {
			// Immediately illuminate light
			abs_value = v;
		}
		value = ((brightness < 0.f) ? -1.f : 1.f) * abs_value;
	}
};

struct Input {
	/** Voltage of the port, zero if not plugged in. Read-only by Module */
	float value = 0.0;
	/** Whether a wire is plugged in */
	bool active = false;
	DualLight plugLight;
	/** Returns the value if a wire is plugged in, otherwise returns the given default value */
	float normalize(float normalValue) {
		return active ? value : normalValue;
	}
};

struct Output {
	/** Voltage of the port. Write-only by Module */
	float value = 0.0;
	/** Whether a wire is plugged in */
	bool active = false;
	DualLight plugLight;
};


struct Module {
	std::vector<Param> params;
	std::vector<Input> inputs;
	std::vector<Output> outputs;
	std::vector<Light> lights;
	/** For CPU usage meter */
	float cpuTime = 0.0;

	/** Constructs a Module with no params, inputs, outputs, and lights */
	Module() {}
	/** Constructs a Module with a fixed number of params, inputs, outputs, and lights */
	Module(int numParams, int numInputs, int numOutputs, int numLights = 0) {
		params.resize(numParams);
		inputs.resize(numInputs);
		outputs.resize(numOutputs);
		lights.resize(numLights);
	}
	virtual ~Module() {}

	/** Advances the module by 1 audio frame with duration 1.0 / gSampleRate */
	virtual void step() {}

	/** Called when the engine sample rate is changed */
	virtual void onSampleRateChange() {}
	/** Called when module is created by the Add Module popup, cloning, or when loading a patch or autosave */
	virtual void onCreate() {}
	/** Called when user explicitly deletes the module, not when Rack is closed or a new patch is loaded */
	virtual void onDelete() {}
	/** Called when user clicks Initialize in the module context menu */
	virtual void onReset() {
		// Call deprecated method
		reset();
	}
	/** Called when user clicks Randomize in the module context menu */
	virtual void onRandomize() {
		// Call deprecated method
		randomize();
	}

	/** Override these to store extra internal data in the "data" property */
	virtual json_t *toJson() { return NULL; }
	virtual void fromJson(json_t *root) {}

	/** Deprecated */
	virtual void reset() {}
	/** Deprecated */
	virtual void randomize() {}
};

struct Wire {
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	void step();
};

void engineInit();
void engineDestroy();
/** Launches engine thread */
void engineStart();
void engineStop();
/** Does not transfer pointer ownership */
void engineAddModule(Module *module);
void engineRemoveModule(Module *module);
/** Does not transfer pointer ownership */
void engineAddWire(Wire *wire);
void engineRemoveWire(Wire *wire);
void engineSetParam(Module *module, int paramId, float value);
void engineSetParamSmooth(Module *module, int paramId, float value);
void engineSetSampleRate(float sampleRate);
float engineGetSampleRate();
/** Returns the inverse of the current sample rate */
float engineGetSampleTime();

extern bool gPaused;


} // namespace rack
