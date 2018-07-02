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
	/** Emulates slow fall (but immediate rise) of LED brightness.
	`frames` rescales the timestep. For example, if your module calls this method every 16 frames, use 16.0.
	*/
	void setBrightnessSmooth(float brightness, float frames = 1.f);
};


struct Input {
	/** Voltage of the port, zero if not plugged in. Read-only by Module */
	float value = 0.0;
	/** Whether a wire is plugged in */
	bool active = false;
	Light plugLights[2];
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
	Light plugLights[2];
};


struct Module {
#ifdef USE_VST2
   // assigned when module is added to engine (see engine.cpp:engineAddModule())
   int vst2_unique_param_base_id;
#endif // USE_VST2
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

	/** Advances the module by 1 audio frame with duration 1.0 / gSampleRate
	Override this method to read inputs and params, and to write outputs and lights.
	*/
	virtual void step() {}

	/** Called when the engine sample rate is changed
	*/
	virtual void onSampleRateChange() {}
	/** Deprecated */
	virtual void onCreate() {}
	/** Deprecated */
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

	/** Override these to store extra internal data in the "data" property of the module's JSON object */
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
#ifdef USE_VST2
void engineSetParam(Module *module, int paramId, float value, bool bVSTAutomate = true);
#else
void engineSetParam(Module *module, int paramId, float value);
#endif
void engineSetParamSmooth(Module *module, int paramId, float value);
void engineSetSampleRate(float sampleRate);
float engineGetSampleRate();
/** Returns the inverse of the current sample rate */
float engineGetSampleTime();


} // namespace rack
