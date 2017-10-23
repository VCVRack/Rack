#pragma once
#include <vector>
#include "util.hpp"
#include <jansson.h>


namespace rack {


struct Param {
	float value = 0.0;
};

struct Input {
	/** Voltage of the port, zero if not plugged in. Read-only by Module */
	float value = 0.0;
	/** Whether a wire is plugged in */
	bool active = false;
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
};

struct Light {
	float value = 0.0;
	void setSmooth(float v);
};


struct Module {
	std::vector<Param> params;
	std::vector<Input> inputs;
	std::vector<Output> outputs;
	std::vector<Light> lights;
	/** For CPU usage meter */
	float cpuTime = 0.0;

	/** Deprecated, use constructor below this one */
	Module() {}
	/** Constructs Module with a fixed number of params, inputs, and outputs */
	Module(int numParams, int numInputs, int numOutputs, int numLights = 0) {
		params.resize(numParams);
		inputs.resize(numInputs);
		outputs.resize(numOutputs);
		lights.resize(numLights);
	}
	virtual ~Module() {}

	/** Advances the module by 1 audio frame with duration 1.0 / gSampleRate */
	virtual void step() {}
	virtual void onSampleRateChange() {}

	/** Override these to store extra internal data in the "data" property */
	virtual json_t *toJson() { return NULL; }
	virtual void fromJson(json_t *root) {}

	/** Override these to implement spacial behavior when user clicks Initialize and Randomize */
	virtual void reset() {}
	virtual void randomize() {}
	/** Deprecated */
	virtual void initialize() final {}
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

extern bool gPaused;


} // namespace rack
