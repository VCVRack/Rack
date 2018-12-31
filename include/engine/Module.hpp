#pragma once
#include "common.hpp"
#include "string.hpp"
#include "engine/Param.hpp"
#include "engine/Input.hpp"
#include "engine/Output.hpp"
#include "engine/Light.hpp"
#include <vector>
#include <jansson.h>


namespace rack {


struct Module {
	int id = 0;
	std::vector<Param> params;
	std::vector<Input> inputs;
	std::vector<Output> outputs;
	std::vector<Light> lights;
	/** For power meter */
	float cpuTime = 0.f;

	/** Constructs a Module with no params, inputs, outputs, and lights */
	Module();
	/** Deprecated. Use setup() instead. */
	Module(int numParams, int numInputs, int numOutputs, int numLights = 0) : Module() {
		setup(numParams, numInputs, numOutputs, numLights);
	}
	virtual ~Module() {}

	void setup(int numParams, int numInputs, int numOutputs, int numLights = 0);
	json_t *toJson();
	void fromJson(json_t *rootJ);
	void reset();
	void randomize();

	/** Advances the module by 1 audio frame with duration 1.0 / gSampleRate
	Override this method to read inputs and params, and to write outputs and lights.
	*/
	virtual void step() {}

	/** Called when the engine sample rate is changed */
	virtual void onSampleRateChange() {}
	/** Called when user clicks Initialize in the module context menu */
	virtual void onReset() {}
	/** Called when user clicks Randomize in the module context menu */
	virtual void onRandomize() {}

	/** Override these to store extra internal data in the "data" property of the module's JSON object */
	virtual json_t *dataToJson() { return NULL; }
	virtual void dataFromJson(json_t *root) {}
};


} // namespace rack
